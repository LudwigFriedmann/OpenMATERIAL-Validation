//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      RenderSensor.cpp
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Base class for all rendering sensors

#include "RenderSensor.h"

ViewPoint::ViewPoint() noexcept
{
    std::fill_n(m_position, 3, 0.0f);
    assignIdentityMatrix(m_rotation);
}

ViewPoint::ViewPoint(const fvec3 loc, const fvec3 xdir, const fvec3 ydir, const fvec3 zdir) noexcept
{
    assignVector3(m_position, loc);
    assignVector(m_rotation[0], xdir[0], ydir[0], zdir[0]);
    assignVector(m_rotation[1], xdir[1], ydir[1], zdir[1]);
    assignVector(m_rotation[2], xdir[2], ydir[2], zdir[2]);
}


RenderSensor::RenderSensor() noexcept :
    m_iWidth(0), m_iHeight(0)
{ }

void RenderSensor::loadProperties(const char *) noexcept {}
