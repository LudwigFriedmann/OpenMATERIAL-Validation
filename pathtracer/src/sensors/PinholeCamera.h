//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      PinholeCamera.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Pinhole camera implementation

#ifndef PINHOLECAMERA_H
#define PINHOLECAMERA_H

#include "RenderSensor.h"

/**
 * @brief @c PinholeCamera class represents a model of the @a pinhole camera,
 * i.e. simple camera without a lens but with a tiny aperture -- effectively
 * a light-proof box with a small hole in one side and provides an interface to
 * work in the frame of @a bidirectional-pathtracing workflow. 
 */
class PinholeCamera final : public RenderSensor
{
    /// @brief A grid represented as a set of 4-channel float-quartet sequence.
    fvec4* m_pImage;
    /// @brief Grid half physical @a width and @a height.
    float m_fHalfH, m_fHalfW;
    /// @brief Camera @a focus length.
    float m_fFocus;

public:
    /// @publicsection -Ctors and -dtor.

    /// @brief Constructs a @a pinhole camera with a given resolution @p width by @p height .
    PinholeCamera(int width, int height);
    /// @brief Constructs a @a pinhole camera with a given resolution @p width by @p height ,
    /// at the specified @a viewpoint @p vp .
    PinholeCamera(const ViewPoint &vp, int width, int height);

    BDPT_DEFINE_COPY_MODE(PinholeCamera, delete)
    BDPT_DEFINE_MOVE_MODE(PinholeCamera, default)

    ~PinholeCamera() override;

public:
    /// @publicsection Getters and setters.

    /// @brief Sets physical @p height of camera grid its width according to ratio @p aspect .
    void setRealHeight(float height, float aspect = 1.0f);
    /// @brief Sets physical sizes of the grid according to camera's @a resolution.
    void adjustToResolution() noexcept;
    /// @brief Returns a pair of physical sizes (as @a width and @a height ).
    std::pair<float, float> getRealSizes() const noexcept;
    /// @brief Adjusts @a focus length to the given field of view @p yFov (in degrees).
    void setYFoV(float yFov) noexcept;
    /// @brief Returns @a focus length. 
    float getFocus() const noexcept;

public:
    /// @publicsection Virtual interface.
    /// @note See the description in the base ( @c RenderSensor ) class.

    void init() override;
    void stop() override;
    RenderRay getRay(int x, int y, RandomSampler &sampler) const override;
    void hit(fvec3 radiance, RenderRay &inRay, RenderRay *outRay = nullptr) override;
    void getImpression(int x, int y, fvec4 v) const override;
    void loadProperties(const char *fileName) noexcept override;
};

#endif // !PINHOLECAMERA_H
