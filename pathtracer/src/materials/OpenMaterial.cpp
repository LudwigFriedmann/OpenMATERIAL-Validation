//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      OpenMaterial.cpp
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Open material intermediate representation

#include "doctest.h"

#include "OpenMaterial.h"
#include "RenderMesh.h"

#define OM_VECTOR_CORRECTION_SHIFT 1e-1f

namespace {

    void fresnelReflection(const Complex &fN, float fCosTheta, float &fTermP, float &fTermS)
    {
        auto fN2 = fN*fN;
        fCosTheta = std::abs(fCosTheta);
        const float fCos2Theta = fCosTheta*fCosTheta;
        const float fSin2Theta = 1.0f - fCos2Theta;

        // Fresnel coefficient for p-polarization
        auto fNumerator_p   = fN2*fCosTheta - std::sqrt(fN2 - fSin2Theta);
        auto fDenominator_p = fN2*fCosTheta + std::sqrt(fN2 - fSin2Theta);
        auto ratio_p = fNumerator_p/fDenominator_p;
        fTermP = std::real(ratio_p * std::conj(ratio_p));

        // Fresnel coefficient for s-polarization
        auto fNumerator_s   = fCosTheta - std::sqrt(fN2 - fSin2Theta);
        auto fDenominator_s = fCosTheta + std::sqrt(fN2 - fSin2Theta);
        auto ratio_s = fNumerator_s/fDenominator_s;
        fTermS = std::real(ratio_s * std::conj(ratio_s));
    }
} // !anonymous namespace

/// @brief Unit test for fresnelReflection
TEST_CASE("Testing fresnelReflection")
{
    float fTermP = 0, fTermS = 0;
    float n = 1.5f;
    Complex fN(n, 0.0f); // Air to glass
    
    // Check normal incidence
    fresnelReflection(fN, 0, fTermP, fTermS);
    CHECK(fTermP == fTermS);
    CHECK(std::abs(fTermP - std::pow((n - 1.0f)/(n + 1.0f), 2.0f)) < 1.0f);
}


void OpenMaterial::setMaterial(const AssetMaterial *material) noexcept
{
    m_pBase = material;
}

const AssetMaterial *OpenMaterial::getMaterial() const noexcept
{
    return m_pBase;
}


void OpenMaterial::modifyFrame(SurfacePoint *sp) noexcept
{
    const fvec3 texNorm = {0.0f, 0.0f, 1.0f};
    sp->applyTextureNormal(texNorm);
}

void OpenMaterial::defineNextDirection(const fvec3 inV, const SurfacePoint *sp, fvec3 outV) noexcept
{
    reflect3(inV, sp->m_normal, outV);

    fvec3 fn, tfn;

    auto F = sp->m_pMesh->m_pF[sp->m_iFaceId];
    auto V = sp->m_pMesh->m_pfV + sp->m_pMesh->m_iVertexOffs;
    auto stride = sp->m_pMesh->m_iStride;

    const float *P[3] = {nullptr};
    std::transform(F, F + 3, P, [V, stride](unsigned int vi) { return V + stride*vi; });
    triangleNormal(P[0], P[1], P[2], fn);
    multVector3(sp->m_pInst->m_normMatrix, fn, tfn);
    normalize3(tfn);

    float correction;
    if ((correction = dot3(outV, tfn)) < OM_VECTOR_CORRECTION_SHIFT) {
        std::transform(tfn, tfn + 3, outV, outV, std::bind(&linearFunction<float>, _1, _2, OM_VECTOR_CORRECTION_SHIFT - correction));
        normalize3(outV);
    }
}

void OpenMaterial::getBrdf(const fvec3 inV, const SurfacePoint *sp, const fvec3 outV, fvec3 brdf, bool) noexcept
{
    if (m_pBase) {
        // IoR data
        const AssetMaterialIor *pIor = m_pBase->getIorPointer();

        // Temperature of the asset
        float fTemperature = m_pBase->getTemperature();

        // Incoming ray cosine
        float fCos = -dot3(inV, sp->m_normal);
        if (std::abs(dot3(outV, sp->m_normal) - fCos) > 1e-6f) {
            std::fill_n(brdf, 3, 0.0f);
            return;
        }

#ifdef FC_VALIDATION
        if (m_pHandler) {
            GenericFCArgument arg;
            auto subject = m_pHandler->getSubject();

            if (subject->name() == PrimitiveIDFC{}.name()) {
                arg.setAsEnumerator(sp->m_iGlobalFaceId);
            } else if (subject->name() == GeometryIDFC{}.name()) {
                arg.setAsEnumerator(sp->m_pMesh->m_iId);
            } else if (subject->name() == MaterialIDFC{}.name()) {
                arg.setAsEnumerator(sp->m_pMesh->m_iMatId);
            } else if (subject->name() == MeshDensityFC{}.name()) {
                arg.setAsClampedFloat(sp->m_fRefDensity);
            } else if (subject->name() == SurfaceGradientFC{}.name()) {
                arg.setAsClampedFloat(sp->m_fAvgGradient);
            } else if (subject->name() == MaterialNameFC{}.name()) {
                arg.setAsByteData("om");
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
            else std::cerr << "False color map \"" << subject->name() << "\" is not supported by OpenMaterial." << std::endl;

            subject->paint(arg, brdf);
        }
        else {
#endif // !FC_VALIDATION
            std::transform(physicalconstants::rgb_wl, physicalconstants::rgb_wl + 3, brdf, [pIor, fTemperature, fCos](float fWavelength) {
                float n, k, fTermP, fTermS;

                pIor->getIor(fTemperature, fWavelength, n, k);
                fresnelReflection(Complex(n, k), fCos, fTermP, fTermS);
                assert(fTermP > 0.0f && fTermS > 0.0f);

                return lerp(fTermP, fTermS, 0.5f) / std::abs(fCos);
            });
#ifdef FC_VALIDATION
        }
#endif // !FC_VALIDATION
    }
    else {
        std::fill_n(brdf, 3, 0.0f);
    }
}

void OpenMaterial::getRayAndBrdf(const fvec3 inV, const SurfacePoint *sp, fvec3 outV, fvec3 brdf, fvec3 emittedRadiance, bool) noexcept
{
    defineNextDirection(inV, sp, outV);
    std::fill_n(emittedRadiance, 3, 0.0f);

    if (m_pBase) {
        // IoR data
        const AssetMaterialIor *pIor = m_pBase->getIorPointer();

        // Temperature of the asset
        float fTemperature = m_pBase->getTemperature();

        // Incoming ray cosine
        float fCos = dot3(outV, sp->m_normal);

#ifdef FC_VALIDATION
        if (m_pHandler) {
            GenericFCArgument arg;
            auto subject = m_pHandler->getSubject();

            if (subject->name() == PrimitiveIDFC{}.name()) {
                arg.setAsEnumerator(sp->m_iGlobalFaceId);
            } else if (subject->name() == GeometryIDFC{}.name()) {
                arg.setAsEnumerator(sp->m_pMesh->m_iId);
            } else if (subject->name() == MaterialIDFC{}.name()) {
                arg.setAsEnumerator(sp->m_pMesh->m_iMatId);
            } else if (subject->name() == MeshDensityFC{}.name()) {
                arg.setAsClampedFloat(sp->m_fRefDensity);
            } else if (subject->name() == SurfaceGradientFC{}.name()) {
                arg.setAsClampedFloat(sp->m_fAvgGradient);
            } else if (subject->name() == MaterialNameFC{}.name()) {
                arg.setAsByteData("om");
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
            else std::cerr << "False color map \"" << subject->name() << "\" is not supported by OpenMaterial." << std::endl;

            subject->paint(arg, brdf);
        }
        else {
#endif // !FC_VALIDATION
            std::transform(physicalconstants::rgb_wl, physicalconstants::rgb_wl + 3, brdf, [pIor, fTemperature, fCos](float fWavelength) {
                float n, k, fTermP, fTermS;

                pIor->getIor(fTemperature, fWavelength, n, k);
                fresnelReflection(Complex(n, k), fCos, fTermP, fTermS);
                assert(fTermP > 0.0f && fTermS > 0.0f);

                return lerp(fTermP, fTermS, 0.5f) / std::abs(fCos);
            });
#ifdef FC_VALIDATION
        }
#endif // !FC_VALIDATION
    }
    else {
        std::fill_n(brdf, 3, 0.0f);
    }
}