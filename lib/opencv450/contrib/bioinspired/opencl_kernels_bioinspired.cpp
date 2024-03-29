// This file is auto-generated. Do not edit!

#include "opencv2/core.hpp"
#include "cvconfig.h"
#include "opencl_kernels_bioinspired.hpp"

#ifdef HAVE_OPENCL

namespace cv
{
namespace ocl
{
namespace bioinspired
{

static const char* const moduleName = "bioinspired";

struct cv::ocl::internal::ProgramEntry retina_kernel_oclsrc={moduleName, "retina_kernel",
"#define WIDTH_MULTIPLE (32 >> 2)\n"
"kernel void horizontalCausalFilter_addInput(\n"
"global const float * input,\n"
"global float * output,\n"
"const int cols,\n"
"const int rows,\n"
"const int elements_per_row,\n"
"const int in_offset,\n"
"const int out_offset,\n"
"const float _tau,\n"
"const float _a\n"
")\n"
"{\n"
"int gid = get_global_id(0);\n"
"if(gid >= rows)\n"
"{\n"
"return;\n"
"}\n"
"global const float * iptr =\n"
"input  + mad24(gid, elements_per_row, in_offset / 4);\n"
"global float * optr =\n"
"output + mad24(gid, elements_per_row, out_offset / 4);\n"
"float res;\n"
"float4 in_v4, out_v4, sum_v4, res_v4 = (float4)(0);\n"
"for(int i = 0; i < cols / 4; ++i, iptr += 4, optr += 4)\n"
"{\n"
"in_v4  = vload4(0, iptr);\n"
"out_v4 = vload4(0, optr) * _tau;\n"
"sum_v4 = in_v4 + out_v4;\n"
"res_v4.x = sum_v4.x + _a * res_v4.w;\n"
"res_v4.y = sum_v4.y + _a * res_v4.x;\n"
"res_v4.z = sum_v4.z + _a * res_v4.y;\n"
"res_v4.w = sum_v4.w + _a * res_v4.z;\n"
"vstore4(res_v4, 0, optr);\n"
"}\n"
"optr = output + mad24(gid + 1, elements_per_row, -4 + out_offset / 4);\n"
"res_v4 = (float4)(0);\n"
"for(int i = 0; i < elements_per_row / 4; ++i, optr -= 4)\n"
"{\n"
"out_v4 = vload4(0, optr);\n"
"res_v4.w = out_v4.w + _a * res_v4.x;\n"
"res_v4.z = out_v4.z + _a * res_v4.w;\n"
"res_v4.y = out_v4.y + _a * res_v4.z;\n"
"res_v4.x = out_v4.x + _a * res_v4.y;\n"
"vstore4(res_v4, 0, optr);\n"
"}\n"
"}\n"
"kernel void verticalCausalFilter(\n"
"global float * output,\n"
"const int cols,\n"
"const int rows,\n"
"const int elements_per_row,\n"
"const int out_offset,\n"
"const float _a,\n"
"const float _gain\n"
")\n"
"{\n"
"int gid = get_global_id(0) * 2;\n"
"if(gid >= cols)\n"
"{\n"
"return;\n"
"}\n"
"global float * optr = output + gid + out_offset / 4;\n"
"float2 input;\n"
"float2 result = (float2)0;\n"
"for(int i = 0; i < rows; ++i, optr += elements_per_row)\n"
"{\n"
"input = vload2(0, optr);\n"
"result = input + _a * result;\n"
"vstore2(result, 0, optr);\n"
"}\n"
"optr = output + (rows - 1) * elements_per_row + gid + out_offset / 4;\n"
"result = (float2)0;\n"
"for(int i = 0; i < rows; ++i, optr -= elements_per_row)\n"
"{\n"
"input = vload2(0, optr);\n"
"result = input + _a * result;\n"
"vstore2(_gain * result, 0, optr);\n"
"}\n"
"}\n"
"kernel void verticalCausalFilter_multichannel(\n"
"global float * output,\n"
"const int cols,\n"
"const int rows,\n"
"const int elements_per_row,\n"
"const int out_offset,\n"
"const float _a,\n"
"const float _gain\n"
")\n"
"{\n"
"int gid = get_global_id(0) * 2;\n"
"if(gid >= cols)\n"
"{\n"
"return;\n"
"}\n"
"global float * optr[3];\n"
"float2 input[3];\n"
"float2 result[3] = { (float2)0, (float2)0, (float2)0 };\n"
"optr[0] = output + gid + out_offset / 4;\n"
"optr[1] = output + gid + out_offset / 4 + rows * elements_per_row;\n"
"optr[2] = output + gid + out_offset / 4 + 2 * rows * elements_per_row;\n"
"for(int i = 0; i < rows; ++i)\n"
"{\n"
"input[0] = vload2(0, optr[0]);\n"
"input[1] = vload2(0, optr[1]);\n"
"input[2] = vload2(0, optr[2]);\n"
"result[0] = input[0] + _a * result[0];\n"
"result[1] = input[1] + _a * result[1];\n"
"result[2] = input[2] + _a * result[2];\n"
"vstore2(result[0], 0, optr[0]);\n"
"vstore2(result[1], 0, optr[1]);\n"
"vstore2(result[2], 0, optr[2]);\n"
"optr[0] += elements_per_row;\n"
"optr[1] += elements_per_row;\n"
"optr[2] += elements_per_row;\n"
"}\n"
"optr[0] = output + (rows - 1) * elements_per_row + gid + out_offset / 4;\n"
"optr[1] = output + (rows - 1) * elements_per_row + gid + out_offset / 4 + rows * elements_per_row;\n"
"optr[2] = output + (rows - 1) * elements_per_row + gid + out_offset / 4 + 2 * rows * elements_per_row;\n"
"result[0] = result[1] = result[2] = (float2)0;\n"
"for(int i = 0; i < rows; ++i)\n"
"{\n"
"input[0] = vload2(0, optr[0]);\n"
"input[1] = vload2(0, optr[1]);\n"
"input[2] = vload2(0, optr[2]);\n"
"result[0] = input[0] + _a * result[0];\n"
"result[1] = input[1] + _a * result[1];\n"
"result[2] = input[2] + _a * result[2];\n"
"vstore2(_gain * result[0], 0, optr[0]);\n"
"vstore2(_gain * result[1], 0, optr[1]);\n"
"vstore2(_gain * result[2], 0, optr[2]);\n"
"optr[0] -= elements_per_row;\n"
"optr[1] -= elements_per_row;\n"
"optr[2] -= elements_per_row;\n"
"}\n"
"}\n"
"kernel void verticalCausalFilter_Irregular(\n"
"global float * output,\n"
"global float * buffer,\n"
"const int cols,\n"
"const int rows,\n"
"const int elements_per_row,\n"
"const int out_offset,\n"
"const int buffer_offset,\n"
"const float gain\n"
")\n"
"{\n"
"int gid = get_global_id(0) * 2;\n"
"if(gid >= cols)\n"
"{\n"
"return;\n"
"}\n"
"global float * optr[3];\n"
"global float * bptr = buffer + gid + buffer_offset / 4;\n"
"float2 result[3] = { (float2)0, (float2)0, (float2)0 };\n"
"float2 grad, input[3];\n"
"optr[0] = output + gid + out_offset / 4;\n"
"optr[1] = output + gid + out_offset / 4 + rows * elements_per_row;\n"
"optr[2] = output + gid + out_offset / 4 + 2 * rows * elements_per_row;\n"
"for(int i = 0; i < rows; ++i, bptr += elements_per_row)\n"
"{\n"
"input[0] = vload2(0, optr[0]);\n"
"input[1] = vload2(0, optr[1]);\n"
"input[2] = vload2(0, optr[2]);\n"
"grad = vload2(0, bptr);\n"
"result[0] = input[0] + grad * result[0];\n"
"result[1] = input[1] + grad * result[1];\n"
"result[2] = input[2] + grad * result[2];\n"
"vstore2(result[0], 0, optr[0]);\n"
"vstore2(result[1], 0, optr[1]);\n"
"vstore2(result[2], 0, optr[2]);\n"
"optr[0] += elements_per_row;\n"
"optr[1] += elements_per_row;\n"
"optr[2] += elements_per_row;\n"
"}\n"
"int start_idx = mad24(rows - 1, elements_per_row, gid);\n"
"optr[0] = output + start_idx + out_offset / 4;\n"
"optr[1] = output + start_idx + out_offset / 4 + rows * elements_per_row;\n"
"optr[2] = output + start_idx + out_offset / 4 + 2 * rows * elements_per_row;\n"
"bptr = buffer + start_idx + buffer_offset / 4;\n"
"result[0] = result[1] = result[2] = (float2)0;\n"
"for(int i = 0; i < rows; ++i, bptr -= elements_per_row)\n"
"{\n"
"input[0] = vload2(0, optr[0]);\n"
"input[1] = vload2(0, optr[1]);\n"
"input[2] = vload2(0, optr[2]);\n"
"grad = vload2(0, bptr);\n"
"result[0] = input[0] + grad * result[0];\n"
"result[1] = input[1] + grad * result[1];\n"
"result[2] = input[2] + grad * result[2];\n"
"vstore2(gain * result[0], 0, optr[0]);\n"
"vstore2(gain * result[1], 0, optr[1]);\n"
"vstore2(gain * result[2], 0, optr[2]);\n"
"optr[0] -= elements_per_row;\n"
"optr[1] -= elements_per_row;\n"
"optr[2] -= elements_per_row;\n"
"}\n"
"}\n"
"kernel void adaptiveHorizontalCausalFilter_addInput(\n"
"global const float * input,\n"
"global const float * gradient,\n"
"global float * output,\n"
"const int cols,\n"
"const int rows,\n"
"const int elements_per_row,\n"
"const int in_offset,\n"
"const int grad_offset,\n"
"const int out_offset\n"
")\n"
"{\n"
"int gid = get_global_id(0);\n"
"if(gid >= rows)\n"
"{\n"
"return;\n"
"}\n"
"global const float * iptr =\n"
"input + mad24(gid, elements_per_row, in_offset / 4);\n"
"global const float * gptr =\n"
"gradient + mad24(gid, elements_per_row, grad_offset / 4);\n"
"global float * optr =\n"
"output + mad24(gid, elements_per_row, out_offset / 4);\n"
"float4 in_v4, grad_v4, out_v4, res_v4 = (float4)(0);\n"
"for(int i = 0; i < cols / 4; ++i, iptr += 4, gptr += 4, optr += 4)\n"
"{\n"
"in_v4   = vload4(0, iptr);\n"
"grad_v4 = vload4(0, gptr);\n"
"res_v4.x = in_v4.x + grad_v4.x * res_v4.w;\n"
"res_v4.y = in_v4.y + grad_v4.y * res_v4.x;\n"
"res_v4.z = in_v4.z + grad_v4.z * res_v4.y;\n"
"res_v4.w = in_v4.w + grad_v4.w * res_v4.z;\n"
"vstore4(res_v4, 0, optr);\n"
"}\n"
"optr = output + mad24(gid + 1, elements_per_row, -4 + out_offset / 4);\n"
"gptr = gradient + mad24(gid + 1, elements_per_row, -4 + grad_offset / 4);\n"
"res_v4 = (float4)(0);\n"
"for(int i = 0; i < cols / 4; ++i, gptr -= 4, optr -= 4)\n"
"{\n"
"grad_v4 = vload4(0, gptr);\n"
"out_v4 = vload4(0, optr);\n"
"res_v4.w = out_v4.w + grad_v4.w * res_v4.x;\n"
"res_v4.z = out_v4.z + grad_v4.z * res_v4.w;\n"
"res_v4.y = out_v4.y + grad_v4.y * res_v4.z;\n"
"res_v4.x = out_v4.x + grad_v4.x * res_v4.y;\n"
"vstore4(res_v4, 0, optr);\n"
"}\n"
"}\n"
"kernel void localLuminanceAdaptation(\n"
"global const float * luma,\n"
"global const float * input,\n"
"global float * output,\n"
"const int cols,\n"
"const int rows,\n"
"const int elements_per_row,\n"
"const float _localLuminanceAddon,\n"
"const float _localLuminanceFactor,\n"
"const float _maxInputValue\n"
")\n"
"{\n"
"int gidx = get_global_id(0) * 4, gidy = get_global_id(1);\n"
"if(gidx >= cols || gidy >= rows)\n"
"{\n"
"return;\n"
"}\n"
"int offset = mad24(gidy, elements_per_row, gidx);\n"
"float4 luma_vec = vload4(0, luma + offset);\n"
"float4 X0 = luma_vec * _localLuminanceFactor + _localLuminanceAddon;\n"
"float4 input_val = vload4(0, input + offset);\n"
"float4 out_vec = (_maxInputValue + X0) * input_val / (input_val + X0 + 0.00000000001f);\n"
"vstore4(out_vec, 0, output + offset);\n"
"}\n"
"kernel void amacrineCellsComputing(\n"
"global const float * opl_on,\n"
"global const float * opl_off,\n"
"global float * prev_in_on,\n"
"global float * prev_in_off,\n"
"global float * out_on,\n"
"global float * out_off,\n"
"const int cols,\n"
"const int rows,\n"
"const int elements_per_row,\n"
"const float coeff\n"
")\n"
"{\n"
"int gidx = get_global_id(0) * 4, gidy = get_global_id(1);\n"
"if(gidx >= cols || gidy >= rows)\n"
"{\n"
"return;\n"
"}\n"
"int offset = mad24(gidy, elements_per_row, gidx);\n"
"opl_on      += offset;\n"
"opl_off     += offset;\n"
"prev_in_on  += offset;\n"
"prev_in_off += offset;\n"
"out_on      += offset;\n"
"out_off     += offset;\n"
"float4 val_opl_on = vload4(0, opl_on);\n"
"float4 val_opl_off = vload4(0, opl_off);\n"
"float4 magnoXonPixelResult = coeff * (vload4(0, out_on) + val_opl_on - vload4(0, prev_in_on));\n"
"vstore4(fmax(magnoXonPixelResult, 0.f), 0, out_on);\n"
"float4 magnoXoffPixelResult = coeff * (vload4(0, out_off) + val_opl_off - vload4(0, prev_in_off));\n"
"vstore4(fmax(magnoXoffPixelResult, 0.f), 0, out_off);\n"
"vstore4(val_opl_on, 0, prev_in_on);\n"
"vstore4(val_opl_off, 0, prev_in_off);\n"
"}\n"
"kernel void OPL_OnOffWaysComputing(\n"
"global float4 * photo_out,\n"
"global float4 * horiz_out,\n"
"global float4 * bipol_on,\n"
"global float4 * bipol_off,\n"
"global float4 * parvo_on,\n"
"global float4 * parvo_off,\n"
"const int cols,\n"
"const int rows,\n"
"const int elements_per_row\n"
")\n"
"{\n"
"int gidx = get_global_id(0), gidy = get_global_id(1);\n"
"if(gidx * 4 >= cols || gidy >= rows)\n"
"{\n"
"return;\n"
"}\n"
"int offset = mad24(gidy, elements_per_row >> 2, gidx);\n"
"photo_out += offset;\n"
"horiz_out += offset;\n"
"bipol_on  += offset;\n"
"bipol_off += offset;\n"
"parvo_on  += offset;\n"
"parvo_off += offset;\n"
"float4 diff = *photo_out - *horiz_out;\n"
"float4 isPositive = convert_float4(abs(diff > (float4)0.0f));\n"
"float4 res_on  = isPositive * diff;\n"
"float4 res_off = (isPositive - (float4)(1.0f)) * diff;\n"
"*bipol_on = res_on;\n"
"*parvo_on = res_on;\n"
"*bipol_off = res_off;\n"
"*parvo_off = res_off;\n"
"}\n"
"inline int bayerSampleOffset(int step, int rows, int x, int y)\n"
"{\n"
"return mad24(y, step, x) +\n"
"((y % 2) + (x % 2)) * rows * step;\n"
"}\n"
"kernel void runColorMultiplexingBayer(\n"
"global const float * input,\n"
"global float * output,\n"
"const int cols,\n"
"const int rows,\n"
"const int elements_per_row\n"
")\n"
"{\n"
"int gidx = get_global_id(0) * 4, gidy = get_global_id(1);\n"
"if(gidx >= cols || gidy >= rows)\n"
"{\n"
"return;\n"
"}\n"
"int offset = mad24(gidy, elements_per_row, gidx);\n"
"float4 val;\n"
"val.x = input[bayerSampleOffset(elements_per_row, rows, gidx + 0, gidy)];\n"
"val.y = input[bayerSampleOffset(elements_per_row, rows, gidx + 1, gidy)];\n"
"val.z = input[bayerSampleOffset(elements_per_row, rows, gidx + 2, gidy)];\n"
"val.w = input[bayerSampleOffset(elements_per_row, rows, gidx + 3, gidy)];\n"
"vstore4(val, 0, output + offset);\n"
"}\n"
"kernel void runColorDemultiplexingBayer(\n"
"global const float * input,\n"
"global float * output,\n"
"const int cols,\n"
"const int rows,\n"
"const int elements_per_row\n"
")\n"
"{\n"
"int gidx = get_global_id(0) * 4, gidy = get_global_id(1);\n"
"if(gidx >= cols || gidy >= rows)\n"
"{\n"
"return;\n"
"}\n"
"int offset = mad24(gidy, elements_per_row, gidx);\n"
"float4 val = vload4(0, input + offset);\n"
"output[bayerSampleOffset(elements_per_row, rows, gidx + 0, gidy)] = val.x;\n"
"output[bayerSampleOffset(elements_per_row, rows, gidx + 1, gidy)] = val.y;\n"
"output[bayerSampleOffset(elements_per_row, rows, gidx + 2, gidy)] = val.z;\n"
"output[bayerSampleOffset(elements_per_row, rows, gidx + 3, gidy)] = val.w;\n"
"}\n"
"kernel void demultiplexAssign(\n"
"global const float * input,\n"
"global float * output,\n"
"const int cols,\n"
"const int rows,\n"
"const int elements_per_row\n"
")\n"
"{\n"
"int gidx = get_global_id(0), gidy = get_global_id(1);\n"
"if(gidx >= cols || gidy >= rows)\n"
"{\n"
"return;\n"
"}\n"
"int offset = bayerSampleOffset(elements_per_row, rows, gidx, gidy);\n"
"output[offset] = input[offset];\n"
"}\n"
"kernel void normalizeGrayOutputCentredSigmoide(\n"
"global const float * input,\n"
"global float * output,\n"
"const int cols,\n"
"const int rows,\n"
"const int elements_per_row,\n"
"const float meanval,\n"
"const float X0\n"
")\n"
"{\n"
"int gidx = get_global_id(0) * 4, gidy = get_global_id(1);\n"
"if(gidx >= cols || gidy >= rows)\n"
"{\n"
"return;\n"
"}\n"
"int offset = mad24(gidy, elements_per_row, gidx);\n"
"float4 input_val = vload4(0, input + offset);\n"
"input_val =  meanval + (meanval + X0) * (input_val - meanval) / (fabs(input_val - meanval) + X0);\n"
"vstore4(input_val, 0, output + offset);\n"
"}\n"
"kernel void normalizePhotoDensity(\n"
"global const float * chroma,\n"
"global const float * colorDensity,\n"
"global const float * multiplex,\n"
"global float * luma,\n"
"global float * demultiplex,\n"
"const int cols,\n"
"const int rows,\n"
"const int elements_per_row,\n"
"const float pG\n"
")\n"
"{\n"
"const int gidx = get_global_id(0) * 4, gidy = get_global_id(1);\n"
"if(gidx >= cols || gidy >= rows)\n"
"{\n"
"return;\n"
"}\n"
"const int offset = mad24(gidy, elements_per_row, gidx);\n"
"int index = offset;\n"
"float4 Cr = vload4(0, chroma + index) * vload4(0, colorDensity + index);\n"
"index += elements_per_row * rows;\n"
"float4 Cg = vload4(0, chroma + index) * vload4(0, colorDensity + index);\n"
"index += elements_per_row * rows;\n"
"float4 Cb = vload4(0, chroma + index) * vload4(0, colorDensity + index);\n"
"const float4 luma_res = (Cr + Cg + Cb) * pG;\n"
"vstore4(luma_res, 0, luma + offset);\n"
"float4 res_v4 = vload4(0, multiplex + offset) - luma_res;\n"
"demultiplex[bayerSampleOffset(elements_per_row, rows, gidx + 0, gidy)] = res_v4.x;\n"
"demultiplex[bayerSampleOffset(elements_per_row, rows, gidx + 1, gidy)] = res_v4.y;\n"
"demultiplex[bayerSampleOffset(elements_per_row, rows, gidx + 2, gidy)] = res_v4.z;\n"
"demultiplex[bayerSampleOffset(elements_per_row, rows, gidx + 3, gidy)] = res_v4.w;\n"
"}\n"
"kernel void computeGradient(\n"
"global const float * luma,\n"
"global float * gradient,\n"
"const int cols,\n"
"const int rows,\n"
"const int elements_per_row\n"
")\n"
"{\n"
"int gidx = get_global_id(0) + 2, gidy = get_global_id(1) + 2;\n"
"if(gidx >= cols - 2 || gidy >= rows - 2)\n"
"{\n"
"return;\n"
"}\n"
"int offset = mad24(gidy, elements_per_row, gidx);\n"
"luma += offset;\n"
"const float v_grad = fabs(luma[elements_per_row] - luma[- elements_per_row]);\n"
"const float h_grad = fabs(luma[1] - luma[-1]);\n"
"const float cur_val  = luma[0];\n"
"const float v_grad_p = fabs(cur_val - luma[- 2 * elements_per_row]);\n"
"const float h_grad_p = fabs(cur_val - luma[- 2]);\n"
"const float v_grad_n = fabs(cur_val - luma[2 * elements_per_row]);\n"
"const float h_grad_n = fabs(cur_val - luma[2]);\n"
"const float horiz_grad = 0.5f * h_grad + 0.25f * (h_grad_p + h_grad_n);\n"
"const float verti_grad = 0.5f * v_grad + 0.25f * (v_grad_p + v_grad_n);\n"
"const bool is_vertical_greater = (horiz_grad < verti_grad) &&\n"
"((verti_grad - horiz_grad) > 1e-5);\n"
"gradient[offset + elements_per_row * rows] = is_vertical_greater ? 0.06f : 0.57f;\n"
"gradient[offset                          ] = is_vertical_greater ? 0.57f : 0.06f;\n"
"}\n"
"kernel void substractResidual(\n"
"global float * input,\n"
"const int cols,\n"
"const int rows,\n"
"const int elements_per_row,\n"
"const float pR,\n"
"const float pG,\n"
"const float pB\n"
")\n"
"{\n"
"const int gidx = get_global_id(0) * 4, gidy = get_global_id(1);\n"
"if(gidx >= cols || gidy >= rows)\n"
"{\n"
"return;\n"
"}\n"
"int indices [3] =\n"
"{\n"
"mad24(gidy, elements_per_row, gidx),\n"
"mad24(gidy + rows, elements_per_row, gidx),\n"
"mad24(gidy + 2 * rows, elements_per_row, gidx)\n"
"};\n"
"float4 vals[3];\n"
"vals[0] = vload4(0, input + indices[0]);\n"
"vals[1] = vload4(0, input + indices[1]);\n"
"vals[2] = vload4(0, input + indices[2]);\n"
"float4 residu = pR * vals[0] + pG * vals[1] + pB * vals[2];\n"
"vstore4(vals[0] - residu, 0, input + indices[0]);\n"
"vstore4(vals[1] - residu, 0, input + indices[1]);\n"
"vstore4(vals[2] - residu, 0, input + indices[2]);\n"
"}\n"
"kernel void clipRGBOutput_0_maxInputValue(\n"
"global float * input,\n"
"const int cols,\n"
"const int rows,\n"
"const int elements_per_row,\n"
"const float maxVal\n"
")\n"
"{\n"
"const int gidx = get_global_id(0) * 4, gidy = get_global_id(1);\n"
"if(gidx >= cols || gidy >= rows)\n"
"{\n"
"return;\n"
"}\n"
"const int offset = mad24(gidy, elements_per_row, gidx);\n"
"float4 val = vload4(0, input + offset);\n"
"val = clamp(val, 0.0f, maxVal);\n"
"vstore4(val, 0, input + offset);\n"
"}\n"
"kernel void normalizeGrayOutputNearZeroCentreredSigmoide(\n"
"global float * input,\n"
"global float * output,\n"
"const int cols,\n"
"const int rows,\n"
"const int elements_per_row,\n"
"const float maxVal,\n"
"const float X0cube\n"
")\n"
"{\n"
"const int gidx = get_global_id(0) * 4, gidy = get_global_id(1);\n"
"if(gidx >= cols || gidy >= rows)\n"
"{\n"
"return;\n"
"}\n"
"const int offset = mad24(gidy, elements_per_row, gidx);\n"
"float4 currentCubeLuminance = vload4(0, input + offset);\n"
"currentCubeLuminance = currentCubeLuminance * currentCubeLuminance * currentCubeLuminance;\n"
"float4 val = currentCubeLuminance * X0cube / (X0cube + currentCubeLuminance);\n"
"vstore4(val, 0, output + offset);\n"
"}\n"
"kernel void centerReductImageLuminance(\n"
"global float * input,\n"
"const int cols,\n"
"const int rows,\n"
"const int elements_per_row,\n"
"const float mean,\n"
"const float std_dev\n"
")\n"
"{\n"
"const int gidx = get_global_id(0) * 4, gidy = get_global_id(1);\n"
"if(gidx >= cols || gidy >= rows)\n"
"{\n"
"return;\n"
"}\n"
"const int offset = mad24(gidy, elements_per_row, gidx);\n"
"float4 val = vload4(0, input + offset);\n"
"val = (val - mean) / std_dev;\n"
"vstore4(val, 0, input + offset);\n"
"}\n"
"kernel void inverseValue(\n"
"global float * input,\n"
"const int cols,\n"
"const int rows,\n"
"const int elements_per_row\n"
")\n"
"{\n"
"const int gidx = get_global_id(0) * 4, gidy = get_global_id(1);\n"
"if(gidx >= cols || gidy >= rows)\n"
"{\n"
"return;\n"
"}\n"
"const int offset = mad24(gidy, elements_per_row, gidx);\n"
"float4 val = vload4(0, input + offset);\n"
"val = 1.f / val;\n"
"vstore4(val, 0, input + offset);\n"
"}\n"
"#define CV_PI 3.1415926535897932384626433832795\n"
"kernel void processRetinaParvoMagnoMapping(\n"
"global float * parvo,\n"
"global float * magno,\n"
"global float * output,\n"
"const int cols,\n"
"const int rows,\n"
"const int halfCols,\n"
"const int halfRows,\n"
"const int elements_per_row,\n"
"const float minDistance\n"
")\n"
"{\n"
"const int gidx = get_global_id(0), gidy = get_global_id(1);\n"
"if(gidx >= cols || gidy >= rows)\n"
"{\n"
"return;\n"
"}\n"
"const int offset = mad24(gidy, elements_per_row, gidx);\n"
"float distanceToCenter =\n"
"sqrt(((float)(gidy - halfRows) * (gidy - halfRows) + (gidx - halfCols) * (gidx - halfCols)));\n"
"float a = distanceToCenter < minDistance ?\n"
"(0.5f + 0.5f * (float)cos(CV_PI * distanceToCenter / minDistance)) : 0;\n"
"float b = 1.f - a;\n"
"output[offset] = parvo[offset] * a + magno[offset] * b;\n"
"}\n"
, "0ad5b79c0e354633fa613f1b5c1b5d01", NULL};

}}}
#endif
