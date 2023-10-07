//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      BitmapTexture.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Structural representation of the texture bitmap

#ifndef BITMAPTEXTURE_H
#define BITMAPTEXTURE_H

#include "SurfacePoint.h"

/**
 * @brief @c BitmapTexture class is a convenient wrapper for a texture @a bitmap
 * image, which provides basic functionality to work with it.
 */
struct BitmapTexture
{
protected:
    /// @protectedsection Intrinsic utility interface for shared data and resources.

    /// @brief An image represented as a set of 4-channel byte-quartet sequence.
    ubvec4* m_pImage;
    /// @brief @a Width of the underlying image.
    int m_iWidth;
    /// @brief @a Height of the underlying image.
    int m_iHeight;
    
    /// @brief Returns indices of texture pixels @p id and their weights @p w for
    /// @a bilinear interpolation at point with coordinates ( @p u , @p v ).
    void getPixCoords(float u, float v, ivec4 id, fvec4 w) const;

public:
    /// @publicsection -Ctors and -vdtor.

    /// @brief Creates new empty instance of @c BitmapTexture .
    BitmapTexture();
    /// @brief A -ctor, which delegates to @ref BitmapTexture::set with all
    /// corresponding arguments.
    BitmapTexture(int w, int h, ubvec4* img, bool copy = true);

    BDPT_DEFINE_COPY_MODE(BitmapTexture, delete)
    BDPT_DEFINE_MOVE_MODE(BitmapTexture, default)

    virtual ~BitmapTexture();

public:
    /// @publicsection Getters and setters.

    /// @brief Acquise source image @p img with a resolution @p w x @p h to the
    /// underlying storage or creates its copy, if @p copy is set to @c true .
    void set(int w, int h, ubvec4* img, bool copy = true);

    /// @brief Returns @a width of the the underlying image.
    inline int getWidth()  const { return m_iWidth; }
    /// @brief Returns @a height of the the underlying image.
    inline int getHeight() const { return m_iHeight; }

public:
    /**
     * @brief Function fetches texture color @p tex at point @p x , @p y @b without interpolation.
     */
    void texelFetch(int x, int y, ubvec4 tex) const;
    /**
     * @brief Same as @ref BitmapTexture::texelFetch(int, int, ubvec4) const, but
     * for floating-point representation of the color @p tex .
     * @overload texelFetch(int, int, ubvec4) const
     */
    void texelFetch(int x, int y, fvec4 tex) const;

    /**
     * @brief Extracts the color from the texture @a bitmap according to
     * the given texture coordinates @p u and @p v and writes the result
     * to @p tex .
     */
    void texture(float u, float v, ubvec4 tex) const;
    /**
     * @brief Same as @ref BitmapTexture::texture(float, float, ubvec4) const, but
     * for floating-point representation of the color @p tex .
     * @overload texture(float, float, ubvec4) const
     */
    void texture(float u, float v, fvec4 tex) const;
    /**
     * @brief Extracts the color from the texture @a bitmap according to
     * the texture coordinates obtained from intersection point @p sp and
     * channel id @p chId and writes the result to @p tex .
     * @overload texture(float, float, fvec4) const
     */
    void texture(const SurfacePoint *sp, int chId, ubvec4 tex) const;
    /**
     * @brief Same as @ref BitmapTexture::texture(const SurfacePoint *, int, ubvec4) const, but
     * for floating-point representation of the color @p tex .
     * @overload texture(const SurfacePoint *, int, ubvec4) const
     */
    void texture(const SurfacePoint *sp, int chId, fvec4 tex) const;
};

#endif // !BITMAPTEXTURE_H
