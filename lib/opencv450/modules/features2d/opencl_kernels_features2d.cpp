// This file is auto-generated. Do not edit!

#include "opencv2/core.hpp"
#include "cvconfig.h"
#include "opencl_kernels_features2d.hpp"

#ifdef HAVE_OPENCL

namespace cv
{
namespace ocl
{
namespace features2d
{

static const char* const moduleName = "features2d";

struct cv::ocl::internal::ProgramEntry akaze_oclsrc={moduleName, "akaze",
"__kernel void\n"
"AKAZE_pm_g2(__global const float* lx, __global const float* ly, __global float* dst,\n"
"float k, int size)\n"
"{\n"
"int i = get_global_id(0);\n"
"if (!(i < size))\n"
"{\n"
"return;\n"
"}\n"
"const float k2inv = 1.0f / (k * k);\n"
"dst[i] = 1.0f / (1.0f + ((lx[i] * lx[i] + ly[i] * ly[i]) * k2inv));\n"
"}\n"
"__kernel void\n"
"AKAZE_nld_step_scalar(__global const float* lt, int lt_step, int lt_offset, int rows, int cols,\n"
"__global const float* lf, __global float* dst, float step_size)\n"
"{\n"
"int i = get_global_id(1);\n"
"int j = get_global_id(0);\n"
"if (!(i < rows && j < cols))\n"
"{\n"
"return;\n"
"}\n"
"int a = (i - 1) * cols;\n"
"int c = (i    ) * cols;\n"
"int b = (i + 1) * cols;\n"
"float res = 0.0f;\n"
"if (i == 0)\n"
"{\n"
"if (j == 0 || j == (cols - 1))\n"
"{\n"
"res = 0.0f;\n"
"} else\n"
"{\n"
"res = (lf[c + j] + lf[c + j + 1])*(lt[c + j + 1] - lt[c + j]) +\n"
"(lf[c + j] + lf[c + j - 1])*(lt[c + j - 1] - lt[c + j]) +\n"
"(lf[c + j] + lf[b + j    ])*(lt[b + j    ] - lt[c + j]);\n"
"}\n"
"} else if (i == (rows - 1))\n"
"{\n"
"if (j == 0 || j == (cols - 1))\n"
"{\n"
"res = 0.0f;\n"
"} else\n"
"{\n"
"res = (lf[c + j] + lf[c + j + 1])*(lt[c + j + 1] - lt[c + j]) +\n"
"(lf[c + j] + lf[c + j - 1])*(lt[c + j - 1] - lt[c + j]) +\n"
"(lf[c + j] + lf[a + j    ])*(lt[a + j    ] - lt[c + j]);\n"
"}\n"
"} else\n"
"{\n"
"if (j == 0)\n"
"{\n"
"res = (lf[c + 0] + lf[c + 1])*(lt[c + 1] - lt[c + 0]) +\n"
"(lf[c + 0] + lf[b + 0])*(lt[b + 0] - lt[c + 0]) +\n"
"(lf[c + 0] + lf[a + 0])*(lt[a + 0] - lt[c + 0]);\n"
"} else if (j == (cols - 1))\n"
"{\n"
"res = (lf[c + j] + lf[c + j - 1])*(lt[c + j - 1] - lt[c + j]) +\n"
"(lf[c + j] + lf[b + j    ])*(lt[b + j    ] - lt[c + j]) +\n"
"(lf[c + j] + lf[a + j    ])*(lt[a + j    ] - lt[c + j]);\n"
"} else\n"
"{\n"
"res = (lf[c + j] + lf[c + j + 1])*(lt[c + j + 1] - lt[c + j]) +\n"
"(lf[c + j] + lf[c + j - 1])*(lt[c + j - 1] - lt[c + j]) +\n"
"(lf[c + j] + lf[b + j    ])*(lt[b + j    ] - lt[c + j]) +\n"
"(lf[c + j] + lf[a + j    ])*(lt[a + j    ] - lt[c + j]);\n"
"}\n"
"}\n"
"dst[c + j] = res * step_size;\n"
"}\n"
"__kernel void\n"
"AKAZE_compute_determinant(__global const float* lxx, __global const float* lxy, __global const float* lyy,\n"
"__global float* dst, float sigma, int size)\n"
"{\n"
"int i = get_global_id(0);\n"
"if (!(i < size))\n"
"{\n"
"return;\n"
"}\n"
"dst[i] = (lxx[i] * lyy[i] - lxy[i] * lxy[i]) * sigma;\n"
"}\n"
, "80f6cd2f334b70062ed64a0a1a866593", NULL};
struct cv::ocl::internal::ProgramEntry brute_force_match_oclsrc={moduleName, "brute_force_match",
"#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics:enable\n"
"#define MAX_FLOAT 3.40282e+038f\n"
"#ifndef T\n"
"#define T float\n"
"#endif\n"
"#ifndef BLOCK_SIZE\n"
"#define BLOCK_SIZE 16\n"
"#endif\n"
"#ifndef MAX_DESC_LEN\n"
"#define MAX_DESC_LEN 64\n"
"#endif\n"
"#define BLOCK_SIZE_ODD          (BLOCK_SIZE + 1)\n"
"#ifndef SHARED_MEM_SZ\n"
"#  if (BLOCK_SIZE < MAX_DESC_LEN)\n"
"#    define SHARED_MEM_SZ      (kercn * (BLOCK_SIZE * MAX_DESC_LEN + BLOCK_SIZE * BLOCK_SIZE))\n"
"#  else\n"
"#    define SHARED_MEM_SZ      (kercn * 2 * BLOCK_SIZE_ODD * BLOCK_SIZE)\n"
"#  endif\n"
"#endif\n"
"#ifndef DIST_TYPE\n"
"#define DIST_TYPE 2\n"
"#endif\n"
"#if (DIST_TYPE == 2)\n"
"#   ifdef T_FLOAT\n"
"typedef float result_type;\n"
"#       if (8 == kercn)\n"
"typedef float8 value_type;\n"
"#           define DIST(x, y) {value_type d = fabs((x) - (y)); result += d.s0 + d.s1 + d.s2 + d.s3 + d.s4 + d.s5 + d.s6 + d.s7;}\n"
"#       elif (4 == kercn)\n"
"typedef float4 value_type;\n"
"#           define DIST(x, y) {value_type d = fabs((x) - (y)); result += d.s0 + d.s1 + d.s2 + d.s3;}\n"
"#       else\n"
"typedef float value_type;\n"
"#           define DIST(x, y) result += fabs((x) - (y))\n"
"#       endif\n"
"#   else\n"
"typedef int result_type;\n"
"#       if (8 == kercn)\n"
"typedef int8 value_type;\n"
"#           define DIST(x, y) {value_type d = abs((x) - (y)); result += d.s0 + d.s1 + d.s2 + d.s3 + d.s4 + d.s5 + d.s6 + d.s7;}\n"
"#       elif (4 == kercn)\n"
"typedef int4 value_type;\n"
"#           define DIST(x, y) {value_type d = abs((x) - (y)); result += d.s0 + d.s1 + d.s2 + d.s3;}\n"
"#       else\n"
"typedef int  value_type;\n"
"#           define DIST(x, y) result += abs((x) - (y))\n"
"#       endif\n"
"#   endif\n"
"#   define DIST_RES(x) (x)\n"
"#elif (DIST_TYPE == 4)\n"
"typedef float result_type;\n"
"#   if (8 == kercn)\n"
"typedef float8 value_type;\n"
"#       define DIST(x, y)   {value_type d = ((x) - (y)); result += dot(d.s0123, d.s0123) + dot(d.s4567, d.s4567);}\n"
"#   elif (4 == kercn)\n"
"typedef float4      value_type;\n"
"#       define DIST(x, y)   {value_type d = ((x) - (y)); result += dot(d, d);}\n"
"#   else\n"
"typedef float       value_type;\n"
"#       define DIST(x, y)   {value_type d = ((x) - (y)); result = mad(d, d, result);}\n"
"#   endif\n"
"#   define DIST_RES(x) sqrt(x)\n"
"#elif (DIST_TYPE == 6)\n"
"#   if (8 == kercn)\n"
"typedef int8 value_type;\n"
"#   elif (4 == kercn)\n"
"typedef int4 value_type;\n"
"#   else\n"
"typedef int value_type;\n"
"#   endif\n"
"typedef int result_type;\n"
"#   define DIST(x, y) result += popcount( (x) ^ (y) )\n"
"#   define DIST_RES(x) (x)\n"
"#endif\n"
"inline result_type reduce_block(\n"
"__local value_type *s_query,\n"
"__local value_type *s_train,\n"
"int lidx,\n"
"int lidy\n"
")\n"
"{\n"
"result_type result = 0;\n"
"#pragma unroll\n"
"for (int j = 0 ; j < BLOCK_SIZE ; j++)\n"
"{\n"
"DIST(s_query[lidy * BLOCK_SIZE_ODD + j], s_train[j * BLOCK_SIZE_ODD + lidx]);\n"
"}\n"
"return DIST_RES(result);\n"
"}\n"
"inline result_type reduce_block_match(\n"
"__local value_type *s_query,\n"
"__local value_type *s_train,\n"
"int lidx,\n"
"int lidy\n"
")\n"
"{\n"
"result_type result = 0;\n"
"#pragma unroll\n"
"for (int j = 0 ; j < BLOCK_SIZE ; j++)\n"
"{\n"
"DIST(s_query[lidy * BLOCK_SIZE_ODD + j], s_train[j * BLOCK_SIZE_ODD + lidx]);\n"
"}\n"
"return result;\n"
"}\n"
"inline result_type reduce_multi_block(\n"
"__local value_type *s_query,\n"
"__local value_type *s_train,\n"
"int block_index,\n"
"int lidx,\n"
"int lidy\n"
")\n"
"{\n"
"result_type result = 0;\n"
"#pragma unroll\n"
"for (int j = 0 ; j < BLOCK_SIZE ; j++)\n"
"{\n"
"DIST(s_query[lidy * MAX_DESC_LEN + block_index * BLOCK_SIZE + j], s_train[j * BLOCK_SIZE + lidx]);\n"
"}\n"
"return result;\n"
"}\n"
"__kernel void BruteForceMatch_Match(\n"
"__global T *query,\n"
"__global T *train,\n"
"__global int *bestTrainIdx,\n"
"__global float *bestDistance,\n"
"int query_rows,\n"
"int query_cols,\n"
"int train_rows,\n"
"int train_cols,\n"
"int step\n"
")\n"
"{\n"
"const int lidx = get_local_id(0);\n"
"const int lidy = get_local_id(1);\n"
"const int groupidx = get_group_id(0);\n"
"const int queryIdx = mad24(BLOCK_SIZE, groupidx, lidy);\n"
"const int queryOffset = min(queryIdx, query_rows - 1) * step;\n"
"__global TN *query_vec = (__global TN *)(query + queryOffset);\n"
"query_cols /= kercn;\n"
"__local float sharebuffer[SHARED_MEM_SZ];\n"
"__local value_type *s_query = (__local value_type *)sharebuffer;\n"
"#if 0 < MAX_DESC_LEN\n"
"__local value_type *s_train = (__local value_type *)sharebuffer + BLOCK_SIZE * MAX_DESC_LEN;\n"
"#pragma unroll\n"
"for (int i = 0; i < MAX_DESC_LEN / BLOCK_SIZE; i++)\n"
"{\n"
"const int loadx = mad24(BLOCK_SIZE, i, lidx);\n"
"s_query[mad24(MAX_DESC_LEN, lidy, loadx)] = loadx < query_cols ? query_vec[loadx] : 0;\n"
"}\n"
"#else\n"
"__local value_type *s_train = (__local value_type *)sharebuffer + BLOCK_SIZE_ODD * BLOCK_SIZE;\n"
"const int s_query_i = mad24(BLOCK_SIZE_ODD, lidy, lidx);\n"
"const int s_train_i = mad24(BLOCK_SIZE_ODD, lidx, lidy);\n"
"#endif\n"
"float myBestDistance = MAX_FLOAT;\n"
"int myBestTrainIdx = -1;\n"
"for (int t = 0, endt = (train_rows + BLOCK_SIZE - 1) / BLOCK_SIZE; t < endt; t++)\n"
"{\n"
"result_type result = 0;\n"
"const int trainOffset = min(mad24(BLOCK_SIZE, t, lidy), train_rows - 1) * step;\n"
"__global TN *train_vec = (__global TN *)(train + trainOffset);\n"
"#if 0 < MAX_DESC_LEN\n"
"#pragma unroll\n"
"for (int i = 0; i < MAX_DESC_LEN / BLOCK_SIZE; i++)\n"
"{\n"
"const int loadx = mad24(BLOCK_SIZE, i, lidx);\n"
"s_train[mad24(BLOCK_SIZE, lidx, lidy)] = loadx < train_cols ? train_vec[loadx] : 0;\n"
"barrier(CLK_LOCAL_MEM_FENCE);\n"
"result += reduce_multi_block(s_query, s_train, i, lidx, lidy);\n"
"barrier(CLK_LOCAL_MEM_FENCE);\n"
"}\n"
"#else\n"
"for (int i = 0, endq = (query_cols + BLOCK_SIZE - 1) / BLOCK_SIZE; i < endq; i++)\n"
"{\n"
"const int loadx = mad24(i, BLOCK_SIZE, lidx);\n"
"if (loadx < query_cols)\n"
"{\n"
"s_query[s_query_i] = query_vec[loadx];\n"
"s_train[s_train_i] = train_vec[loadx];\n"
"}\n"
"else\n"
"{\n"
"s_query[s_query_i] = 0;\n"
"s_train[s_train_i] = 0;\n"
"}\n"
"barrier(CLK_LOCAL_MEM_FENCE);\n"
"result += reduce_block_match(s_query, s_train, lidx, lidy);\n"
"barrier(CLK_LOCAL_MEM_FENCE);\n"
"}\n"
"#endif\n"
"result = DIST_RES(result);\n"
"const int trainIdx = mad24(BLOCK_SIZE, t, lidx);\n"
"if (queryIdx < query_rows && trainIdx < train_rows && result < myBestDistance )\n"
"{\n"
"myBestDistance = result;\n"
"myBestTrainIdx = trainIdx;\n"
"}\n"
"}\n"
"barrier(CLK_LOCAL_MEM_FENCE);\n"
"__local float *s_distance = (__local float *)sharebuffer;\n"
"__local int *s_trainIdx = (__local int *)(sharebuffer + BLOCK_SIZE_ODD * BLOCK_SIZE);\n"
"s_distance += lidy * BLOCK_SIZE_ODD;\n"
"s_trainIdx += lidy * BLOCK_SIZE_ODD;\n"
"s_distance[lidx] = myBestDistance;\n"
"s_trainIdx[lidx] = myBestTrainIdx;\n"
"barrier(CLK_LOCAL_MEM_FENCE);\n"
"#pragma unroll\n"
"for (int k = 0 ; k < BLOCK_SIZE; k++)\n"
"{\n"
"if (myBestDistance > s_distance[k])\n"
"{\n"
"myBestDistance = s_distance[k];\n"
"myBestTrainIdx = s_trainIdx[k];\n"
"}\n"
"}\n"
"if (queryIdx < query_rows && lidx == 0)\n"
"{\n"
"bestTrainIdx[queryIdx] = myBestTrainIdx;\n"
"bestDistance[queryIdx] = myBestDistance;\n"
"}\n"
"}\n"
"__kernel void BruteForceMatch_RadiusMatch(\n"
"__global T *query,\n"
"__global T *train,\n"
"float maxDistance,\n"
"__global int *bestTrainIdx,\n"
"__global float *bestDistance,\n"
"__global int *nMatches,\n"
"int query_rows,\n"
"int query_cols,\n"
"int train_rows,\n"
"int train_cols,\n"
"int bestTrainIdx_cols,\n"
"int step,\n"
"int ostep\n"
")\n"
"{\n"
"const int lidx = get_local_id(0);\n"
"const int lidy = get_local_id(1);\n"
"const int groupidx = get_group_id(0);\n"
"const int groupidy = get_group_id(1);\n"
"const int queryIdx = mad24(BLOCK_SIZE, groupidy, lidy);\n"
"const int queryOffset = min(queryIdx, query_rows - 1) * step;\n"
"__global TN *query_vec = (__global TN *)(query + queryOffset);\n"
"const int trainIdx = mad24(BLOCK_SIZE, groupidx, lidx);\n"
"const int trainOffset = min(mad24(BLOCK_SIZE, groupidx, lidy), train_rows - 1) * step;\n"
"__global TN *train_vec = (__global TN *)(train + trainOffset);\n"
"query_cols /= kercn;\n"
"__local float sharebuffer[SHARED_MEM_SZ];\n"
"__local value_type *s_query = (__local value_type *)sharebuffer;\n"
"__local value_type *s_train = (__local value_type *)sharebuffer + BLOCK_SIZE_ODD * BLOCK_SIZE;\n"
"result_type result = 0;\n"
"const int s_query_i = mad24(BLOCK_SIZE_ODD, lidy, lidx);\n"
"const int s_train_i = mad24(BLOCK_SIZE_ODD, lidx, lidy);\n"
"for (int i = 0 ; i < (query_cols + BLOCK_SIZE - 1) / BLOCK_SIZE ; ++i)\n"
"{\n"
"const int loadx = mad24(BLOCK_SIZE, i, lidx);\n"
"if (loadx < query_cols)\n"
"{\n"
"s_query[s_query_i] = query_vec[loadx];\n"
"s_train[s_train_i] = train_vec[loadx];\n"
"}\n"
"else\n"
"{\n"
"s_query[s_query_i] = 0;\n"
"s_train[s_train_i] = 0;\n"
"}\n"
"barrier(CLK_LOCAL_MEM_FENCE);\n"
"result += reduce_block(s_query, s_train, lidx, lidy);\n"
"barrier(CLK_LOCAL_MEM_FENCE);\n"
"}\n"
"if (queryIdx < query_rows && trainIdx < train_rows && convert_float(result) < maxDistance)\n"
"{\n"
"int ind = atom_inc(nMatches + queryIdx);\n"
"if(ind < bestTrainIdx_cols)\n"
"{\n"
"bestTrainIdx[mad24(queryIdx, ostep, ind)] = trainIdx;\n"
"bestDistance[mad24(queryIdx, ostep, ind)] = result;\n"
"}\n"
"}\n"
"}\n"
"__kernel void BruteForceMatch_knnMatch(\n"
"__global T *query,\n"
"__global T *train,\n"
"__global int2 *bestTrainIdx,\n"
"__global float2 *bestDistance,\n"
"int query_rows,\n"
"int query_cols,\n"
"int train_rows,\n"
"int train_cols,\n"
"int step\n"
")\n"
"{\n"
"const int lidx = get_local_id(0);\n"
"const int lidy = get_local_id(1);\n"
"const int groupidx = get_group_id(0);\n"
"const int queryIdx = mad24(BLOCK_SIZE, groupidx, lidy);\n"
"const int queryOffset = min(queryIdx, query_rows - 1) * step;\n"
"__global TN *query_vec = (__global TN *)(query + queryOffset);\n"
"query_cols /= kercn;\n"
"__local float sharebuffer[SHARED_MEM_SZ];\n"
"__local value_type *s_query = (__local value_type *)sharebuffer;\n"
"#if 0 < MAX_DESC_LEN\n"
"__local value_type *s_train = (__local value_type *)sharebuffer + BLOCK_SIZE * MAX_DESC_LEN;\n"
"#pragma unroll\n"
"for (int i = 0 ;  i <  MAX_DESC_LEN / BLOCK_SIZE; i ++)\n"
"{\n"
"int loadx = mad24(BLOCK_SIZE, i, lidx);\n"
"s_query[mad24(MAX_DESC_LEN, lidy, loadx)] = loadx < query_cols ? query_vec[loadx] : 0;\n"
"}\n"
"#else\n"
"__local value_type *s_train = (__local value_type *)sharebuffer + BLOCK_SIZE_ODD * BLOCK_SIZE;\n"
"const int s_query_i = mad24(BLOCK_SIZE_ODD, lidy, lidx);\n"
"const int s_train_i = mad24(BLOCK_SIZE_ODD, lidx, lidy);\n"
"#endif\n"
"float myBestDistance1 = MAX_FLOAT;\n"
"float myBestDistance2 = MAX_FLOAT;\n"
"int myBestTrainIdx1 = -1;\n"
"int myBestTrainIdx2 = -1;\n"
"for (int t = 0, endt = (train_rows + BLOCK_SIZE - 1) / BLOCK_SIZE; t < endt ; t++)\n"
"{\n"
"result_type result = 0;\n"
"int trainOffset = min(mad24(BLOCK_SIZE, t, lidy), train_rows - 1) * step;\n"
"__global TN *train_vec = (__global TN *)(train + trainOffset);\n"
"#if 0 < MAX_DESC_LEN\n"
"#pragma unroll\n"
"for (int i = 0 ; i < MAX_DESC_LEN / BLOCK_SIZE ; i++)\n"
"{\n"
"const int loadx = mad24(BLOCK_SIZE, i, lidx);\n"
"s_train[mad24(BLOCK_SIZE, lidx, lidy)] = loadx < train_cols ? train_vec[loadx] : 0;\n"
"barrier(CLK_LOCAL_MEM_FENCE);\n"
"result += reduce_multi_block(s_query, s_train, i, lidx, lidy);\n"
"barrier(CLK_LOCAL_MEM_FENCE);\n"
"}\n"
"#else\n"
"for (int i = 0, endq = (query_cols + BLOCK_SIZE -1) / BLOCK_SIZE; i < endq ; i++)\n"
"{\n"
"const int loadx = mad24(BLOCK_SIZE, i, lidx);\n"
"if (loadx < query_cols)\n"
"{\n"
"s_query[s_query_i] = query_vec[loadx];\n"
"s_train[s_train_i] = train_vec[loadx];\n"
"}\n"
"else\n"
"{\n"
"s_query[s_query_i] = 0;\n"
"s_train[s_train_i] = 0;\n"
"}\n"
"barrier(CLK_LOCAL_MEM_FENCE);\n"
"result += reduce_block_match(s_query, s_train, lidx, lidy);\n"
"barrier(CLK_LOCAL_MEM_FENCE);\n"
"}\n"
"#endif\n"
"result = DIST_RES(result);\n"
"const int trainIdx = mad24(BLOCK_SIZE, t, lidx);\n"
"if (queryIdx < query_rows && trainIdx < train_rows)\n"
"{\n"
"if (result < myBestDistance1)\n"
"{\n"
"myBestDistance2 = myBestDistance1;\n"
"myBestTrainIdx2 = myBestTrainIdx1;\n"
"myBestDistance1 = result;\n"
"myBestTrainIdx1 = trainIdx;\n"
"}\n"
"else if (result < myBestDistance2)\n"
"{\n"
"myBestDistance2 = result;\n"
"myBestTrainIdx2 = trainIdx;\n"
"}\n"
"}\n"
"}\n"
"barrier(CLK_LOCAL_MEM_FENCE);\n"
"__local float *s_distance = (__local float *)sharebuffer;\n"
"__local int *s_trainIdx = (__local int *)(sharebuffer + BLOCK_SIZE_ODD * BLOCK_SIZE);\n"
"s_distance += lidy * BLOCK_SIZE_ODD;\n"
"s_trainIdx += lidy * BLOCK_SIZE_ODD;\n"
"s_distance[lidx] = myBestDistance1;\n"
"s_trainIdx[lidx] = myBestTrainIdx1;\n"
"float bestDistance1 = MAX_FLOAT;\n"
"float bestDistance2 = MAX_FLOAT;\n"
"int bestTrainIdx1 = -1;\n"
"int bestTrainIdx2 = -1;\n"
"barrier(CLK_LOCAL_MEM_FENCE);\n"
"if (lidx == 0)\n"
"{\n"
"for (int i = 0 ; i < BLOCK_SIZE ; i++)\n"
"{\n"
"float val = s_distance[i];\n"
"if (val < bestDistance1)\n"
"{\n"
"bestDistance2 = bestDistance1;\n"
"bestTrainIdx2 = bestTrainIdx1;\n"
"bestDistance1 = val;\n"
"bestTrainIdx1 = s_trainIdx[i];\n"
"}\n"
"else if (val < bestDistance2)\n"
"{\n"
"bestDistance2 = val;\n"
"bestTrainIdx2 = s_trainIdx[i];\n"
"}\n"
"}\n"
"}\n"
"barrier(CLK_LOCAL_MEM_FENCE);\n"
"s_distance[lidx] = myBestDistance2;\n"
"s_trainIdx[lidx] = myBestTrainIdx2;\n"
"barrier(CLK_LOCAL_MEM_FENCE);\n"
"if (lidx == 0)\n"
"{\n"
"for (int i = 0 ; i < BLOCK_SIZE ; i++)\n"
"{\n"
"float val = s_distance[i];\n"
"if (val < bestDistance2)\n"
"{\n"
"bestDistance2 = val;\n"
"bestTrainIdx2 = s_trainIdx[i];\n"
"}\n"
"}\n"
"}\n"
"myBestDistance1 = bestDistance1;\n"
"myBestDistance2 = bestDistance2;\n"
"myBestTrainIdx1 = bestTrainIdx1;\n"
"myBestTrainIdx2 = bestTrainIdx2;\n"
"if (queryIdx < query_rows && lidx == 0)\n"
"{\n"
"bestTrainIdx[queryIdx] = (int2)(myBestTrainIdx1, myBestTrainIdx2);\n"
"bestDistance[queryIdx] = (float2)(myBestDistance1, myBestDistance2);\n"
"}\n"
"}\n"
, "35c3a1e231d446e4088561e3604fb94f", NULL};
struct cv::ocl::internal::ProgramEntry fast_oclsrc={moduleName, "fast",
"inline int cornerScore(__global const uchar* img, int step)\n"
"{\n"
"int k, tofs, v = img[0], a0 = 0, b0;\n"
"int d[16];\n"
"#define LOAD2(idx, ofs) \\\n"
"tofs = ofs; d[idx] = (short)(v - img[tofs]); d[idx+8] = (short)(v - img[-tofs])\n"
"LOAD2(0, 3);\n"
"LOAD2(1, -step+3);\n"
"LOAD2(2, -step*2+2);\n"
"LOAD2(3, -step*3+1);\n"
"LOAD2(4, -step*3);\n"
"LOAD2(5, -step*3-1);\n"
"LOAD2(6, -step*2-2);\n"
"LOAD2(7, -step-3);\n"
"#pragma unroll\n"
"for( k = 0; k < 16; k += 2 )\n"
"{\n"
"int a = min((int)d[(k+1)&15], (int)d[(k+2)&15]);\n"
"a = min(a, (int)d[(k+3)&15]);\n"
"a = min(a, (int)d[(k+4)&15]);\n"
"a = min(a, (int)d[(k+5)&15]);\n"
"a = min(a, (int)d[(k+6)&15]);\n"
"a = min(a, (int)d[(k+7)&15]);\n"
"a = min(a, (int)d[(k+8)&15]);\n"
"a0 = max(a0, min(a, (int)d[k&15]));\n"
"a0 = max(a0, min(a, (int)d[(k+9)&15]));\n"
"}\n"
"b0 = -a0;\n"
"#pragma unroll\n"
"for( k = 0; k < 16; k += 2 )\n"
"{\n"
"int b = max((int)d[(k+1)&15], (int)d[(k+2)&15]);\n"
"b = max(b, (int)d[(k+3)&15]);\n"
"b = max(b, (int)d[(k+4)&15]);\n"
"b = max(b, (int)d[(k+5)&15]);\n"
"b = max(b, (int)d[(k+6)&15]);\n"
"b = max(b, (int)d[(k+7)&15]);\n"
"b = max(b, (int)d[(k+8)&15]);\n"
"b0 = min(b0, max(b, (int)d[k]));\n"
"b0 = min(b0, max(b, (int)d[(k+9)&15]));\n"
"}\n"
"return -b0-1;\n"
"}\n"
"__kernel\n"
"void FAST_findKeypoints(\n"
"__global const uchar * _img, int step, int img_offset,\n"
"int img_rows, int img_cols,\n"
"volatile __global int* kp_loc,\n"
"int max_keypoints, int threshold )\n"
"{\n"
"int j = get_global_id(0) + 3;\n"
"int i = get_global_id(1) + 3;\n"
"if (i < img_rows - 3 && j < img_cols - 3)\n"
"{\n"
"__global const uchar* img = _img + mad24(i, step, j + img_offset);\n"
"int v = img[0], t0 = v - threshold, t1 = v + threshold;\n"
"int k, tofs, v0, v1;\n"
"int m0 = 0, m1 = 0;\n"
"#define UPDATE_MASK(idx, ofs) \\\n"
"tofs = ofs; v0 = img[tofs]; v1 = img[-tofs]; \\\n"
"m0 |= ((v0 < t0) << idx) | ((v1 < t0) << (8 + idx)); \\\n"
"m1 |= ((v0 > t1) << idx) | ((v1 > t1) << (8 + idx))\n"
"UPDATE_MASK(0, 3);\n"
"if( (m0 | m1) == 0 )\n"
"return;\n"
"UPDATE_MASK(2, -step*2+2);\n"
"UPDATE_MASK(4, -step*3);\n"
"UPDATE_MASK(6, -step*2-2);\n"
"#define EVEN_MASK (1+4+16+64)\n"
"if( ((m0 | (m0 >> 8)) & EVEN_MASK) != EVEN_MASK &&\n"
"((m1 | (m1 >> 8)) & EVEN_MASK) != EVEN_MASK )\n"
"return;\n"
"UPDATE_MASK(1, -step+3);\n"
"UPDATE_MASK(3, -step*3+1);\n"
"UPDATE_MASK(5, -step*3-1);\n"
"UPDATE_MASK(7, -step-3);\n"
"if( ((m0 | (m0 >> 8)) & 255) != 255 &&\n"
"((m1 | (m1 >> 8)) & 255) != 255 )\n"
"return;\n"
"m0 |= m0 << 16;\n"
"m1 |= m1 << 16;\n"
"#define CHECK0(i) ((m0 & (511 << i)) == (511 << i))\n"
"#define CHECK1(i) ((m1 & (511 << i)) == (511 << i))\n"
"if( CHECK0(0) + CHECK0(1) + CHECK0(2) + CHECK0(3) +\n"
"CHECK0(4) + CHECK0(5) + CHECK0(6) + CHECK0(7) +\n"
"CHECK0(8) + CHECK0(9) + CHECK0(10) + CHECK0(11) +\n"
"CHECK0(12) + CHECK0(13) + CHECK0(14) + CHECK0(15) +\n"
"CHECK1(0) + CHECK1(1) + CHECK1(2) + CHECK1(3) +\n"
"CHECK1(4) + CHECK1(5) + CHECK1(6) + CHECK1(7) +\n"
"CHECK1(8) + CHECK1(9) + CHECK1(10) + CHECK1(11) +\n"
"CHECK1(12) + CHECK1(13) + CHECK1(14) + CHECK1(15) == 0 )\n"
"return;\n"
"{\n"
"int idx = atomic_inc(kp_loc);\n"
"if( idx < max_keypoints )\n"
"{\n"
"kp_loc[1 + 2*idx] = j;\n"
"kp_loc[2 + 2*idx] = i;\n"
"}\n"
"}\n"
"}\n"
"}\n"
"__kernel\n"
"void FAST_nonmaxSupression(\n"
"__global const int* kp_in, volatile __global int* kp_out,\n"
"__global const uchar * _img, int step, int img_offset,\n"
"int rows, int cols, int counter, int max_keypoints)\n"
"{\n"
"const int idx = get_global_id(0);\n"
"if (idx < counter)\n"
"{\n"
"int x = kp_in[1 + 2*idx];\n"
"int y = kp_in[2 + 2*idx];\n"
"__global const uchar* img = _img + mad24(y, step, x + img_offset);\n"
"int s = cornerScore(img, step);\n"
"if( (x < 4 || s > cornerScore(img-1, step)) +\n"
"(y < 4 || s > cornerScore(img-step, step)) != 2 )\n"
"return;\n"
"if( (x >= cols - 4 || s > cornerScore(img+1, step)) +\n"
"(y >= rows - 4 || s > cornerScore(img+step, step)) +\n"
"(x < 4 || y < 4 || s > cornerScore(img-step-1, step)) +\n"
"(x >= cols - 4 || y < 4 || s > cornerScore(img-step+1, step)) +\n"
"(x < 4 || y >= rows - 4 || s > cornerScore(img+step-1, step)) +\n"
"(x >= cols - 4 || y >= rows - 4 || s > cornerScore(img+step+1, step)) == 6)\n"
"{\n"
"int new_idx = atomic_inc(kp_out);\n"
"if( new_idx < max_keypoints )\n"
"{\n"
"kp_out[1 + 3*new_idx] = x;\n"
"kp_out[2 + 3*new_idx] = y;\n"
"kp_out[3 + 3*new_idx] = s;\n"
"}\n"
"}\n"
"}\n"
"}\n"
, "f5e6f463f21a7ed77bd4d2c753478305", NULL};
struct cv::ocl::internal::ProgramEntry orb_oclsrc={moduleName, "orb",
"#define LAYERINFO_SIZE 1\n"
"#define LAYERINFO_OFS 0\n"
"#define KEYPOINT_SIZE 3\n"
"#define ORIENTED_KEYPOINT_SIZE 4\n"
"#define KEYPOINT_X 0\n"
"#define KEYPOINT_Y 1\n"
"#define KEYPOINT_Z 2\n"
"#define KEYPOINT_ANGLE 3\n"
"#ifdef ORB_RESPONSES\n"
"__kernel void\n"
"ORB_HarrisResponses(__global const uchar* imgbuf, int imgstep, int imgoffset0,\n"
"__global const int* layerinfo, __global const int* keypoints,\n"
"__global float* responses, int nkeypoints )\n"
"{\n"
"int idx = get_global_id(0);\n"
"if( idx < nkeypoints )\n"
"{\n"
"__global const int* kpt = keypoints + idx*KEYPOINT_SIZE;\n"
"__global const int* layer = layerinfo + kpt[KEYPOINT_Z]*LAYERINFO_SIZE;\n"
"__global const uchar* img = imgbuf + imgoffset0 + layer[LAYERINFO_OFS] +\n"
"(kpt[KEYPOINT_Y] - blockSize/2)*imgstep + (kpt[KEYPOINT_X] - blockSize/2);\n"
"int i, j;\n"
"int a = 0, b = 0, c = 0;\n"
"for( i = 0; i < blockSize; i++, img += imgstep-blockSize )\n"
"{\n"
"for( j = 0; j < blockSize; j++, img++ )\n"
"{\n"
"int Ix = (img[1] - img[-1])*2 + img[-imgstep+1] - img[-imgstep-1] + img[imgstep+1] - img[imgstep-1];\n"
"int Iy = (img[imgstep] - img[-imgstep])*2 + img[imgstep-1] - img[-imgstep-1] + img[imgstep+1] - img[-imgstep+1];\n"
"a += Ix*Ix;\n"
"b += Iy*Iy;\n"
"c += Ix*Iy;\n"
"}\n"
"}\n"
"responses[idx] = ((float)a * b - (float)c * c - HARRIS_K * (float)(a + b) * (a + b))*scale_sq_sq;\n"
"}\n"
"}\n"
"#endif\n"
"#ifdef ORB_ANGLES\n"
"#define _DBL_EPSILON 2.2204460492503131e-16f\n"
"#define atan2_p1 (0.9997878412794807f*57.29577951308232f)\n"
"#define atan2_p3 (-0.3258083974640975f*57.29577951308232f)\n"
"#define atan2_p5 (0.1555786518463281f*57.29577951308232f)\n"
"#define atan2_p7 (-0.04432655554792128f*57.29577951308232f)\n"
"inline float fastAtan2( float y, float x )\n"
"{\n"
"float ax = fabs(x), ay = fabs(y);\n"
"float a, c, c2;\n"
"if( ax >= ay )\n"
"{\n"
"c = ay/(ax + _DBL_EPSILON);\n"
"c2 = c*c;\n"
"a = (((atan2_p7*c2 + atan2_p5)*c2 + atan2_p3)*c2 + atan2_p1)*c;\n"
"}\n"
"else\n"
"{\n"
"c = ax/(ay + _DBL_EPSILON);\n"
"c2 = c*c;\n"
"a = 90.f - (((atan2_p7*c2 + atan2_p5)*c2 + atan2_p3)*c2 + atan2_p1)*c;\n"
"}\n"
"if( x < 0 )\n"
"a = 180.f - a;\n"
"if( y < 0 )\n"
"a = 360.f - a;\n"
"return a;\n"
"}\n"
"__kernel void\n"
"ORB_ICAngle(__global const uchar* imgbuf, int imgstep, int imgoffset0,\n"
"__global const int* layerinfo, __global const int* keypoints,\n"
"__global float* responses, const __global int* u_max,\n"
"int nkeypoints, int half_k )\n"
"{\n"
"int idx = get_global_id(0);\n"
"if( idx < nkeypoints )\n"
"{\n"
"__global const int* kpt = keypoints + idx*KEYPOINT_SIZE;\n"
"__global const int* layer = layerinfo + kpt[KEYPOINT_Z]*LAYERINFO_SIZE;\n"
"__global const uchar* center = imgbuf + imgoffset0 + layer[LAYERINFO_OFS] +\n"
"kpt[KEYPOINT_Y]*imgstep + kpt[KEYPOINT_X];\n"
"int u, v, m_01 = 0, m_10 = 0;\n"
"for( u = -half_k; u <= half_k; u++ )\n"
"m_10 += u * center[u];\n"
"for( v = 1; v <= half_k; v++ )\n"
"{\n"
"int v_sum = 0;\n"
"int d = u_max[v];\n"
"for( u = -d; u <= d; u++ )\n"
"{\n"
"int val_plus = center[u + v*imgstep], val_minus = center[u - v*imgstep];\n"
"v_sum += (val_plus - val_minus);\n"
"m_10 += u * (val_plus + val_minus);\n"
"}\n"
"m_01 += v * v_sum;\n"
"}\n"
"responses[idx] = fastAtan2((float)m_01, (float)m_10);\n"
"}\n"
"}\n"
"#endif\n"
"#ifdef ORB_DESCRIPTORS\n"
"__kernel void\n"
"ORB_computeDescriptor(__global const uchar* imgbuf, int imgstep, int imgoffset0,\n"
"__global const int* layerinfo, __global const int* keypoints,\n"
"__global uchar* _desc, const __global int* pattern,\n"
"int nkeypoints, int dsize )\n"
"{\n"
"int idx = get_global_id(0);\n"
"if( idx < nkeypoints )\n"
"{\n"
"int i;\n"
"__global const int* kpt = keypoints + idx*ORIENTED_KEYPOINT_SIZE;\n"
"__global const int* layer = layerinfo + kpt[KEYPOINT_Z]*LAYERINFO_SIZE;\n"
"__global const uchar* center = imgbuf + imgoffset0 + layer[LAYERINFO_OFS] +\n"
"kpt[KEYPOINT_Y]*imgstep + kpt[KEYPOINT_X];\n"
"float angle = as_float(kpt[KEYPOINT_ANGLE]);\n"
"angle *= 0.01745329251994329547f;\n"
"float cosa;\n"
"float sina = sincos(angle, &cosa);\n"
"__global uchar* desc = _desc + idx*dsize;\n"
"#define GET_VALUE(idx) \\\n"
"center[mad24(convert_int_rte(pattern[(idx)*2] * sina + pattern[(idx)*2+1] * cosa), imgstep, \\\n"
"convert_int_rte(pattern[(idx)*2] * cosa - pattern[(idx)*2+1] * sina))]\n"
"for( i = 0; i < dsize; i++ )\n"
"{\n"
"int val;\n"
"#if WTA_K == 2\n"
"int t0, t1;\n"
"t0 = GET_VALUE(0); t1 = GET_VALUE(1);\n"
"val = t0 < t1;\n"
"t0 = GET_VALUE(2); t1 = GET_VALUE(3);\n"
"val |= (t0 < t1) << 1;\n"
"t0 = GET_VALUE(4); t1 = GET_VALUE(5);\n"
"val |= (t0 < t1) << 2;\n"
"t0 = GET_VALUE(6); t1 = GET_VALUE(7);\n"
"val |= (t0 < t1) << 3;\n"
"t0 = GET_VALUE(8); t1 = GET_VALUE(9);\n"
"val |= (t0 < t1) << 4;\n"
"t0 = GET_VALUE(10); t1 = GET_VALUE(11);\n"
"val |= (t0 < t1) << 5;\n"
"t0 = GET_VALUE(12); t1 = GET_VALUE(13);\n"
"val |= (t0 < t1) << 6;\n"
"t0 = GET_VALUE(14); t1 = GET_VALUE(15);\n"
"val |= (t0 < t1) << 7;\n"
"pattern += 16*2;\n"
"#elif WTA_K == 3\n"
"int t0, t1, t2;\n"
"t0 = GET_VALUE(0); t1 = GET_VALUE(1); t2 = GET_VALUE(2);\n"
"val = t2 > t1 ? (t2 > t0 ? 2 : 0) : (t1 > t0);\n"
"t0 = GET_VALUE(3); t1 = GET_VALUE(4); t2 = GET_VALUE(5);\n"
"val |= (t2 > t1 ? (t2 > t0 ? 2 : 0) : (t1 > t0)) << 2;\n"
"t0 = GET_VALUE(6); t1 = GET_VALUE(7); t2 = GET_VALUE(8);\n"
"val |= (t2 > t1 ? (t2 > t0 ? 2 : 0) : (t1 > t0)) << 4;\n"
"t0 = GET_VALUE(9); t1 = GET_VALUE(10); t2 = GET_VALUE(11);\n"
"val |= (t2 > t1 ? (t2 > t0 ? 2 : 0) : (t1 > t0)) << 6;\n"
"pattern += 12*2;\n"
"#elif WTA_K == 4\n"
"int t0, t1, t2, t3, k;\n"
"int a, b;\n"
"t0 = GET_VALUE(0); t1 = GET_VALUE(1);\n"
"t2 = GET_VALUE(2); t3 = GET_VALUE(3);\n"
"a = 0, b = 2;\n"
"if( t1 > t0 ) t0 = t1, a = 1;\n"
"if( t3 > t2 ) t2 = t3, b = 3;\n"
"k = t0 > t2 ? a : b;\n"
"val = k;\n"
"t0 = GET_VALUE(4); t1 = GET_VALUE(5);\n"
"t2 = GET_VALUE(6); t3 = GET_VALUE(7);\n"
"a = 0, b = 2;\n"
"if( t1 > t0 ) t0 = t1, a = 1;\n"
"if( t3 > t2 ) t2 = t3, b = 3;\n"
"k = t0 > t2 ? a : b;\n"
"val |= k << 2;\n"
"t0 = GET_VALUE(8); t1 = GET_VALUE(9);\n"
"t2 = GET_VALUE(10); t3 = GET_VALUE(11);\n"
"a = 0, b = 2;\n"
"if( t1 > t0 ) t0 = t1, a = 1;\n"
"if( t3 > t2 ) t2 = t3, b = 3;\n"
"k = t0 > t2 ? a : b;\n"
"val |= k << 4;\n"
"t0 = GET_VALUE(12); t1 = GET_VALUE(13);\n"
"t2 = GET_VALUE(14); t3 = GET_VALUE(15);\n"
"a = 0, b = 2;\n"
"if( t1 > t0 ) t0 = t1, a = 1;\n"
"if( t3 > t2 ) t2 = t3, b = 3;\n"
"k = t0 > t2 ? a : b;\n"
"val |= k << 6;\n"
"pattern += 16*2;\n"
"#else\n"
"#error \"unknown/undefined WTA_K value; should be 2, 3 or 4\"\n"
"#endif\n"
"desc[i] = (uchar)val;\n"
"}\n"
"}\n"
"}\n"
"#endif\n"
, "a7c2cfaeda19907b637211b1cc91d253", NULL};

}}}
#endif
