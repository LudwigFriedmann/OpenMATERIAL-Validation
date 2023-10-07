//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      Denoiser.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Simple median denoiser

#ifndef DENOISER_H
#define DENOISER_H

#include "UtilityFunctions.h"
#include <memory>

/**
 * @brief Implements image denoising using the @a median-filter technique.
 * @tparam HalfSize An integer which corresponds to the width of the filter;
 * @param img   A @a source image, where the output is written;
 * @param iw    Image @a width;
 * @param ih    Image @a height.
 */
template<int HalfSize>
void SimpleDenoiserMedian(fvec4* img, int iw, int ih)
{
    constexpr auto Size = 2 * HalfSize + 1;
    constexpr auto Size2 = Size * Size;

    auto new_img = new fvec4[iw*ih];

    float windowR[Size2], windowG[Size2], windowB[Size2];

    int i, j, n;
    int hsz = Size / 2;
    
    // All pixels except for rand
    for (i = hsz; i < ih - hsz; ++i) {
        for (j = hsz; j < iw - hsz; ++j) {
            n = i*iw + j; // current pixel

            int id = 0;
            for (int y = i - hsz; y <= i + hsz; ++y)
                for (int x= j - hsz; x <= j + hsz; ++x) {
                    windowR[id] = img[linearFunction(y, x, iw)][0];
                    windowG[id] = img[linearFunction(y, x, iw)][1];
                    windowB[id] = img[linearFunction(y, x, iw)][2];
                    ++id;
                }

            // Sort arrays
            std::sort(windowR, windowR + Size2);
            std::sort(windowG, windowG + Size2);
            std::sort(windowB, windowB + Size2);

            // Apply median filter
            new_img[n][0] = windowR[(Size2 - 1) / 2];
            new_img[n][1] = windowG[(Size2 - 1) / 2];
            new_img[n][2] = windowB[(Size2 - 1) / 2];
            new_img[n][3] = 1.0f;
        }
    }

    for (i = hsz; i < ih - hsz; ++i)
        std::memcpy(img + (i*iw + hsz), new_img + (i*iw + hsz), sizeof(fvec4)*(iw - 2*hsz));

    delete[] new_img;
}

#endif // !DENOISER_H
