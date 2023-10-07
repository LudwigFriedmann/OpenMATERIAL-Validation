//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      RenderLight.cpp
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Base class for all light entities on the scene

#include "RenderLight.h"

RenderLight::RenderLight() : m_fPower(0.0f), m_pSampler(nullptr), m_iIndex(0) {}

void RenderLight::setRandomSampler(RandomSampler *rs) noexcept
{
    m_pSampler = rs;
}

float RenderLight::getPower() const noexcept
{
    return m_fPower;
}

float RenderLight::getAttenuationDistance() noexcept
{
    return std::numeric_limits<float>::max();
}
