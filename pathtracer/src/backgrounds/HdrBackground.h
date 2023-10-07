//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      HdrBackground.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-17
/// @brief     HDR background representation

#ifndef BACKGROUND_HDR_H
#define BACKGROUND_HDR_H

#include <string>

#include "stb_image.h"

#include "Background.h"

/**
 * @brief @c HDRBackground class represents a model of @a HDR background in a form
 * suitable to work with in the scope of @a bidirectional-pathtracing workflow.
 */
class HDRBackground final: public Background
{
    /// @brief Pointer to @a HDR buffer.
    fvec3 *m_pImage = nullptr;
    /// @brief @a Width of the underlying image.
    int m_iWidth;
    /// @brief @a Height of the underlying image.
    int m_iHeight;
    /// @brief Scaler for outgoing radiance.
    float m_fRadianceScale;

    void computeAverage();

public:
    /// @publicsection -Ctors and -dtor.

    /// @brief Creates @c HDRBackground out of the file @p rsHdr .
    explicit HDRBackground(const char *rsHdr);
    /// @brief -Dtor.
    ~HDRBackground() override;

public:
    /// @publicsection Getters and setters.

    /// @brief Sets the overall radiance intensity factor to @p s .
    void setRadianceScale(float s) noexcept { m_fRadianceScale = s; }

public:
    /// @publicsection Virtual interface.
    /// @note See the description in the base ( @c Background ) class.

    void getRadiance(fvec3 dir, fvec3 val) override;
};

#endif // !BACKGROUND_HDR_H
