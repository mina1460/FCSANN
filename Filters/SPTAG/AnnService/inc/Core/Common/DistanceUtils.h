// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#ifndef _SPTAG_COMMON_DISTANCEUTILS_H_
#define _SPTAG_COMMON_DISTANCEUTILS_H_

#include <xmmintrin.h>
#include <functional>
#include <iostream>

#include "CommonUtils.h"
#include "InstructionUtils.h"

namespace SPTAG
{
    namespace COMMON
    {
        // using DistanceCalcReturn = float(*)(const T*, const T*, DimensionType);
        template <typename T>
        using DistanceCalcReturn = float (*)(const T *pX_f, const T *pY_f, DimensionType dimension_f, SizeType filtersNum, const T *pX, const T *pY, DimensionType dimension);
        template <typename T>
        inline DistanceCalcReturn<T> DistanceCalcSelector(SPTAG::DistCalcMethod p_method);
        const static float W = 3.0;
        const static float V = 0.10;

        static float filterWeight = 1.0;

        class DistanceUtils
        {
        public:
            template <typename T>
            static float ComputeL2Distance(const T *pX, const T *pY, DimensionType length)
            {
                const T *pEnd4 = pX + ((length >> 2) << 2);
                const T *pEnd1 = pX + length;

                float diff = 0;

                while (pX < pEnd4)
                {
                    float c1 = ((float)(*pX++) - (float)(*pY++));
                    diff += c1 * c1;
                    c1 = ((float)(*pX++) - (float)(*pY++));
                    diff += c1 * c1;
                    c1 = ((float)(*pX++) - (float)(*pY++));
                    diff += c1 * c1;
                    c1 = ((float)(*pX++) - (float)(*pY++));
                    diff += c1 * c1;
                }
                while (pX < pEnd1)
                {
                    float c1 = ((float)(*pX++) - (float)(*pY++));
                    diff += c1 * c1;
                }
                return diff;
            }

            template <typename T>
            static float computeManhattanDistance(const T *pX, const T *pY, DimensionType dimension)
            {
                const T *pEnd4 = pX + ((dimension >> 2) << 2);
                const T *pEnd1 = pX + dimension;

                float diff = 0;

                while (pX < pEnd4)
                {
                    float c1 = ((float)(*pX++) - (float)(*pY++));
                    diff += abs(c1);
                    c1 = ((float)(*pX++) - (float)(*pY++));
                    diff += abs(c1);
                    c1 = ((float)(*pX++) - (float)(*pY++));
                    diff += abs(c1);
                    c1 = ((float)(*pX++) - (float)(*pY++));
                    diff += abs(c1);
                }
                while (pX < pEnd1)
                {
                    float c1 = ((float)(*pX++) - (float)(*pY++));
                    diff += abs(c1);
                }
                return diff;
            }

            template <typename T>
            static float ComputeFiltersDistance(const T *pX_filters, const T *pY_filters, DimensionType dimension_f, SizeType filtersNum, const float xy_distance)
            {

                // if there are no filters, return 0
                if (filtersNum == 0)
                    return 0.0;

                float filtersDistance = 0;
                // float bias = 10 * (W * xy_distance + 3.322);
                float bias = 4.322;

                // loop over all filters
                for (int filter_idx = 0; filter_idx < filtersNum; filter_idx++)
                {
                    float expo = computeManhattanDistance(&pX_filters[filter_idx * dimension_f], &pY_filters[filter_idx * dimension_f], dimension_f);
                    if (expo == 0) // if the manhattan distance is 0, then the filters are the same
                        continue;
                    float logg = log10(expo + 2);
                    filtersDistance += bias - (1 / logg);
                }

                return filtersDistance / filtersNum;
            }

            template <typename T>
            static float computeFusionDistance(const T *pX_f, const T *pY_f, DimensionType dimension_f, SizeType filtersNum, const T *pX, const T *pY, DimensionType dimension)
            {
                // TODO: for testing to be removed
                if (pY_f == NULL)
                {
                    printf("pY_f is NULL\n");
                    return ComputeL2Distance(pX, pY, dimension);
                }
                if (pX_f == NULL)
                {
                    printf("pX_f is NULL\n");
                    return ComputeL2Distance(pX, pY, dimension);
                }
                //////////////////////////////////////////
                bool isSize4 = (sizeof(T) == 4);
                float l2_dist;
                if (InstructionSet::AVX512())
                {
                    l2_dist = ComputeL2Distance_AVX512(pX, pY, dimension);
                }
                else if (InstructionSet::AVX2() || (isSize4 && InstructionSet::AVX()))
                {
                    l2_dist = ComputeL2Distance_AVX(pX, pY, dimension);
                }
                else if (InstructionSet::SSE2() || (isSize4 && InstructionSet::SSE()))
                {
                    l2_dist = ComputeL2Distance_SSE(pX, pY, dimension);
                }
                else
                {
                    l2_dist = ComputeL2Distance(pX, pY, dimension);
                }
                float fil_dist = ComputeFiltersDistance(pX_f, pY_f, dimension_f, filtersNum, l2_dist);

                float DW = dimension / (dimension + dimension_f * filtersNum * W * filterWeight);
                float FW = 1 - DW;
                // const float DW = 0.25;
                // const float FW = 1.0;

                return DW * l2_dist + FW * fil_dist;
            }

            static float ComputeL2Distance_SSE(const std::int8_t *pX, const std::int8_t *pY, DimensionType length);
            static float ComputeL2Distance_AVX(const std::int8_t *pX, const std::int8_t *pY, DimensionType length);
            static float ComputeL2Distance_AVX512(const std::int8_t *pX, const std::int8_t *pY, DimensionType length);

            static float ComputeL2Distance_SSE(const std::uint8_t *pX, const std::uint8_t *pY, DimensionType length);
            static float ComputeL2Distance_AVX(const std::uint8_t *pX, const std::uint8_t *pY, DimensionType length);
            static float ComputeL2Distance_AVX512(const std::uint8_t *pX, const std::uint8_t *pY, DimensionType length);

            static float ComputeL2Distance_SSE(const std::int16_t *pX, const std::int16_t *pY, DimensionType length);
            static float ComputeL2Distance_AVX(const std::int16_t *pX, const std::int16_t *pY, DimensionType length);
            static float ComputeL2Distance_AVX512(const std::int16_t *pX, const std::int16_t *pY, DimensionType length);

            static float ComputeL2Distance_SSE(const float *pX, const float *pY, DimensionType length);
            static float ComputeL2Distance_AVX(const float *pX, const float *pY, DimensionType length);
            static float ComputeL2Distance_AVX512(const float *pX, const float *pY, DimensionType length);

            template <typename T>
            static float ComputeCosineDistance(const T *pX, const T *pY, DimensionType length)
            {
                const T *pEnd4 = pX + ((length >> 2) << 2);
                const T *pEnd1 = pX + length;

                float diff = 0;

                while (pX < pEnd4)
                {
                    float c1 = ((float)(*pX++) * (float)(*pY++));
                    diff += c1;
                    c1 = ((float)(*pX++) * (float)(*pY++));
                    diff += c1;
                    c1 = ((float)(*pX++) * (float)(*pY++));
                    diff += c1;
                    c1 = ((float)(*pX++) * (float)(*pY++));
                    diff += c1;
                }
                while (pX < pEnd1)
                    diff += ((float)(*pX++) * (float)(*pY++));
                int base = Utils::GetBase<T>();
                return base * base - diff;
            }

            static float ComputeCosineDistance_SSE(const std::int8_t *pX, const std::int8_t *pY, DimensionType length);
            static float ComputeCosineDistance_AVX(const std::int8_t *pX, const std::int8_t *pY, DimensionType length);
            static float ComputeCosineDistance_AVX512(const std::int8_t *pX, const std::int8_t *pY, DimensionType length);

            static float ComputeCosineDistance_SSE(const std::uint8_t *pX, const std::uint8_t *pY, DimensionType length);
            static float ComputeCosineDistance_AVX(const std::uint8_t *pX, const std::uint8_t *pY, DimensionType length);
            static float ComputeCosineDistance_AVX512(const std::uint8_t *pX, const std::uint8_t *pY, DimensionType length);

            static float ComputeCosineDistance_SSE(const std::int16_t *pX, const std::int16_t *pY, DimensionType length);
            static float ComputeCosineDistance_AVX(const std::int16_t *pX, const std::int16_t *pY, DimensionType length);
            static float ComputeCosineDistance_AVX512(const std::int16_t *pX, const std::int16_t *pY, DimensionType length);

            static float ComputeCosineDistance_SSE(const float *pX, const float *pY, DimensionType length);
            static float ComputeCosineDistance_AVX(const float *pX, const float *pY, DimensionType length);
            static float ComputeCosineDistance_AVX512(const float *pX, const float *pY, DimensionType length);

            template <typename T>
            static inline float ComputeDistance(const T *pX_f, const T *pY_f, DimensionType dimension_f, SizeType filtersNum, const T *pX, const T *pY, DimensionType dimension, SPTAG::DistCalcMethod distCalcMethod)
            {
                auto func = DistanceCalcSelector<T>(distCalcMethod);
                return func(pX_f, pY_f, dimension_f, filtersNum, pX, pY, dimension);
            }

            static inline float ConvertCosineSimilarityToDistance(float cs)
            {
                // Cosine similarity is in [-1, 1], the higher the value, the closer are the two vectors.
                // However, the tree is built and searched based on "distance" between two vectors, that's >=0. The smaller the value, the closer are the two vectors.
                // So we do a linear conversion from a cosine similarity to a distance value.
                return 1 - cs; //[1, 3]
            }

            static inline float ConvertDistanceBackToCosineSimilarity(float d)
            {
                return 1 - d;
            }
        };

        template <typename T>
        inline DistanceCalcReturn<T> DistanceCalcSelector(SPTAG::DistCalcMethod p_method)
        {
            // bool isSize4 = (sizeof(T) == 4);
            // switch (p_method)
            // {
            // case SPTAG::DistCalcMethod::InnerProduct:
            // case SPTAG::DistCalcMethod::Cosine:
            //     if (InstructionSet::AVX512())
            //     {
            //         return &(DistanceUtils::ComputeCosineDistance_AVX512);
            //     }
            //     else if (InstructionSet::AVX2() || (isSize4 && InstructionSet::AVX()))
            //     {
            //         return &(DistanceUtils::ComputeCosineDistance_AVX);
            //     }
            //     else if (InstructionSet::SSE2() || (isSize4 && InstructionSet::SSE()))
            //     {
            //         return &(DistanceUtils::ComputeCosineDistance_SSE);
            //     }
            //     else {
            //         return &(DistanceUtils::ComputeCosineDistance);
            //     }

            // case SPTAG::DistCalcMethod::L2:
            //     if (InstructionSet::AVX512())
            //     {
            //         return &(DistanceUtils::ComputeL2Distance_AVX512);
            //     }
            //     else if (InstructionSet::AVX2() || (isSize4 && InstructionSet::AVX()))
            //     {
            //         return &(DistanceUtils::ComputeL2Distance_AVX);
            //     }
            //     else if (InstructionSet::SSE2() || (isSize4 && InstructionSet::SSE()))
            //     {
            //         return &(DistanceUtils::ComputeL2Distance_SSE);
            //     }
            //     else {
            //         return &(DistanceUtils::computeFusionDistance);
            //     }

            // default:
            //     break;
            // }
            return &(DistanceUtils::computeFusionDistance);
            return nullptr;
        }
    }
}

#endif // _SPTAG_COMMON_DISTANCEUTILS_H_
