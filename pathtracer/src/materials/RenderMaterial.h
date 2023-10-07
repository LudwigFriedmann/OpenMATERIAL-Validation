//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      RenderMaterial.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Base class for all rendering material objects

#ifndef RENDERMATERIAL_H
#define RENDERMATERIAL_H

#include "BitmapTexture.h"

#ifdef FC_VALIDATION
#   include "FCHandler.h"
#endif // !FC_VALIDATION

#include "RandomSampler.h"
#include "SurfacePoint.h"

/**
 * @brief @c RenderMaterial class is used to represent the basic general material
 * interface required for rendering in the scope of @a bidirectional-pathtracing workflow.
 * Inherit from this one and implement a minimal interface. 
 */
class RenderMaterial
{
protected:
    /// @protectedsection Intrinsic utility interface for shared data and resources.

    /// @brief List of textures, avaliable in the scene.
    BitmapTexture *m_pTextures;
    /// @brief Number of textures, avaliable in the scene.
    int m_iTexturesN;
    /// @brief Class for generating random values.
    RandomSampler *m_pSampler;

#ifdef FC_VALIDATION
    /// @brief Handler for @a false-color rendering.
    FCHandler *m_pHandler;
#endif // !FC_VALIDATION

public:
    /// @publicsection Default methods and basic interface (e.g. @a -ctors).

    RenderMaterial() noexcept;
    
    BDPT_DEFINE_COPY_MODE(RenderMaterial, delete)
    BDPT_DEFINE_MOVE_MODE(RenderMaterial, default)

    virtual ~RenderMaterial() noexcept = default;

    /// @brief Sets a unique @c RandomSampler instance commonly used
    /// in the @a importance-sampling.
    void setRandomSampler(RandomSampler *) noexcept;
    /// @brief Binds the set of resource bitmap textures @p tex and their number
    /// @p texN to the current material.
    void setTextures(BitmapTexture *tex, int texN) noexcept;

#ifdef FC_VALIDATION
    /// @brief Sets a unique @c FCHandler instance used for a @a false-color
    /// rendering and validation modes.
    void setFalseColorHandler(FCHandler *) noexcept;
#endif // !FC_VALIDATION

public:
    /// @publicsection Virtual interface.

    /// @brief If material uses normal texture, this funtion should return
    /// the number of texture channel for the @a normal texture.
    virtual int getNormalTextureChannel() const noexcept;

    /// @brief If material uses emissive texture, the funtion should return
    /// the number of texture channel for the @a emissive texture.
    virtual int getEmissivityTextureChannel() const noexcept;

    /**
     * @brief Transforms the normal of the @a intersection point according to
     * the texture normal at this point.
     *
     * @details If material defines normal map, it should modify intersection frame
     * at point @p sp, using @ref SurfacePoint::applyTextureNormal function.
     */
    virtual void modifyFrame(SurfacePoint *sp) noexcept = 0;

    /**
     * @brief Computes and returns the direction of the @a reflected/sampled ray.
     * 
     * @details Material should generate unit outgoing vector @p outV using its @a BSDF.
     * Both @p inV and @p outV are set in @b global frame. Normal from @a normal map has to
     * be @b already applied to @p sp .
     */
    virtual void defineNextDirection(const fvec3 inV, const SurfacePoint *sp, fvec3 outV) noexcept = 0;

    /**
     * @brief Computes @a BRDF for @p outV based on known input vector @p inV and writes the
     * result to @p brdf .
     * 
     * @details Contact point is described in @p sp . Flag @p impSmpScaler tells whether one
     * has to divide obtained @a brdf by @a importance-sampling probability (impementation defined).
     */
    virtual void getBrdf(const fvec3 inV, const SurfacePoint *sp, const fvec3 outV, fvec3 brdf, bool impSmpScale = true) noexcept = 0;

    /**
     * @brief Computes ray @p outV and @p brdf at point @p sp based on incident ray @p inV .
     * If material is itself @a emissive, an emitted radiance has to be written to @p emittedRadiance .
     *
     * @details Flag @p inv should designate the case when the @a BRDF is being computed in the reversed
     * direction, e.g. from light source towards camera.
     */
    virtual void getRayAndBrdf(const fvec3 inV, const SurfacePoint *sp, fvec3 outV, fvec3 brdf, fvec3 emittedRadiance, bool inv = false) noexcept;

    /// @brief Returns @c false if material is not emissive, otherwise returns @c true and writes resulting
    /// emissivity (e.g. emission color * emission factor) to @p emissivity produced at the point with
    /// texture coordinates @p texCoord .
    virtual bool getEmissivity(const fvec2 texCoord, fvec3 emissivity) const noexcept;

    /// @brief If material supports @a masking, this function returns @p true in case when point @p sp
    /// shall be masked.
    virtual bool isMasked(const SurfacePoint *sp) const noexcept;
};

#endif // !RENDERMATERIAL_H
