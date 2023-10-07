//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      RenderLightPoint.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Scene point light source description

#ifndef RENDERLIGHTPOINT_H
#define RENDERLIGHTPOINT_H

#include "RenderLight.h"

/**
 * @brief @c RenderLightPoint is a intermediate representation of @c PointLight ,
 * which impelements the interface of @c RenderLight and is suitable in a
 * scope of @a bidirectional-pathtracing workflow.
 */
class RenderLightPoint final : public RenderLight
{
    /// @brief @a Location of the light.
    fvec3 m_position;
    /// @brief @a Intensity of the light.
    fvec3 m_intensity;
    /// @brief @a Range of the light.
    float m_fRange;

public:
    /// @publicsection Default methods and basic interface (e.g. @a -ctors).

    /// @brief Creates new empty instance of @c RenderLightPoint .
    RenderLightPoint();
    /// @brief Default -dtor.
    ~RenderLightPoint() override = default;

public:
    /// @publicsection Getters and setters.

    /// @brief Sets position, intensity and range of the point light source
    /// to @p lpos , @p lintencity and @p lrange respectively.
    void set(const fvec3 lpos, const fvec3 lintencity, float lrange) noexcept;

public: 
    /// @publicsection Virtual interface.
    /// @note See the description in the base ( @c RenderLight ) class.

    void getRandomRay(fvec3 O, fvec3 R, float& pdf, fvec3 radiance) override;
    void getRadianceAlongRay(fvec3 R, float& pdf, fvec3 radiance) override;
    float getAttenuationDistance() noexcept override;
};

#endif // !RENDERLIGHTPOINT_H
