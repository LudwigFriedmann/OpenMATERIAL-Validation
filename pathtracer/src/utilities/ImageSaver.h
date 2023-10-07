//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      ImageSaver.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Utility module to save images in different formats

#ifndef IMAGESAVER_H
#define IMAGESAVER_H

#include <fstream>

#include "stb_image_write.h"

#include "Exceptions.h"
#include "UtilityFunctions.h"

/// @brief @c ImageSaver class provides a wide functionality to save images
/// in different formats.
struct ImageSaver final
{
    /// @brief Saves a @a png image @p data to the file @p filename under the given resolution
    /// @p x and @p y with @p comp number of channels.
    static void savePng(char const *filename, int x, int y, int comp, const void *data)
    {
        stbi_write_png(filename, x, y, comp, data, comp * static_cast<int>(sizeof(unsigned char)) * x);
    }

    /// @brief Saves a @a bmp image @p data to the file @p filename under the given resolution
    /// @p x and @p y with @p comp number of channels.
    static void saveBmp(char const *filename, int x, int y, int comp, const void *data)
    {
        stbi_write_bmp(filename, x, y, comp, data);
    }

    /// @brief Saves a @a tga image @p data to the file @p filename under the given resolution
    /// @p x and @p y with @p comp number of channels.
    static void saveTga(char const *filename, int x, int y, int comp, const void *data)
    {
        stbi_write_tga(filename, x, y, comp, data);
    }

    /// @brief Saves a @a jpg image @p data to the file @p filename under the given resolution
    /// @p x and @p y with @p comp number of channels. The quality of the image is given by
    /// @p quality value.
    static void saveJpg(char const *filename, int x, int y, int comp, const void *data, int quality)
    {
        stbi_write_jpg(filename, x, y, comp, data, quality);
    }

    /// @brief Saves a @a pfm image @p data to the file @p filename under the given resolution
    /// @p x and @p y .
    /// @note @b Always 3-channel image.
    static void savePfm(char const *filename, int x, int y, const float *data)
    {
        std::ofstream file;
        file.open(filename, std::ios::out | std::ios::trunc | std::ios::binary);

        if (!file.is_open())
            throw OSError(std::string("Cannot open file: ").append(filename));

        // Write PFM header
        file << "PF" << std::endl;
        file << x << " " << y << std::endl;
        file << "-1.000000" << std::endl;

        if (m_bFlip) {
            for (int i = 0; i < y; i++) {
                for (int j = 0; j < x; j++) {
                    // Rows: top-->bottom, row pixels: left-->right, color: R,G,B
                    file.write(reinterpret_cast<const char *>(data + 4*(i*x + j)), 3 * sizeof(float));
                }
            }
        }
        else {
            for (int i = 0; i < y; i++) {
                for (int j = 0; j < x; j++) {
                    // Rows: bottom-->top, row pixels: left-->right, color: R,G,B
                    file.write(reinterpret_cast<const char *>(data + 4*((y - i)*x + j)), 3 * sizeof(float));
                }
            }
        }
    }

    /// @brief Flips an image vertically when writing it, if @p flip is @c true . 
    static void flipVerticallyOnWrite(bool flip)
    {
        stbi_flip_vertically_on_write(flip);
        m_bFlip = flip;
    }

private:
    static bool m_bFlip;
};

#endif // !IMAGESAVER_H