//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      SurfacePoint.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Surface-ray intersection point model

#ifndef SURFACEPOINT_H
#define SURFACEPOINT_H

#include "UtilityFunctions.h"

struct RenderInstance;
struct RenderMesh;

/**
 * @brief @c SurfacePoint class represents surface-ray @a intersection point
 * and provides all @a structural and @a geometrical information at the hit point.
 */
struct SurfacePoint final
{
    /// @brief Instance of @a intersection.
    RenderInstance *m_pInst = nullptr;
    /// @brief Mesh of @a intersection.
    RenderMesh *m_pMesh = nullptr;
    /// @brief Local index of the @a hit-face.
    int m_iFaceId = -1;
    /// @brief Global index of the @a hit-face.
    int m_iGlobalFaceId = -1;

    /// @brief @a Position of the intersection point.
    fvec3 m_position = {fInfinity, fInfinity, fInfinity};
    /// @brief @a Normal in the intersection point.
    fvec3 m_normal = {0.0f};
    /// @brief @a Tangent in the intersection point (last @b signed entry 
    /// indicates the @a handedness of the basis).
    fvec4 m_tangent = {0.0f};
    /// @brief @a Bi-normal in the intersection point.
    fvec3 m_binormal = {0.0f};
    /// @brief @a Barycentric coordinates of the intersection.
    fvec3 m_bary = {0.0f};
    /// @brief Indicates, whether frame is modified by @a normal texture ( @c true ) or not.
    bool m_bIsTexNorm = false;

#ifdef FC_VALIDATION
    /// @brief Area of the @a hit-face.
    float m_fRefDensity = 0.0f;
    /// @brief Average surface gradient with respect to neighboring faces.
    float m_fAvgGradient = 0.0f;
#endif // !FC_VALIDATION

public:
    /// @publicsection Interaction interface.

    /// @brief Computes texture coordinates for channel @p chId if exist.
    /// If so, writes result to @p tc and returns @c true, otherwise returns @c false .
    bool getTexCoords(int chId, fvec2 tc) const;

    /** 
     * @brief Applies @a normal texture to the basis of current intersection point.
     *
     * @details If @a normal texture is defined, it should be passed to this function.
     * @param texNorm   Unit normal vector in @a tangent space obtained from the texture.
     */
    void applyTextureNormal(const fvec3 texNorm) noexcept;
};

#endif // !SURFACEPOINT_H
