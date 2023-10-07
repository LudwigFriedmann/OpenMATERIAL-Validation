//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      SurfacePoint.cpp
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Surface-ray intersection point model

#include "SurfacePoint.h"
#include "RenderMesh.h"

bool SurfacePoint::getTexCoords(int chId, fvec2 tc) const
{
    return m_pMesh->getTexCoord(m_bary, m_iFaceId, tc, chId);
}

void SurfacePoint::applyTextureNormal(const fvec3 texNorm) noexcept
{
    if (m_bIsTexNorm) return;
    fvec3 nn = {
        texNorm[0]*m_tangent[0]+texNorm[1]*m_binormal[0]+texNorm[2]*m_normal[0],
        texNorm[0]*m_tangent[1]+texNorm[1]*m_binormal[1]+texNorm[2]*m_normal[1],
        texNorm[0]*m_tangent[2]+texNorm[1]*m_binormal[2]+texNorm[2]*m_normal[2]
    };
    
    normalizeIfNeeded3(nn, m_normal);
    cross3(m_binormal, m_normal, m_tangent);
    normalizeIfNeeded3(m_tangent);
    cross3(m_normal, m_tangent, m_binormal);
    std::transform(m_binormal, m_binormal + 3, m_binormal, Scalef{m_tangent[3]});
    m_bIsTexNorm = true;
}
