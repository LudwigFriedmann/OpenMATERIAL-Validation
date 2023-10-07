//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      ToneMapping.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Tome mapping algorithms and filters

#ifndef TONEMAPPING_H
#define TONEMAPPING_H

#include "UtilityFunctions.h"

/** 
 * @brief Implements @a gamma-correction tone mapping of the image.
 * 
 * @param img       A @a source image, where the output is written;
 * @param iw        Image @a width;
 * @param ih        Image @a height;
 * @param A         Scaling factor in the @a gamma-correction mapping;
 * @param gamma     Exponent in the @a gamma-correction mapping;
 */
void gammaCorrection(fvec4* img, int iw, int ih, float A = 1.0f, float gamma = 0.5f) noexcept;

#endif // !TONEMAPPING_H