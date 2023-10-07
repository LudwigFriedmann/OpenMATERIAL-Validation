//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      RenderRay.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Base class for a rendering ray structure

#ifndef RENDERRAY_H
#define RENDERRAY_H

#include "UtilityFunctions.h"

/**
 * @brief @c RenderRay class represents a general structure of the physically-based
 * rendering ray suitable to work in the scope of @a bidirectional-pathtracing
 * workflow.
 */
class RenderRay final
{
protected:
    /// @protectedsection Intrinsic utility interface for shared data and resources.

    /// @brief Ray integer @a index.
    int m_iId;
    /// @brief Ray @a origin.
    fvec3 m_origin;
    /// @brief Ray @a direction.
    fvec3 m_direction;

public:
    /// @publicsection -Ctors and -vdtor.

    /// @brief Creates new empty instance of @c RenderRay . 
    RenderRay();
    /// @brief A -ctor, which delegates to @ref RenderRay::set with all
    /// corresponding arguments.
    RenderRay(const fvec3 o, const fvec3 r, int index);
    /// @brief Default -vdtor.
    virtual ~RenderRay() = default;

public:
    /// @publicsection Getters and setters.

    /// @brief Sets the @a index to @p id . 
    void setIndex(int id);
    /// @brief Returns ray @a index.
    int getIndex() const;

    /// @brief Sets the @a origin to @p o . 
    void setOrigin(const fvec3 o);
    /// @brief Returns the @a origin to @p o . 
    void getOrigin(fvec3 o) const;
    /// @brief Returns a pointer to @a origin. 
    float* O();

    /// @brief Sets the @a direction to @p r . 
    void setDirection(const fvec3 r);
    /// @brief Returns the @a direction to @p d . 
    void getDirection(fvec3 r) const;
    /// @brief Returns a pointer to @a direction. 
    float* R();

    /// @brief Sets the @a origin, @a direction and @a index to
    /// @p o , @p r and @p id respectively.
    void set(const fvec3 o, const fvec3 r, int id);
};

#endif // !RENDERRAY_H
