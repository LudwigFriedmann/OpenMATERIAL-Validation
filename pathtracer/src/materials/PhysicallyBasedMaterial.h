//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      PhysicallyBasedMaterial.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Physically based rendering material intermediate representation

#ifndef PHYSICALLYBASEDMATERIAL_H
#define PHYSICALLYBASEDMATERIAL_H

#include "materials/PbrMaterial.h"
#include "RenderMaterial.h"

/**
 * @brief @c PhysicallyBasedMaterial is a intermediate representation of
 * @c PbrMaterial , which impelements the interface of @c RenderMaterial
 * and is suitable in a scope of @a bidirectional-pathtracing workflow.
 */
class PhysicallyBasedMaterial final : public RenderMaterial
{
    struct Helper;

    /// @brief Reference material.
    const PbrMaterial *m_pBase = nullptr;

public:
    /// @publicsection Default methods and basic interface (e.g. @a -ctors).

    /// @brief Creates new empty instance of @c PhysicallyBasedMaterial .
    PhysicallyBasedMaterial() = default;
    /// @brief Default -dtor.
    ~PhysicallyBasedMaterial() override = default;

public:
    /// @publicsection Getters and setters.

    /// @brief Sets the base reference material.
    void setMaterial(const PbrMaterial *) noexcept;
    /// @brief Returns the base reference material.
    const PbrMaterial *getMaterial() const noexcept;

public:
    /// @publicsection Virtual interface.
    /// @note See the description in the base ( @c RenderMaterial ) class.

    int getNormalTextureChannel() const noexcept override;
    int getEmissivityTextureChannel() const noexcept override;
    void modifyFrame(SurfacePoint *sp) noexcept override;
    void defineNextDirection(const fvec3 inV, const SurfacePoint *sp, fvec3 outV) noexcept override;
    void getBrdf(const fvec3 inV, const SurfacePoint *sp, const fvec3 outV, fvec3 brdf, bool impSmpScale = true) noexcept override;
    void getRayAndBrdf(const fvec3 inV, const SurfacePoint *sp, fvec3 outV, fvec3 brdf, fvec3 emittedRadiance, bool inv = false) noexcept override;
    bool getEmissivity(const fvec2 texCoord, fvec3 emissivity) const noexcept override;
    bool isMasked(const SurfacePoint *) const noexcept override;
};

#endif // !PHYSICALLYBASEDMATERIAL_H
