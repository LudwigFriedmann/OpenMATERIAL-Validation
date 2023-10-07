//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      RenderLightPoint.cpp
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Scene point light source description

#include "RenderLightPoint.h"

RenderLightPoint::RenderLightPoint()
{
    std::fill_n(m_position, 3, 0.0f);
    std::fill_n(m_intensity, 3, 0.0f);
    m_fRange = 0.0f;
}

void RenderLightPoint::set(const fvec3 lpos, const fvec3 lintencity, float lrange) noexcept
{
    assignVector3(m_position, lpos);
    assignVector3(m_intensity, lintencity);
    m_fRange = lrange;
    m_fPower = 0.2126f*m_intensity[0]+0.7152f*m_intensity[1]+0.0722f*m_intensity[2];
}

void RenderLightPoint::getRandomRay(fvec3 O, fvec3 R, float &pdf, fvec3 radiance)
{
    m_pSampler->uniformSphere(R);
    assignVector3(O, m_position);
    pdf = 1.0f/(4.0f*M_PIf);
    assignVector3(radiance, m_intensity);
}

void RenderLightPoint::getRadianceAlongRay(fvec3, float &pdf, fvec3 radiance)
{
    pdf = 1.0f/(4.0f*M_PIf);
    assignVector3(radiance, m_intensity);
}

float RenderLightPoint::getAttenuationDistance() noexcept
{
    return m_fRange;
}
