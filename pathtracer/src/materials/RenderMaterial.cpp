//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      RenderMaterial.cpp
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Base class for all rendering material objects

#include "RenderMaterial.h"

RenderMaterial::RenderMaterial() noexcept
{
    m_pTextures = nullptr;
    m_iTexturesN = 0;
    m_pSampler = nullptr;
#ifdef FC_VALIDATION
    m_pHandler = nullptr;
#endif // !FC_VALIDATION
}

void RenderMaterial::setRandomSampler(RandomSampler *smpl) noexcept
{
    m_pSampler = smpl;
}

void RenderMaterial::setTextures(BitmapTexture *tex, int texN) noexcept
{
    m_pTextures = tex;
    m_iTexturesN = texN;
}

#ifdef FC_VALIDATION
void RenderMaterial::setFalseColorHandler(FCHandler *hndlr) noexcept
{
    m_pHandler = hndlr;
}
#endif // !FC_VALIDATION

int RenderMaterial::getNormalTextureChannel() const noexcept
{
    return -1; // No normal channel is used
}

int RenderMaterial::getEmissivityTextureChannel() const noexcept
{
    return -1; // No emissive channel is used
}

void RenderMaterial::getRayAndBrdf(const fvec3 inV, const SurfacePoint* sp, fvec3 outV, fvec3 brdf, fvec3 emittedRadiance, bool inv) noexcept
{
    defineNextDirection(inV, sp, outV);

    if (!inv) {
        getBrdf(inV, sp, outV, brdf);
    }
    else {
        fvec3 inv_inV, inv_outV;
        std::transform(outV, outV + 3, inv_inV, std::negate<float>{});
        std::transform(inV, inV + 3, inv_outV, std::negate<float>{});
        getBrdf(inv_inV, sp, inv_outV, brdf);
    }

    int etc = getEmissivityTextureChannel();
    if (etc >= 0) {
        fvec2 euv;
        sp->getTexCoords(etc, euv);
        getEmissivity(euv, emittedRadiance);
    }
    else std::fill_n(emittedRadiance, 3, 0.0f);
}

bool RenderMaterial::getEmissivity(const fvec2, fvec3) const noexcept
{
    return false; // No emissivity is generated
}

bool RenderMaterial::isMasked(const SurfacePoint *) const noexcept
{
    return false; // No material is masked
}