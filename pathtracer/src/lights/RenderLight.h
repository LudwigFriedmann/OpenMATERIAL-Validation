//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      RenderLight.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Base class for all light entities on the scene

#ifndef RENDERLIGHT_H
#define RENDERLIGHT_H

#include "RandomSampler.h"
#include "UtilityFunctions.h"

/**
 * @brief @c RenderLight class provides uniform interface for all light sources allocated
 * on the @a rendering scene in the frame of @a bidirectional-pathtracing workflow.
 * Inherit from this one and implement a minimal interface. 
 */
class RenderLight
{
protected:
    /// @protectedsection Intrinsic utility interface for shared data and resources.

    /// @brief Value that is used for light importance sampling.
    float m_fPower;
    /// @brief Class for generating random values.
    RandomSampler *m_pSampler;

public:
    /// @publicsection -Ctors and -vdtor.

    /// @brief Creates new empty instance of @c RenderLight .
    RenderLight();
    /// @brief Default -vdtor.
    virtual ~RenderLight() = default;

public:
    /// @publicsection Getters and setters.

    /// @brief Number of the light sources in the array of light sources of the scene.
    int m_iIndex;

    /// @brief Sets a unique @c RandomSampler instance commonly used
    /// in the @a importance-sampling.
    void setRandomSampler(RandomSampler *) noexcept;
    /// @brief Returns the power of the @a light source.
    float getPower() const noexcept;

public:
    /// @publicsection Virtual interface.

    /// @brief Produces the random ray emitted from point @p O in the direction @p R
    /// with an intensity @p radiance and writes probability distribution factor to @p pdf .
    virtual void getRandomRay(fvec3 O, fvec3 R, float &pdf, fvec3 radiance) = 0;

    /// @brief Computes the emitted intensity @p radiance along the ray @p R and writes
    /// corresponding probability distribution factor to @p pdf .
    virtual void getRadianceAlongRay(fvec3 R, float &pdf, fvec3 radiance) = 0;

    /// @brief Return the @a attenuation distance for the current point light source.
    virtual float getAttenuationDistance() noexcept;
};

#endif // !RENDERLIGHT_H
