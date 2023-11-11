// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
//
// Copyright (C) 2018-2020 Intel Corporation

#include "precomp.hpp"

// needs to be included regardless if IE is present or not
// (cv::gapi::ie::backend() is still there and is defined always)
#include "backends/ie/giebackend.hpp"

#ifdef HAVE_INF_ENGINE

#if INF_ENGINE_RELEASE <= 2019010000
#   error G-API IE module supports only OpenVINO IE >= 2019 R1
#endif

#include <functional>
#include <unordered_set>

#include <ade/util/algorithm.hpp>

#include <ade/util/range.hpp>
#include <ade/util/zip_range.hpp>
#include <ade/util/chain_range.hpp>
#include <ade/typed_graph.hpp>

#include <opencv2/core/utility.hpp>
#include <opencv2/core/utils/logger.hpp>

#include <opencv2/gapi/gcommon.hpp>
#include <opencv2/gapi/garray.hpp>
#include <opencv2/gapi/gopaque.hpp>
#include <opencv2/gapi/util/any.hpp>
#include <opencv2/gapi/gtype_traits.hpp>
#include <opencv2/gapi/infer.hpp>
#include <opencv2/gapi/own/convert.hpp>

#include "compiler/gobjref.hpp"
#include "compiler/gmodel.hpp"

#include "backends/ie/util.hpp"
#include "backends/ie/giebackend/giewrapper.hpp"

#include "api/gbackend_priv.hpp" // FIXME: Make it part of Backend SDK!

namespace IE = InferenceEngine;

namespace {

inline IE::ROI toIE(const cv::Rect &rc) {
    return IE::ROI
        { 0u
        , static_cast<std::size_t>(rc.x)
        , static_cast<std::size_t>(rc.y)
        , static_cast<std::size_t>(rc.width)
        , static_cast<std::size_t>(rc.height)
        };
}

inline IE::SizeVector toIE(const cv::MatSize &sz) {
    return cv::to_own<IE::SizeVector::value_type>(sz);
}
inline std::vector<int> toCV(const IE::SizeVector &vsz) {
    std::vector<int> result;
    result.reserve(vsz.size());
    for (auto sz : vsz) {
        result.push_back(ade::util::checked_cast<int>(sz));
    }
    return result;
}

inline IE::Layout toIELayout(const std::size_t ndims) {
    static const IE::Layout lts[] = {
        IE::Layout::SCALAR,
        IE::Layout::C,
        IE::Layout::NC,
        IE::Layout::CHW,
        IE::Layout::NCHW,
        IE::Layout::NCDHW,
    };
    // FIXME: This is not really a good conversion,
    // since it may also stand for NHWC/HW/CN/NDHWC data
    CV_Assert(ndims < sizeof(lts) / sizeof(lts[0]));
    return lts[ndims];
}

inline IE::Precision toIE(int depth) {
    switch (depth) {
    case CV_8U:  return IE::Precision::U8;
    case CV_32F: return IE::Precision::FP32;
    default:     GAPI_Assert(false && "Unsupported data type");
    }
    return IE::Precision::UNSPECIFIED;
}
inline int toCV(IE::Precision prec) {
    switch (prec) {
    case IE::Precision::U8:   return CV_8U;
    case IE::Precision::FP32: return CV_32F;
    default:     GAPI_Assert(false && "Unsupported data type");
    }
    return -1;
}

inline IE::TensorDesc toIE(const cv::Mat &mat, cv::gapi::ie::TraitAs hint) {
    const auto &sz = mat.size;

    // NB: For some reason RGB image is 2D image
    // (since channel component is not counted here).
    // Note: regular 2D vectors also fall into this category
    if (sz.dims() == 2 && hint == cv::gapi::ie::TraitAs::IMAGE)
    {
        // NB: This logic is mainly taken from IE samples
        const size_t pixsz    = CV_ELEM_SIZE1(mat.type());
        const size_t channels = mat.channels();
        const size_t height   = mat.size().height;
        const size_t width    = mat.size().width;

        const size_t strideH  = mat.step.buf[0];
        const size_t strideW  = mat.step.buf[1];

        const bool is_dense =
            strideW == pixsz * channels &&
            strideH == strideW * width;

        if (!is_dense)
            cv::util::throw_error(std::logic_error("Doesn't support conversion"
                                                   " from non-dense cv::Mat"));

        return IE::TensorDesc(toIE(mat.depth()),
                              IE::SizeVector{1, channels, height, width},
                              IE::Layout::NHWC);
    }

    return IE::TensorDesc(toIE(mat.depth()), toIE(sz), toIELayout(sz.dims()));
}

inline IE::Blob::Ptr wrapIE(const cv::Mat &mat, cv::gapi::ie::TraitAs hint) {
    const auto tDesc = toIE(mat, hint);
    switch (mat.depth()) {
        // NB: Seems there's no way to create an untyped (T-less) Blob::Ptr
        // in IE given only precision via TensorDesc. So we have to do this:
#define HANDLE(E,T) \
        case CV_##E: return IE::make_shared_blob<T>(tDesc, const_cast<T*>(mat.ptr<T>()))
        HANDLE(8U, uint8_t);
        HANDLE(32F, float);
#undef HANDLE
    default: GAPI_Assert(false && "Unsupported data type");
    }
    return IE::Blob::Ptr{};
}

template<class MatType>
inline void copyFromIE(const IE::Blob::Ptr &blob, MatType &mat) {
    switch (blob->getTensorDesc().getPrecision()) {
#define HANDLE(E,T)                                                 \
        case IE::Precision::E: std::copy_n(blob->buffer().as<T*>(), \
                                           mat.total(),             \
                                           reinterpret_cast<T*>(mat.data)); \
            break;
        HANDLE(U8, uint8_t);
        HANDLE(FP32, float);
#undef HANDLE
    default: GAPI_Assert(false && "Unsupported data type");
    }
}

// IE-specific metadata, represents a network with its parameters
struct IEUnit {
    static const char *name() { return "IEModelConfig"; }

    cv::gapi::ie::detail::ParamDesc params;
    IE::CNNNetwork net;
    IE::InputsDataMap inputs;
    IE::OutputsDataMap outputs;

    explicit IEUnit(const cv::gapi::ie::detail::ParamDesc &pp)
        : params(pp) {
        net = cv::gimpl::ie::wrap::readNetwork(params);
        inputs  = net.getInputsInfo();
        outputs = net.getOutputsInfo();
        // The practice shows that not all inputs and not all outputs
        // are mandatory to specify in IE model.
        // So what we're concerned here about is:
        // if operation's (not topology's) input/output number is
        // greater than 1, then we do care about input/output layer
        // names. Otherwise, names are picked up automatically.
        // TODO: Probably this check could be done at the API entry point? (gnet)
        if (params.num_in > 1u && params.num_in != params.input_names.size()) {
            cv::util::throw_error(std::logic_error("Please specify input layer names for "
                                                   + params.model_path));
        }
        if (params.num_out > 1u && params.num_out != params.output_names.size()) {
            cv::util::throw_error(std::logic_error("Please specify output layer names for "
                                                   + params.model_path));
        }
        if (params.num_in == 1u && params.input_names.empty()) {
            params.input_names = { inputs.begin()->first };
        }
        if (params.num_out == 1u && params.output_names.empty()) {
            params.output_names = { outputs.begin()->first };
        }
    }

    // This method is [supposed to be] called at Island compilation stage
    cv::gimpl::ie::IECompiled compile() const {
        auto plugin       = cv::gimpl::ie::wrap::getPlugin(params);
        auto this_network = cv::gimpl::ie::wrap::loadNetwork(plugin, net, params);
        auto this_request = this_network.CreateInferRequest();

        // Bind const data to infer request
        for (auto &&p : params.const_inputs) {
            // FIXME: SetBlob is known to be inefficient,
            // it is worth to make a customizable "initializer" and pass the
            // cv::Mat-wrapped blob there to support IE's optimal "GetBlob idiom"
            // Still, constant data is to set only once.
            this_request.SetBlob(p.first, wrapIE(p.second.first, p.second.second));
        }
        return {plugin, this_network, this_request};
    }
};

struct IECallContext
{
    // Input parameters passed to an inference operation.
    std::vector<cv::GArg> args;

    //FIXME: avoid conversion of arguments from internal representation to OpenCV one on each call
    //to OCV kernel. (This can be achieved by a two single time conversions in GCPUExecutable::run,
    //once on enter for input and output arguments, and once before return for output arguments only
    //FIXME: check if the above applies to this backend (taken from CPU)
    std::unordered_map<std::size_t, cv::GRunArgP> results;

    // Generic accessor API
    template<typename T>
    const T& inArg(std::size_t input) { return args.at(input).get<T>(); }

    // Syntax sugar
    const cv::Mat&   inMat(std::size_t input) {
        return inArg<cv::Mat>(input);
    }
    cv::Mat&         outMatR(std::size_t output) {
        return *cv::util::get<cv::Mat*>(results.at(output));
    }

    template<typename T> std::vector<T>& outVecR(std::size_t output) { // FIXME: the same issue
        return outVecRef(output).wref<T>();
    }
    cv::detail::VectorRef& outVecRef(std::size_t output) {
        return cv::util::get<cv::detail::VectorRef>(results.at(output));
    }
};

struct IECallable {
    static const char *name() { return "IERequestCallable"; }
    // FIXME: Make IECallContext manage them all? (3->1)
    using Run = std::function<void(cv::gimpl::ie::IECompiled &, const IEUnit &, IECallContext &)>;
    Run run;
};

struct KImpl {
    cv::gimpl::CustomMetaFunction::CM customMetaFunc;
    IECallable::Run run;
};

// FIXME: Is there a way to take a typed graph (our GModel),
// and create a new typed graph _ATOP_ of that (by extending with a couple of
// new types?).
// Alternatively, is there a way to compose types graphs?
//
// If not, we need to introduce that!
using GIEModel = ade::TypedGraph
    < cv::gimpl::Protocol
    , cv::gimpl::Op
    , cv::gimpl::NetworkParams
    , cv::gimpl::CustomMetaFunction
    , IEUnit
    , IECallable
    >;

// FIXME: Same issue with Typed and ConstTyped
using GConstGIEModel = ade::ConstTypedGraph
    < cv::gimpl::Protocol
    , cv::gimpl::Op
    , cv::gimpl::NetworkParams
    , cv::gimpl::CustomMetaFunction
    , IEUnit
    , IECallable
    >;
} // anonymous namespace

// GCPUExcecutable implementation //////////////////////////////////////////////
cv::gimpl::ie::GIEExecutable::GIEExecutable(const ade::Graph &g,
                                            const std::vector<ade::NodeHandle> &nodes)
    : m_g(g), m_gm(m_g) {
    // FIXME: Currently this backend is capable to run a single inference node only.
    // Need to extend our island fusion with merge/not-to-merge decision making parametrization
    GConstGIEModel iem(g);

    for (auto &nh : nodes) {
        switch (m_gm.metadata(nh).get<NodeType>().t) {
        case NodeType::OP:
            if (this_nh == nullptr) {
                this_nh = nh;
                this_iec = iem.metadata(this_nh).get<IEUnit>().compile();
            }
            else
                util::throw_error(std::logic_error("Multi-node inference is not supported!"));
            break;

        case NodeType::DATA: {
            m_dataNodes.push_back(nh);
            const auto &desc = m_gm.metadata(nh).get<Data>();
            if (desc.storage == Data::Storage::CONST_VAL) {
                util::throw_error(std::logic_error("No const data please!"));
            }
            if (desc.storage == Data::Storage::INTERNAL) {
                util::throw_error(std::logic_error("No internal data please!"));
            }
            break;
        }
        default: util::throw_error(std::logic_error("Unsupported NodeType type"));
        }
    }
}

// FIXME: Document what it does
cv::GArg cv::gimpl::ie::GIEExecutable::packArg(const cv::GArg &arg) {
    // No API placeholders allowed at this point
    // FIXME: this check has to be done somewhere in compilation stage.
    GAPI_Assert(   arg.kind != cv::detail::ArgKind::GMAT
                && arg.kind != cv::detail::ArgKind::GSCALAR
                && arg.kind != cv::detail::ArgKind::GARRAY);

    if (arg.kind != cv::detail::ArgKind::GOBJREF) {
        util::throw_error(std::logic_error("Inference supports G-types ONLY!"));
    }
    GAPI_Assert(arg.kind == cv::detail::ArgKind::GOBJREF);

    // Wrap associated CPU object (either host or an internal one)
    // FIXME: object can be moved out!!! GExecutor faced that.
    const cv::gimpl::RcDesc &ref = arg.get<cv::gimpl::RcDesc>();
    switch (ref.shape)
    {
    case GShape::GMAT:    return GArg(m_res.slot<cv::Mat>()[ref.id]);

    // Note: .at() is intentional for GArray as object MUST be already there
    //   (and constructed by either bindIn/Out or resetInternal)
    case GShape::GARRAY:  return GArg(m_res.slot<cv::detail::VectorRef>().at(ref.id));

    // Note: .at() is intentional for GOpaque as object MUST be already there
    //   (and constructed by either bindIn/Out or resetInternal)
    case GShape::GOPAQUE:  return GArg(m_res.slot<cv::detail::OpaqueRef>().at(ref.id));

    default:
        util::throw_error(std::logic_error("Unsupported GShape type"));
        break;
    }
}

void cv::gimpl::ie::GIEExecutable::run(std::vector<InObj>  &&input_objs,
                                       std::vector<OutObj> &&output_objs) {
    // Update resources with run-time information - what this Island
    // has received from user (or from another Island, or mix...)
    // FIXME: Check input/output objects against GIsland protocol

    for (auto& it : input_objs)   magazine::bindInArg (m_res, it.first, it.second);
    for (auto& it : output_objs)  magazine::bindOutArg(m_res, it.first, it.second);

    // FIXME: Running just a single node now.
    // Not sure if need to support many of them, though
    // FIXME: Make this island-unmergeable?
    const auto &op = m_gm.metadata(this_nh).get<Op>();

    // Initialize kernel's execution context:
    // - Input parameters
    IECallContext context;
    context.args.reserve(op.args.size());
    using namespace std::placeholders;
    ade::util::transform(op.args,
                          std::back_inserter(context.args),
                          std::bind(&GIEExecutable::packArg, this, _1));

    // - Output parameters.
    for (const auto &out_it : ade::util::indexed(op.outs)) {
        // FIXME: Can the same GArg type resolution mechanism be reused here?
        const auto out_port  = ade::util::index(out_it);
        const auto out_desc  = ade::util::value(out_it);
        context.results[out_port] = magazine::getObjPtr(m_res, out_desc);
    }

    // And now trigger the execution
    GConstGIEModel giem(m_g);
    const auto &uu = giem.metadata(this_nh).get<IEUnit>();
    const auto &kk = giem.metadata(this_nh).get<IECallable>();
    kk.run(this_iec, uu, context);

    for (auto &it : output_objs) magazine::writeBack(m_res, it.first, it.second);

    // In/Out args clean-up is mandatory now with RMat
    for (auto &it : input_objs) magazine::unbind(m_res, it.first);
    for (auto &it : output_objs) magazine::unbind(m_res, it.first);
}

namespace cv {
namespace gimpl {
namespace ie {

struct Infer: public cv::detail::KernelTag {
    using API = cv::GInferBase;
    static cv::gapi::GBackend backend()  { return cv::gapi::ie::backend(); }
    static KImpl kernel()                { return KImpl{outMeta, run}; }

    static cv::GMetaArgs outMeta(const ade::Graph      &gr,
                                 const ade::NodeHandle &nh,
                                 const cv::GMetaArgs   &in_metas,
                                 const cv::GArgs       &/*in_args*/) {
        // Specify network's output layer metadata to the framework
        // Also specify the input information to the IE from the framework
        // NB: Have no clue if network's input [dimensions] may ever define
        // its output dimensions. It seems possible with OpenCV DNN APIs

        cv::GMetaArgs result;

        GConstGIEModel gm(gr);
        const auto &uu = gm.metadata(nh).get<IEUnit>();

        // Initialize input information
        // Note our input layers list order matches the API order and so
        // meta order.
        GAPI_Assert(uu.params.input_names.size() == in_metas.size()
                    && "Known input layers count doesn't match input meta count");

        for (auto &&it : ade::util::zip(ade::util::toRange(uu.params.input_names),
                                        ade::util::toRange(in_metas))) {
            auto       &&ii = uu.inputs.at(std::get<0>(it));
            const auto & mm =              std::get<1>(it);

            GAPI_Assert(util::holds_alternative<cv::GMatDesc>(mm)
                        && "Non-GMat inputs are not supported");

            const auto &meta = util::get<cv::GMatDesc>(mm);
            ii->setPrecision(toIE(meta.depth));
            ii->getPreProcess().setResizeAlgorithm(IE::RESIZE_BILINEAR);
        }

        // FIXME: It would be nice here to have an exact number of network's
        // input/output parameters. Probably GCall should store it here for us.
        // It doesn't, as far as I know..
        for (const auto &out_name : uu.params.output_names) {
            // NOTE: our output_names vector follows the API order
            // of this operation's outputs
            const IE::DataPtr& ie_out = uu.outputs.at(out_name);
            const IE::SizeVector dims = ie_out->getTensorDesc().getDims();

            cv::GMatDesc outm(toCV(ie_out->getPrecision()),
                              toCV(ie_out->getTensorDesc().getDims()));
            result.emplace_back(outm);
        }
        return result;
    }

    static void run(IECompiled &iec, const IEUnit &uu, IECallContext &ctx) {
        // non-generic version for now:
        // - assumes all inputs/outputs are always Mats
        for (auto i : ade::util::iota(uu.params.num_in)) {
            // TODO: Ideally we shouldn't do SetBlob() but GetBlob() instead,
            // and redirect our data producers to this memory
            // (A memory dialog comes to the picture again)

            const cv::Mat this_mat = ctx.inMat(i);
            // FIXME: By default here we trait our inputs as images.
            // May be we need to make some more intelligence here about it
            IE::Blob::Ptr this_blob = wrapIE(this_mat, cv::gapi::ie::TraitAs::IMAGE);
            iec.this_request.SetBlob(uu.params.input_names[i], this_blob);
        }
        iec.this_request.Infer();
        for (auto i : ade::util::iota(uu.params.num_out)) {
            // TODO: Think on avoiding copying here.
            // Either we should ask IE to use our memory (what is not always the
            // best policy) or use IE-allocated buffer inside (and pass it to the graph).
            // Not a <very> big deal for classifiers and detectors,
            // but may be critical to segmentation.

            cv::Mat& out_mat = ctx.outMatR(i);
            IE::Blob::Ptr this_blob = iec.this_request.GetBlob(uu.params.output_names[i]);
            copyFromIE(this_blob, out_mat);
        }
    }
};

struct InferList: public cv::detail::KernelTag {
    using API = cv::GInferListBase;
    static cv::gapi::GBackend backend()  { return cv::gapi::ie::backend(); }
    static KImpl kernel()                { return KImpl{outMeta, run}; }

    static cv::GMetaArgs outMeta(const ade::Graph      &gr,
                                 const ade::NodeHandle &nh,
                                 const cv::GMetaArgs   &in_metas,
                                 const cv::GArgs       &/*in_args*/) {
        // Specify the input information to the IE from the framework
        // NB: Have no clue if network's input [dimensions] may ever define
        // its output dimensions. It seems possible with OpenCV DNN APIs

        GConstGIEModel gm(gr);
        const auto &uu = gm.metadata(nh).get<IEUnit>();

        // Initialize input information
        // Note our input layers list order matches the API order and so
        // meta order.
        GAPI_Assert(uu.params.input_names.size() == (in_metas.size() - 1u)
                    && "Known input layers count doesn't match input meta count");

        std::size_t idx = 1u;
        for (auto &&input_name : uu.params.input_names) {
            auto       &&ii = uu.inputs.at(input_name);
            const auto & mm = in_metas[idx++];

            GAPI_Assert(util::holds_alternative<cv::GMatDesc>(mm)
                        && "Non-GMat inputs are not supported");

            const auto &meta = util::get<cv::GMatDesc>(mm);
            ii->setPrecision(toIE(meta.depth));
            ii->getPreProcess().setResizeAlgorithm(IE::RESIZE_BILINEAR);
        }

        // roi-list version is much easier at the moment.
        // All our outputs are vectors which don't have
        // metadata at the moment - so just create a vector of
        // "empty" array metadatas of the required size.
        return cv::GMetaArgs(uu.params.output_names.size(),
                             cv::GMetaArg{cv::empty_array_desc()});
    }

    static void run(IECompiled &iec, const IEUnit &uu, IECallContext &ctx) {
        // non-generic version for now:
        // - assumes zero input is always ROI list
        // - assumes all inputs/outputs are always Mats
        GAPI_Assert(uu.params.num_in == 1); // roi list is not counted in net's inputs

        const auto& in_roi_vec = ctx.inArg<cv::detail::VectorRef>(0u).rref<cv::Rect>();
        const cv::Mat this_mat = ctx.inMat(1u);
        // Since we do a ROI list inference, always assume our input buffer is image
        IE::Blob::Ptr this_blob = wrapIE(this_mat, cv::gapi::ie::TraitAs::IMAGE);

        // FIXME: This could be done ONCE at graph compile stage!
        std::vector< std::vector<int> > cached_dims(uu.params.num_out);
        for (auto i : ade::util::iota(uu.params.num_out)) {
            const IE::DataPtr& ie_out = uu.outputs.at(uu.params.output_names[i]);
            cached_dims[i] = toCV(ie_out->getTensorDesc().getDims());
            ctx.outVecR<cv::Mat>(i).clear();
            // FIXME: Isn't this should be done automatically
            // by some resetInternalData(), etc? (Probably at the GExecutor level)
        }

        for (const auto &rc : in_roi_vec) {
            // FIXME: Assumed only 1 input
            IE::Blob::Ptr roi_blob = IE::make_shared_blob(this_blob, toIE(rc));
            iec.this_request.SetBlob(uu.params.input_names[0u], roi_blob);
            iec.this_request.Infer();

            // While input is fixed to be 1,
            // there may be still multiple outputs
            for (auto i : ade::util::iota(uu.params.num_out)) {
                std::vector<cv::Mat> &out_vec = ctx.outVecR<cv::Mat>(i);

                IE::Blob::Ptr out_blob = iec.this_request.GetBlob(uu.params.output_names[i]);

                cv::Mat out_mat(cached_dims[i], toCV(out_blob->getTensorDesc().getPrecision()));
                copyFromIE(out_blob, out_mat);  // FIXME: Avoid data copy. Not sure if it is possible though
                out_vec.push_back(std::move(out_mat));
            }
        }
    }
};

struct InferList2: public cv::detail::KernelTag {
    using API = cv::GInferList2Base;
    static cv::gapi::GBackend backend()  { return cv::gapi::ie::backend(); }
    static KImpl kernel()                { return KImpl{outMeta, run}; }

    static cv::GMetaArgs outMeta(const ade::Graph      &gr,
                                 const ade::NodeHandle &nh,
                                 const cv::GMetaArgs   &in_metas,
                                 const cv::GArgs       &/*in_args*/) {
        // Specify the input information to the IE from the framework
        // NB: Have no clue if network's input [dimensions] may ever define
        // its output dimensions. It seems possible with OpenCV DNN APIs

        GConstGIEModel gm(gr);
        const auto &uu = gm.metadata(nh).get<IEUnit>();

        // Initialize input information
        // Note our input layers list order matches the API order and so
        // meta order.
        GAPI_Assert(uu.params.input_names.size() == (in_metas.size() - 1u)
                    && "Known input layers count doesn't match input meta count");

        const auto &op = gm.metadata(nh).get<Op>();

        // In contrast to InferList, the InferList2 has only one
        // "full-frame" image argument, and all the rest are arrays of
        // ether ROI or blobs. So here we set the 0th arg image format
        // to all inputs which are ROI-based (skipping the
        // "blob"-based ones)
        // FIXME: this is filtering not done, actually! GArrayDesc has
        // no hint for its underlying type!
        const auto &mm_0   = in_metas[0u];
        const auto &meta_0 = util::get<cv::GMatDesc>(mm_0);
        GAPI_Assert(   !meta_0.isND()
                    && !meta_0.planar
                    && "Only images are supported as the 0th argument");
        std::size_t idx = 1u;
        for (auto &&input_name : uu.params.input_names) {
                  auto &ii = uu.inputs.at(input_name);
            const auto &mm = in_metas[idx];
            GAPI_Assert(util::holds_alternative<cv::GArrayDesc>(mm)
                        && "Non-array inputs are not supported");

            if (op.k.inKinds[idx] == cv::detail::OpaqueKind::CV_RECT) {
                // This is a cv::Rect -- configure the IE preprocessing
                ii->setPrecision(toIE(meta_0.depth));
                ii->getPreProcess().setResizeAlgorithm(IE::RESIZE_BILINEAR);
            } else {
                // This is a cv::GMat (equals to: cv::Mat)
                // Just validate that it is really the type
                // (other types are prohibited here)
                GAPI_Assert(op.k.inKinds[idx] == cv::detail::OpaqueKind::CV_MAT);
            }
            idx++; // NB: Never forget to increment the counter
        }

        // roi-list version is much easier at the moment.
        // All our outputs are vectors which don't have
        // metadata at the moment - so just create a vector of
        // "empty" array metadatas of the required size.
        return cv::GMetaArgs(uu.params.output_names.size(),
                             cv::GMetaArg{cv::empty_array_desc()});
    }

    static void run(IECompiled &iec, const IEUnit &uu, IECallContext &ctx) {
        GAPI_Assert(ctx.args.size() > 1u
                    && "This operation must have at least two arguments");

        // Since we do a ROI list inference, always assume our input buffer is image
        const cv::Mat mat_0  = ctx.inMat(0u);
        IE::Blob::Ptr blob_0 = wrapIE(mat_0, cv::gapi::ie::TraitAs::IMAGE);

        // Take the next argument, which must be vector (of any kind).
        // Use it only to obtain the ROI list size (sizes of all other
        // vectors must be equal to this one)
        const auto list_size = ctx.inArg<cv::detail::VectorRef>(1u).size();

        // FIXME: This could be done ONCE at graph compile stage!
        std::vector< std::vector<int> > cached_dims(uu.params.num_out);
        for (auto i : ade::util::iota(uu.params.num_out)) {
            const IE::DataPtr& ie_out = uu.outputs.at(uu.params.output_names[i]);
            cached_dims[i] = toCV(ie_out->getTensorDesc().getDims());
            ctx.outVecR<cv::Mat>(i).clear();
            // FIXME: Isn't this should be done automatically
            // by some resetInternalData(), etc? (Probably at the GExecutor level)
        }

        // For every ROI in the list {{{
        for (const auto &list_idx : ade::util::iota(list_size)) {
            // For every input of the net {{{
            for (auto in_idx : ade::util::iota(uu.params.num_in)) {
                const auto &this_vec = ctx.inArg<cv::detail::VectorRef>(in_idx+1u);
                GAPI_Assert(this_vec.size() == list_size);
                // Prepare input {{{
                IE::Blob::Ptr this_blob;
                if (this_vec.getKind() == cv::detail::OpaqueKind::CV_RECT) {
                    // ROI case - create an ROI blob
                    const auto &vec = this_vec.rref<cv::Rect>();
                    this_blob = IE::make_shared_blob(blob_0, toIE(vec[list_idx]));
                } else if (this_vec.getKind() == cv::detail::OpaqueKind::CV_MAT) {
                    // Mat case - create a regular blob
                    // FIXME: NOW Assume Mats are always BLOBS (not
                    // images)
                    const auto &vec = this_vec.rref<cv::Mat>();
                    const auto &mat = vec[list_idx];
                    this_blob = wrapIE(mat, cv::gapi::ie::TraitAs::TENSOR);
                } else {
                    GAPI_Assert(false && "Only Rect and Mat types are supported for infer list 2!");
                }
                iec.this_request.SetBlob(uu.params.input_names[in_idx], this_blob);
                // }}} (Preapre input)
            } // }}} (For every input of the net)

            // Run infer request {{{
            iec.this_request.Infer();
            // }}} (Run infer request)

            // For every output of the net {{{
            for (auto i : ade::util::iota(uu.params.num_out)) {
                // Push results to the list {{{
                std::vector<cv::Mat> &out_vec = ctx.outVecR<cv::Mat>(i);
                IE::Blob::Ptr out_blob = iec.this_request.GetBlob(uu.params.output_names[i]);
                cv::Mat out_mat(cached_dims[i], toCV(out_blob->getTensorDesc().getPrecision()));
                copyFromIE(out_blob, out_mat);  // FIXME: Avoid data copy. Not sure if it is possible though
                out_vec.push_back(std::move(out_mat));
                // }}} (Push results to the list)
            } // }}} (For every output of the net)
        } // }}} (For every ROI in the list)
    }
};

} // namespace ie
} // namespace gapi
} // namespace cv


// IE backend implementation of GBackend::Priv ///////////////////////
namespace {
    class GIEBackendImpl final: public cv::gapi::GBackend::Priv {
        virtual void unpackKernel(ade::Graph            &gr,
                                  const ade::NodeHandle &nh,
                                  const cv::GKernelImpl &ii) override {
            using namespace cv::gimpl;
            // FIXME: Introduce a DNNBackend interface which'd specify
            // the framework for this???
            GIEModel gm(gr);
            auto &np = gm.metadata(nh).get<NetworkParams>();
            auto &pp = cv::util::any_cast<cv::gapi::ie::detail::ParamDesc>(np.opaque);
            const auto &ki = cv::util::any_cast<KImpl>(ii.opaque);

            GModel::Graph model(gr);
            auto& op = model.metadata(nh).get<Op>();

            // NB: In case generic infer, info about in/out names is stored in operation (op.params)
            if (pp.is_generic)
            {
                auto& info      = cv::util::any_cast<cv::InOutInfo>(op.params);
                pp.input_names  = info.in_names;
                pp.output_names = info.out_names;
                pp.num_in       = info.in_names.size();
                pp.num_out      = info.out_names.size();
            }

            gm.metadata(nh).set(IEUnit{pp});
            gm.metadata(nh).set(IECallable{ki.run});
            gm.metadata(nh).set(CustomMetaFunction{ki.customMetaFunc});
        }

        virtual EPtr compile(const ade::Graph &graph,
                             const cv::GCompileArgs &,
                             const std::vector<ade::NodeHandle> &nodes) const override {
            return EPtr{new cv::gimpl::ie::GIEExecutable(graph, nodes)};
        }

        virtual cv::gapi::GKernelPackage auxiliaryKernels() const override {
            return cv::gapi::kernels< cv::gimpl::ie::Infer
                                    , cv::gimpl::ie::InferList
                                    , cv::gimpl::ie::InferList2
                                    >();
        }
    };
}

cv::gapi::GBackend cv::gapi::ie::backend() {
    static cv::gapi::GBackend this_backend(std::make_shared<GIEBackendImpl>());
    return this_backend;
}

cv::Mat cv::gapi::ie::util::to_ocv(IE::Blob::Ptr blob) {
    const auto& tdesc = blob->getTensorDesc();
    return cv::Mat(toCV(tdesc.getDims()),
                   toCV(tdesc.getPrecision()),
                   blob->buffer().as<uint8_t*>());
}

std::vector<int> cv::gapi::ie::util::to_ocv(const IE::SizeVector &dims) {
    return toCV(dims);
}

IE::Blob::Ptr cv::gapi::ie::util::to_ie(cv::Mat &blob) {
    return wrapIE(blob, cv::gapi::ie::TraitAs::IMAGE);
}

#else // HAVE_INF_ENGINE

cv::gapi::GBackend cv::gapi::ie::backend() {
    // Still provide this symbol to avoid linking issues
    util::throw_error(std::runtime_error("G-API has been compiled without OpenVINO IE support"));
}
#endif // HAVE_INF_ENGINE
