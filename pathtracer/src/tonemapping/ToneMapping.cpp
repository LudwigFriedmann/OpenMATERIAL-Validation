//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      ToneMapping.cpp
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Tome mapping algorithms and filters

#include <iostream>

#include "ToneMapping.h"

namespace tonemapping {

    float luminance(const fvec3 inI) noexcept
    {
        fvec3 k = {0.2126f, 0.7152f, 0.0722f};
        return dot3(k, inI);
    }
}

void gammaCorrection(fvec4* img, int iw, int ih, float A, float gamma) noexcept
{
    float inI, outI, factor;
    float max = std::numeric_limits<float>::lowest();
    float min = std::numeric_limits<float>::lowest();

    std::for_each(img, img + iw*ih, [&inI, &outI, &factor, &max, &min, A, gamma](float *pixel) {
        inI = tonemapping::luminance(pixel);
        if (inI > max) max = inI;

        outI = A * std::pow(inI, gamma);
        factor = inI > fEpsilon ? outI / inI : 1.0f;
        std::transform(pixel, pixel + 3, pixel, Scalef{factor});

        outI = tonemapping::luminance(pixel);
        if (outI > min) min = outI;
    });
    
    max = std::numeric_limits<float>::lowest();
    min = std::numeric_limits<float>::max();

    std::for_each(img, img + iw*ih, [&max, &min](float *pixel) {
        min = std::min(min, *std::min_element(pixel, pixel + 3));
        max = std::max(max, *std::max_element(pixel, pixel + 3));
    });

    std::for_each(img, img + iw*ih, [](float *pixel) {
        std::transform(pixel, pixel + 3, pixel, clamp01f);
        pixel[3] = 1.0f;
    });
}