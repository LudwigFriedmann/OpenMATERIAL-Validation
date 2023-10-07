//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      RenderRay.cpp
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Base class for a rendering ray structure

#include "RenderRay.h"

RenderRay::RenderRay()
{
    m_iId = 0;
    std::fill_n(m_origin, 3, 0.0f);
    std::fill_n(m_direction, 3, 0.0f);
}

RenderRay::RenderRay(const fvec3 o, const fvec3 r, int id)
{
    set(o,r,id);
}

void RenderRay::setOrigin(const fvec3 o)
{
    assignVector3(m_origin, o);
}

void RenderRay::setDirection(const fvec3 r)
{
    assignVector3(m_direction,r);
    /// @b CHANGED @p r TO @p m_direction .
    normalizeIfNeeded3(m_direction);
}

void RenderRay::setIndex(int id)
{
    m_iId = id;
}

void RenderRay::set(const fvec3 o, const fvec3 r, int id)
{
    assignVector3(m_origin, o);
    assignVector3(m_direction, r);
    m_iId = id;
}

void RenderRay::getDirection(fvec3 r) const
{
    assignVector3(r, m_direction);
}

void RenderRay::getOrigin(fvec3 o) const
{
    assignVector3(o, m_origin);
}

float* RenderRay::O()
{
    return m_origin;
}

float* RenderRay::R()
{
    return m_direction;
}
int RenderRay::getIndex() const
{
    return m_iId;
}
