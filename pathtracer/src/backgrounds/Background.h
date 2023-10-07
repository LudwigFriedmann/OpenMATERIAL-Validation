//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      Background.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-25
/// @brief     Base class for all rendering scene backgrounds

#ifndef BACKGROUND_H
#define BACKGROUND_H

#include "UtilityFunctions.h"

/**
 * @brief @c Background class is a generic structure, which represents the background of the
 * @a rendering scene in the scope of @a bidirectional-pathtracing workflow.
 *
 * @details Used for reading, storing and fetching the values of a certain pixel on the background.
 * If background is used in the rendering, hit on the background happens when the ray misses
 * all geometry in the scene. In that case, a direction of the ray is needed to get the desired
 * RGB spectrum from the background.
 */
class Background
{
protected:
    /// @protectedsection Intrinsic utility interface for shared data and resources.
    
    /// @brief Rotation of the background.
    fmat3 m_rotation;
    /// @brief Average radiance of the background.
    fvec3 m_avgRadiance;
    /// @brief Emitted radiance of the background.
    fvec3 m_emitRadiance;

public:   
    /// @publicsection -Ctors and -vdtor.

    /// @brief Creates a background with no transformations and with both its
    /// @a average and @a emitted radiance values set to 100.
    Background()
    {
        assignIdentityMatrix(m_rotation);
        std::fill_n(m_emitRadiance, 3, 100.0f);
        assignVector3(m_avgRadiance, m_emitRadiance);
    }

    /// @brief Default -vdtor.
    virtual ~Background() = default;

public: 
    /// @publicsection Getters and setters.

    /// @brief Writes the background transformation from @p R .
    void setRotation(const fmat3 R) noexcept
    {
        std::memcpy(m_rotation, R, sizeof(fmat3));
    }

public:
    /// @publicsection Virtual interface.

    /// @brief Computes the @a emitted radiance of the background along ray @p dir
    /// and writes the result to @p val .
    virtual void getRadiance(fvec3 dir, fvec3 val)
    {
        BDPT_UNUSED(dir);
        assignVector3(val, m_emitRadiance);
    }

    /// @brief Returns @a average radiance of the background to @p avg .
    virtual void getAverage(fvec3 avg)
    {
        assignVector3(avg, m_avgRadiance);
    }
};

#endif // !BACKGROUND_H
