//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      DiffuseColorMaterial.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Purely diffusive material

#ifndef DIFFUSECOLORMATERIAL_H
#define DIFFUSECOLORMATERIAL_H

#include "RenderMaterial.h"

/**
 * @brief @c DiffuseColorMaterial is a special material, which emulates the
 * properties of the missing material, impelements the interface of @c RenderMaterial
 * and therefore is suitable in a scope of @a bidirectional-pathtracing workflow.
 */
class DiffuseColorMaterial final : public RenderMaterial
{
    /// @brief Index of the @a base-color texture map.
    int m_iColorMap;
    /// @brief Index of the @a base-color texture channel.
    int m_iColorChannel;
    /// @brief Material @a base-color.
    fvec4 m_color;

public:
    /// @publicsection Default methods and basic interface (e.g. @a -ctors).

    /// @brief Creates new empty instance of @c DiffuseColorMaterial out of given base color @c c .
    explicit DiffuseColorMaterial(const fvec4 c) noexcept;
    /// @brief Default -dtor.
    ~DiffuseColorMaterial() noexcept override = default;

public:
    /// @publicsection Getters and setters.

    /// @brief Sets the @a base-color of the material.
    void setColor(const fvec4) noexcept;
    /// @brief Sets the indices of map and channel to the @a base-color texture.
    /// @param mapId        Texture map index;
    /// @param channelId    Texture channel index;
    void setColorMap(int mapId, int channelId = 0) noexcept;

public:
    /// @publicsection Virtual interface.
    /// @note See the description in the base ( @c RenderMaterial ) class.

    void modifyFrame(SurfacePoint* sp) noexcept override;
    void defineNextDirection(const fvec3 inV, const SurfacePoint *sp, fvec3 outV) noexcept override;
    void getBrdf(const fvec3 inV, const SurfacePoint *sp, const fvec3 outV, fvec3 brdf, bool impSmpScale = true) noexcept override;
};

#endif // !DIFFUSECOLORMATERIAL_H
