//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      DiffuseColorMaterial.cpp
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Purely diffusive material

#include <iostream>

#include "BRDFModel.h"
#include "DiffuseColorMaterial.h"
#include "RenderMesh.h"

DiffuseColorMaterial::DiffuseColorMaterial(const fvec4 c) noexcept
{
    assignVector4(m_color, c);
    m_iColorMap = -1;
    m_iColorChannel = 0;
}

void DiffuseColorMaterial::setColor(const fvec4 c) noexcept
{
    assignVector4(m_color, c);    
}

void DiffuseColorMaterial::setColorMap(int mapId, int channelId) noexcept
{
    m_iColorMap = mapId;
    m_iColorChannel = channelId;
}

void DiffuseColorMaterial::modifyFrame(SurfacePoint *sp) noexcept
{
    fvec3 tn = {0.0f, 0.0f, 1.0f};
    sp->applyTextureNormal(tn);
}

void DiffuseColorMaterial::defineNextDirection(const fvec3, const SurfacePoint *sp, fvec3 outV) noexcept
{
    fvec3 v;
    m_pSampler->uniformHemisphere(v);
    const float *basis[3] = {sp->m_tangent, sp->m_binormal, sp->m_normal};
    transformToNormalSpace(basis, v, outV);
}

void DiffuseColorMaterial::getBrdf(const fvec3 inV, const SurfacePoint* sp, const fvec3, fvec3 brdf, bool) noexcept
{
#ifdef FC_VALIDATION
        if (m_pHandler) {
            GenericFCArgument arg;
            auto subject = m_pHandler->getSubject();

            if (subject->name() == PrimitiveIDFC{}.name()) {
                arg.setAsEnumerator(sp->m_iGlobalFaceId);
            } else if (subject->name() == GeometryIDFC{}.name()) {
                arg.setAsEnumerator(sp->m_pMesh->m_iId);
            } else if (subject->name() == MaterialIDFC{}.name()) {
                arg.setAsEnumerator(-1);
            } else if (subject->name() == MetallicCftFC{}.name()) {
                assignVector4(brdf, m_color);
            } else if (subject->name() == RoughnessCftFC{}.name()) {
                assignVector4(brdf, m_color);
            } else if (subject->name() == MeshDensityFC{}.name()) {
                arg.setAsClampedFloat(sp->m_fRefDensity);
            } else if (subject->name() == SurfaceGradientFC{}.name()) {
                arg.setAsClampedFloat(sp->m_fAvgGradient);
            } else if (subject->name() == MaterialNameFC{}.name()) {
                arg.setAsByteData("mm");
            } else if (subject->name() == InvertedNormalFC{}.name()) {
                fvec3 fn, tfn;
                auto F = sp->m_pMesh->m_pF[sp->m_iFaceId];
                auto V = sp->m_pMesh->m_pfV + sp->m_pMesh->m_iVertexOffs;
                auto stride = sp->m_pMesh->m_iStride;

                const float *P[3] = {nullptr};
                std::transform(F, F + 3, P, [V, stride](unsigned int vi) { return V + stride*vi; });
                triangleNormal(P[0], P[1], P[2], fn);
                multVector3(sp->m_pInst->m_normMatrix, fn, tfn);
                normalize3(tfn);
                arg.setAsBoolean(dot3(inV, tfn) > 0.0f);
            }

            subject->paint(arg, brdf);
        }
        else {
#endif // !FC_VALIDATION
            BDPT_UNUSED(inV);

            constexpr float pdf     = 1.0f / M_PIf;
            constexpr float brdfPdf = 1.0f / (pdf * M_PIf);

            fvec2 uv;
            fvec4 albedo;
            if (0 <= m_iColorMap && m_iColorMap < m_iTexturesN && sp->getTexCoords(m_iColorChannel, uv)) {
                fvec4 tc;
                m_pTextures[m_iColorMap].texture(uv[0], uv[1], tc);
                std::transform(m_color, m_color + 4, tc, albedo, std::multiplies<float>{});
            }
            else assignVector4(albedo, m_color);

            std::transform(albedo, albedo + 3, brdf, Scalef{brdfPdf});
#ifdef FC_VALIDATION
        }
#endif // !FC_VALIDATION
}
