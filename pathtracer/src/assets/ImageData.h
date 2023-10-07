//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      ImageData.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Support for glTF image reading

#ifndef IMAGEDATA_H
#define IMAGEDATA_H

#include "tiny_gltf.h"

#include "UtilityFunctions.h"

/**
 * @brief @c ImageData class is a unified wrapper of the @a glTF image of
 * different formats, which allows one to iterate over corresponding data.
 */
struct ImageData final
{
    /// @brief @a Index of the image.
    int m_iId;
    /// @brief An image represented as a set of 4-channel byte-quartet sequence.
    ubvec4* m_pImage;
    /// @brief @a Width of the underlying image.
    int m_iWidth;
    /// @brief @a Height of the underlying image.
    int m_iHeight;

public:
    /// @publicsection Default methods and basic interface (e.g. @a -ctors).

    /// @brief Creates new empty instance of @c ImageData .
    ImageData() noexcept;
    /// @brief Copy -ctor.
    ImageData(const ImageData &) noexcept;
    /// @brief Move -ctor.
    ImageData(ImageData &&) noexcept;
    /// @brief -Dtor.
    ~ImageData();

    /// @brief Loads image data from @p img and sets the index to @p id .
    void load(const tinygltf::Image &img, int imgId);
    /// @brief Frees all allocated memory.
    void free();

    /**
     * @brief Iterates over the @p data , performs an action over its
     * elements and writes the result to the current image.
     * 
     * @tparam T    An arbitrary data type;
     * @tparam Func A callable object with a single argument of type @c T and
     * return value @a convertible to @c unsigned @c char ;
     * @param data  Source data of type @p T ;
     * @param cn    Number of components for the current image;
     * @param func  Functor of type @c Func ; 
     */
    template<typename T, typename Func>
    void iterateData(const T *data, int cn, Func func)
    {
        int jd = 0;
        for (int i = 0; i < m_iHeight; ++i) {
            for (int j = 0; j < m_iWidth; ++j) {
                std::generate_n(m_pImage[i*m_iWidth + j], cn, [data, &jd, func] {
                    return func(data[jd++]);
                });
            }
        }
    }
};

#endif // !IMAGEDATA_H
