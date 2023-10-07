//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      constants.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-06-24
/// @brief     Define constants

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <limits>

#include "Defines.h"

/// @brief Mathematical constant @a pi.
#ifndef M_PIf
#   define M_PIf    3.14159265358979323846f
#endif

/// @brief @c float representation of infinity.
#ifndef fInfinity
#   define fInfinity std::numeric_limits<float>::infinity()
#endif

/// @brief @c float representation of machine epsilon.
#ifndef fEpsilon
#   define fEpsilon std::numeric_limits<float>::epsilon()
#endif


/// @brief Physical constants.
namespace physicalconstants {

    /// @brief Reduced Planck constant \f$\hbar\f$ [m² kg / s].
    static constexpr float hbar = 1.0545718e-34f;

    /// @brief Reduced Planck constant \f$\hbar\f$ [eV s/rad].
    static constexpr float hbar_eV = 6.582119514e-16f;

    /// @brief Boltzmann constant \f$k_\mathrm{B}\f$ [m² kg / ( K s² )].
    static constexpr float kB = 1.38064852e-23f;

    /// @brief Speed of light \f$c\f$ in vacuum [m/s].
    static constexpr float c = 299792458.0f;

    /// @brief Elementary charge \f$e\f$ [C].
    static constexpr float e = 1.60217662e-19f;

    /// @brief Wave lengths of 3 base channels ( @a red, @a green, @a blue ) [m].
    static constexpr float rgb_wl[3] = {6.50000004e-07f, 5.10000007e-07f, 4.40000008e-07f};
}

#define BDPT_MISSING_MATERIAL_COLOR 1000.0f, 0.0f, 1000.0f

#endif // !CONSTANTS_H