//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      RendererParameters.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Common rendering parameters holder

#ifndef RENDERERPARAMETERS_H
#define RENDERERPARAMETERS_H

#include "UtilityFunctions.h"

/**
 * @brief @c RendererParameters is a plain aggregate of special rendering
 * parameters in the current framework of @a bidirectional-pathtracing
 * rendering.
 */
struct RendererParameters final
{
    /// @publicsection General flags.

    /// @brief Turns on @a automatic-centric in case of a single geometry object.
    bool automaticCentering = false;
    /// @brief Turns on writing @a denoised image (uses simple @a median denoiser).
    bool useDenoiser        = false;
    /// @brief Enables @a false-color-rendering mode.
    bool falseColorMode     = false;


    /// @publicsection Scene settings.

    /// @brief Scene @a rotation.
    fvec3 sceneRotationZYXdeg   = {0.0f, 0.0f, 0.0f};
    /// @brief Scene @a translation.
    fvec3 sceneTranslation      = {0.0f, 0.0f, 0.0f};


    /// @publicsection Output image settings.

    /// @brief Output image @a filename.
    const char *outputFile  = "../render_image.png";
    /// @brief Output image @a width.
    int outputWidth         = 800;
    /// @brief Output image @a height.
    int outputHeight        = 600;


    /// @publicsection Background settings.

    /// @brief @a HDR background @a filename.
    const char *hdrFile = "";
    /// @brief @a HDR background linear scaling factor.
    float hdrScaleValue = M_PIf;


    /// @publicsection Camera settings.

    /// @brief Camera lens properties @a filename.
    const char *cameraPropertiesFile    = "";
    /// @brief Number of camera bounces.
    int cameraBouncesN                  = 10;


    /// @publicsection Lighting settings.

    /// @brief Light distance attenuation exponent \f$e: ~D^{-e}\f$.
    /// If light distance is limited then D is relative value clamped between 0 and 1.
    int lightDistanceAttenuation    = 1;
    /// @brief Linear coefficient for point lights intensity.
    float lightScaleValue           = 100.0f;
    /// @brief No objects can be closer to light to prevent infinite intensity.
    float lightMinDistance          = 0.01f;
    /// @brief Number of lights bounces.
    int lightBouncesN               = 10;


    /// @publicsection Rendering options.

    /// @brief Number of threads for a @a parallel rendering.
    int numberOfCores       = 16;
    /// @brief Maximal @a connection path length.
    int maxPathLength       = 8;
    /// @brief Number of samples ( @a primary rays) used for each pixels.
    int samplesPerPixel     = 20;
    /// @brief @a Cut-off intensity to avoid accumulation of rays with very low intensity.  
    float rayCutPixValue    = 0.002f;


    /// @publicsection Tone mapping options.

    /// @brief @a Gamma-correction exponent.
    float gamma = 0.5f;
};

#endif // !RENDERERPARAMETERS_H
