//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      PhysicallyBasedMaterial.cpp
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Physically based rendering material intermediate representation

#include <cassert>
#include <cstring>

#include "BRDFModel.h"
#include "PhysicallyBasedMaterial.h"
#include "RenderMesh.h"

namespace {

    using BSDF = BSDFModel<ndf::GGX>;

    template<typename T, typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
    bool rangeCheck(T index, T begin, T end) noexcept { return begin <= index && index < end; }
} // !anonymous namespace


struct PhysicallyBasedMaterial::Helper final
{
    const PhysicallyBasedMaterial *pbm = nullptr;
    explicit Helper(const PhysicallyBasedMaterial *mat) noexcept : pbm(mat) {}

    void getTextureRGBA(const SurfacePoint *sp, fvec4 rgba) const noexcept
    {
        auto baseColorMapId = pbm->m_pBase->getTextureMapId(PbrMaterial::BaseColor);

        fvec3 uv = {0.0f, 0.0f, 1.0f};
        if (rangeCheck(baseColorMapId, 0, pbm->m_iTexturesN) && sp->getTexCoords(pbm->m_pBase->getTextureChannelId(PbrMaterial::BaseColor), uv)) {

            fmat3 TM;
            pbm->m_pBase->getTextureTransformation(PbrMaterial::BaseColor, TM);

            fvec3 uvTransformed;
            multVector3(TM, uv, uvTransformed);

            pbm->m_pTextures[baseColorMapId].texture(uvTransformed[0], uvTransformed[1], rgba);
        }
        else std::fill_n(rgba, 4, 1.0f);

        fvec4 baseColorFactors;
        pbm->m_pBase->getBaseColorFactors(baseColorFactors);
        std::transform(rgba, rgba + 4, baseColorFactors, rgba, std::multiplies<float>{});
    }

    void getTextureOccMetRoughValues(const SurfacePoint *sp, fvec3 omr) const noexcept
    {
        const auto *textures = pbm->m_pTextures;

        auto metRoughMapId = pbm->m_pBase->getTextureMapId(PbrMaterial::MetallicRoughness);
        auto occlusionMapId = pbm->m_pBase->getTextureMapId(PbrMaterial::Occlusion);

        fvec3 uv;
        fvec4 textBuf;

        assignVector(omr, 1.0f, pbm->m_pBase->getMetallicFactor(), pbm->m_pBase->getRoughnessFactor());

        assignVector(uv, 0.0f, 0.0f, 1.0f);
        if (rangeCheck(metRoughMapId, 0, pbm->m_iTexturesN) && sp->getTexCoords(pbm->m_pBase->getTextureChannelId(PbrMaterial::MetallicRoughness), uv)) {

            fmat3 TM;
            pbm->m_pBase->getTextureTransformation(PbrMaterial::MetallicRoughness, TM);
            
            fvec3 uvTransformed;
            multVector3(TM, uv, uvTransformed);
            textures[metRoughMapId].texture(uvTransformed[0], uvTransformed[1], textBuf);

            omr[0] *= textBuf[Red];
            omr[1] *= textBuf[Blue];
            omr[2] *= textBuf[Green];
        }

        assignVector(uv, 0.0f, 0.0f, 1.0f);
        if (pbm->m_pBase->isOcclusionSeparateMap() && rangeCheck(occlusionMapId, 0, pbm->m_iTexturesN) &&
            sp->getTexCoords(pbm->m_pBase->getTextureChannelId(PbrMaterial::Occlusion), uv)) {
            
            fmat3 TM;
            pbm->m_pBase->getTextureTransformation(PbrMaterial::Occlusion, TM);

            fvec3 uvTransformed;
            multVector3(TM, uv, uvTransformed);
            textures[occlusionMapId].texture(uvTransformed[0], uvTransformed[1], textBuf);
            omr[0] = textBuf[Red];
        }
    }

    void occluseColor(float occlusion, fvec3 color) noexcept
    {
        std::transform(color, color + 3, color, [occlusion, this](float c) {
            return lerp(c, occlusion * c, pbm->m_pBase->getOcclusionStrength());
        });
    }

    bool getTextureEmissivity(const fvec2 texCoord, fvec3 emissivity) const noexcept
    {
        auto emissiveMapId = pbm->m_pBase->getTextureMapId(PbrMaterial::Emissivity);

        if (rangeCheck(emissiveMapId, 0, pbm->m_iTexturesN)) {

            fmat3 TM;
            pbm->m_pBase->getTextureTransformation(PbrMaterial::Emissivity, TM);

            fvec3 emissiveFactors;
            pbm->m_pBase->getEmissiveFactors(emissiveFactors);

            fvec3 uv = {texCoord[0], texCoord[1], 1.0f}, uvTransformed;
            multVector3(TM, uv, uvTransformed);

            fvec4 textBuf;
            pbm->m_pTextures[emissiveMapId].texture(uvTransformed[0], uvTransformed[1], textBuf);
            std::transform(textBuf, textBuf + 3, emissiveFactors, emissivity, std::multiplies<float>{});
            return true;
        }
        else return false;
    }
};

void PhysicallyBasedMaterial::setMaterial(const PbrMaterial *material) noexcept
{
    m_pBase = material;
}

const PbrMaterial *PhysicallyBasedMaterial::getMaterial() const noexcept
{
    return m_pBase;
}


int PhysicallyBasedMaterial::getNormalTextureChannel() const noexcept
{
    return m_pBase->getTextureChannelId(PbrMaterial::Normal);
}

int PhysicallyBasedMaterial::getEmissivityTextureChannel() const noexcept
{
    return m_pBase->getTextureChannelId(PbrMaterial::Emissivity);
}

void PhysicallyBasedMaterial::modifyFrame(SurfacePoint *sp) noexcept
{
    auto normalMapId = m_pBase->getTextureMapId(PbrMaterial::Normal);

    fvec3 uv = {0.0f, 0.0f, 1.0f};
    fvec4 texNorm = {0.0f, 0.0f, 1.0f, 0.0f};
    if (rangeCheck(normalMapId, 0, m_iTexturesN) && sp->getTexCoords(m_pBase->getTextureChannelId(PbrMaterial::Normal), uv)) {

        fmat3 TM;
        m_pBase->getTextureTransformation(PbrMaterial::Normal, TM);

        fvec3 uvTransformed;
        multVector3(TM, uv, uvTransformed);
        m_pTextures[normalMapId].texture(uvTransformed[0], uvTransformed[1], texNorm);
        if (texNorm[2] <= 0.5f) texNorm[2] = 0.5f + fEpsilon;
        std::transform(texNorm, texNorm + 3, texNorm, std::bind(Func3D(linearFunction), 2.0f, -1.0f, _1));
        std::transform(texNorm, texNorm + 2, texNorm, Scalef{m_pBase->getNormalScale()});
        normalize3(texNorm);
    }
    sp->applyTextureNormal(texNorm);
}

void PhysicallyBasedMaterial::defineNextDirection(const fvec3, const SurfacePoint *sp, fvec3 outV) noexcept
{
    fvec3 randDirector;
    m_pSampler->uniformHemisphere(randDirector);
    const float *basis[3] = {sp->m_tangent, sp->m_binormal, sp->m_normal};
    transformToNormalSpace(basis, randDirector, outV);
    normalize3(outV);
}

void PhysicallyBasedMaterial::getBrdf(const fvec3 inV, const SurfacePoint* sp, const fvec3 outV, fvec3 brdf, bool) noexcept
{
    Helper h(this);

    BSDFData data;
    std::copy_n(inV, 3, data.I);
    std::copy_n(outV, 3, data.O);
    std::copy_n(sp->m_normal, 3, data.N);

    h.getTextureRGBA(sp, data.color);
    if (m_pBase->getAlphaMode() != PbrMaterial::Blend) data.color[Alpha] = 1.0f;
    data.isTransmissive = m_pBase->isDoubleSided() || data.color[Alpha] < 1.0f;

    if (dot3(inV, sp->m_normal) > 0.0f && data.isTransmissive) {
        std::transform(data.N, data.N + 3, data.N, std::negate<float>{});
    }

    fvec3 omr;
    h.getTextureOccMetRoughValues(sp, omr);
    h.occluseColor(omr[0], data.color);
    data.metallness = omr[1];
    data.roughness = clamp(omr[2], 0.00001f, 0.99999f);
    data.alpha = std::pow(data.roughness, 2.0f);

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
        } else if (subject->name() == MetallicCftFC{}.name()) {
            arg.setAsClampedFloat(omr[1]);
        } else if (subject->name() == RoughnessCftFC{}.name()) {
            arg.setAsClampedFloat(omr[2]);
        } else if (subject->name() == MeshDensityFC{}.name()) {
            arg.setAsClampedFloat(sp->m_fRefDensity);
        } else if (subject->name() == SurfaceGradientFC{}.name()) {
            arg.setAsClampedFloat(sp->m_fAvgGradient);
        } else if (subject->name() == MaterialNameFC{}.name()) {
            arg.setAsByteData("pbr");
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
        BSDF::evaluateDirect(data, brdf);
#ifdef FC_VALIDATION
    }
#endif // !FC_VALIDATION
}


void PhysicallyBasedMaterial::getRayAndBrdf(const fvec3 inV, const SurfacePoint *sp, fvec3 outV, fvec3 brdf, fvec3 emittedRadiance, bool) noexcept
{
    std::fill_n(emittedRadiance, 3, 0.0f);

#ifdef FC_VALIDATION
    if (m_pHandler) {
        std::fill_n(outV, 3, 0.0f);
        getBrdf(inV, sp, outV, brdf);
    }
    else {
#endif // !FC_VALIDATION
        Helper h(this);

        BSDFData data;
        std::copy_n(inV, 3, data.I);
        std::copy_n(sp->m_normal, 3, data.N);
        std::copy_n(sp->m_tangent, 3, data.T);

        h.getTextureRGBA(sp, data.color);
        if (m_pBase->getAlphaMode() != PbrMaterial::Blend) data.color[Alpha] = 1.0f;
        data.isTransmissive = m_pBase->isDoubleSided() || data.color[Alpha] < 1.0f;

        fvec3 omr;
        h.getTextureOccMetRoughValues(sp, omr);
        h.occluseColor(omr[0], data.color);
        data.metallness = omr[1];
        data.roughness = clamp(omr[2], 0.00001f, 0.99999f);
        data.alpha = std::pow(data.roughness, 2.0f);

        data.eta = 1.0f / (m_pBase->isDoubleSided() ? 1.0f : m_pBase->getIor());

        BSDF::sampleIndirect(data, m_pSampler, brdf);
        std::copy_n(data.O, 3, outV);

        auto etc = getEmissivityTextureChannel();
        if (etc >= 0) {
            fvec2 euv;
            sp->getTexCoords(etc, euv);
            getEmissivity(euv, emittedRadiance);
        }
#ifdef FC_VALIDATION
    }
#endif // !FC_VALIDATION
}

bool PhysicallyBasedMaterial::getEmissivity(const fvec2 texCoord, fvec3 emissivity) const noexcept
{
    if (m_pBase->isEmissive()) {
        return Helper(this).getTextureEmissivity(texCoord, emissivity);
    }
    else return false;
}

bool PhysicallyBasedMaterial::isMasked(const SurfacePoint* sp) const noexcept
{
    if (m_pBase->getAlphaMode() != PbrMaterial::Mask) return false;

    float alpha = m_pBase->getBaseColorChannel(Alpha);
    if (alpha < m_pBase->getAlphaCutOff()) return true;

    auto baseColorMapId = m_pBase->getTextureMapId(PbrMaterial::BaseColor);
    if (rangeCheck(baseColorMapId, 0, m_iTexturesN)) {
        fvec4 tc;
        m_pTextures[baseColorMapId].texture(sp, m_pBase->getTextureChannelId(PbrMaterial::BaseColor), tc);
        alpha *= tc[Alpha];
    }

    return alpha < m_pBase->getAlphaCutOff();
}