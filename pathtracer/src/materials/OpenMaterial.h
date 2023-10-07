//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      OpenMaterial.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Open material intermediate representation

#ifndef OPENMATERIAL_H
#define OPENMATERIAL_H

#include "RenderMaterial.h"
#include "materials/AssetMaterial.h"

/**
 * @brief @c OpenMaterial is a intermediate representation of @c AssetMaterial ,
 * which impelements the interface of @c RenderMaterial and is suitable in a
 * scope of @a bidirectional-pathtracing workflow.
 */
class OpenMaterial final : public RenderMaterial
{
    /// @brief Reference material.
    const AssetMaterial *m_pBase = nullptr;

public:
    /// @publicsection Default methods and basic interface (e.g. @a -ctors).

    /// @brief Creates new empty instance of @c OpenMaterial .
    OpenMaterial() = default;
    /// @brief Default -dtor.
    ~OpenMaterial() override = default;

public:
    /// @publicsection Getters and setters.

    /// @brief Sets the base reference material.
    void setMaterial(const AssetMaterial *) noexcept;
    /// @brief Returns the base reference material.
    const AssetMaterial *getMaterial() const noexcept;

public:
    /// @publicsection Virtual interface.
    /// @note See the description in the base ( @c RenderMaterial ) class.

    void modifyFrame(SurfacePoint *sp) noexcept override;
    void defineNextDirection(const fvec3 inV, const SurfacePoint *sp, fvec3 outV) noexcept override;
    void getBrdf(const fvec3 inV, const SurfacePoint *sp, const fvec3 outV, fvec3 brdf, bool impSmpScale = true) noexcept override;
    void getRayAndBrdf(const fvec3 inV, const SurfacePoint *sp, fvec3 outV, fvec3 brdf, fvec3 emittedRadiance, bool inv = false) noexcept override;
};

#endif // !OPENMATERIAL_H
