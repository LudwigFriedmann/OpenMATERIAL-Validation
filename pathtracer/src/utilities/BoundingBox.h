//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      BoundingBox.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-06-30
/// @brief     Geometry bounding box

#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include <numeric>

#include "UtilityFunctions.h"

/**
 * @brief @c BoundingBox class is a utility structure, which represents the
 * rectangular bounding of any geometrical object on the scene.
 * 
 * @tparam T    Any arithmetical type.
 */
template <typename T>
struct BoundingBox final
{
    static_assert(
        std::is_signed<T>::value && std::is_arithmetic<T>::value,
        "Only signed arithmetic types are allowed for bounding box representation"
    );

    /// @brief Lowest corner of bounding box.
    T m_P0[3];
    /// @brief Highest corner of bounding box.
    T m_P1[3];

public:
    /// @publicsection Default methods and basic interface (e.g. @a -ctors).

    BoundingBox() noexcept { reset(); }

    /// @brief Resets bounding box to a default ( @b invalid ) state.
    void reset() noexcept
    {
        std::fill_n(m_P0, 3, std::numeric_limits<T>::max());
        std::fill_n(m_P1, 3, std::numeric_limits<T>::lowest());
    }

    /// @brief Checks whether bounding box is in the valid state.
    bool isValid() const noexcept
    {
        return std::equal(m_P0, m_P0 + 3, m_P1, std::less_equal<T>{});
    }

public:
    /// @publicsection Main interface.

    /**
     * @brief Adds a single point @p X to the current bounding box.
     * 
     * @details Entries of @p X are compared with current limits and the marginal
     * values are chosen, such that the bounding box always "grows".
     */
    void add(const T *X)
    {
        std::transform(X, X + 3, m_P0, m_P0, &min<T>);
        std::transform(X, X + 3, m_P1, m_P1, &max<T>);
    }
    /**
     * @brief Same as @ref BoundingBox<T>::add(const T *), but for a triplet of
     * arithmetic value @p X , @p Y and @p Z .
     */
    void add(T X, T Y, T Z) noexcept
    {
        const T P[3] = {X, Y, Z};
        add(P);
    }
    /**
     * @brief Unlike to other 2 overloaded functions adds another entire
     * bounding box @p BB .
     */
    void add(const BoundingBox &BB) noexcept
    {
        std::transform(BB.m_P0, BB.m_P0 + 3, m_P0, m_P0, &min<T>);
        std::transform(BB.m_P1, BB.m_P1 + 3, m_P1, m_P1, &max<T>);
    }

    /// @brief Computes and returns a new bounding box, which represents the
    /// intersection of @p BB with the current one.
    BoundingBox intersection(const BoundingBox &BB) const noexcept
    {
        BoundingBox I;
        std::transform(m_P0, m_P0 + 3, BB.m_P0, I.m_P0, &max<T>);
        std::transform(m_P1, m_P1 + 3, BB.m_P1, I.m_P1, &min<T>);
        return I;
    }

    /// @brief Transforms current bounding box with rotation matrix @p Rot and
    /// translation vector @p Shift .
    void transform(const T Shift[], const T Rot[][3])
    {
        auto RotShift = [Shift, Rot](const T *I, T *O) {
            multVector3(Rot, I, O);
            std::transform(O, O + 3, Shift, O, std::plus<T>{});
        };

        T P[8][3];
        getCorners(P);
        reset();

        T O[3];
        std::for_each(P, P + 8, [RotShift, &O, this](const T *I) {
            RotShift(I, O);
            add(O);
        });
    }

public:
    /// @publicsection Getters.

    /// @brief Returns the volume of current bounding box, if it is @a valid, otherwise returns 0.
    T getVolume() const noexcept
    {
        return isValid() ? std::inner_product(m_P1, m_P1 + 3, m_P0, static_cast<T>(1), std::multiplies<T>{}, std::minus<T>{}) : static_cast<T>(0);
    }

    /// @brief Returns the surface area of current bounding box, if it is @a valid, otherwise returns 0.
    float getSurfaceArea() const noexcept
    {
        if (!isValid()) return static_cast<T>(0);
        T W = m_P1[0] - m_P0[0];
        T L = m_P1[1] - m_P0[1];
        T H = m_P1[2] - m_P0[2];
        return static_cast<T>(2.0)*(W*L + W*H + L*H);
    }

    /// @brief Returns the longes side of the bounding box.
    float getLongestSide() const noexcept
    {
        return isValid() ? std::inner_product(m_P1, m_P1 + 3, m_P0, static_cast<T>(0), &max<T>, std::minus<T>{}) : static_cast<T>(0);
    }

    /// @brief Writes a geometrical center of the bounding box to @p C . 
    void getCenter(T *C) const noexcept
    {
        std::transform(m_P0, m_P0 + 3, m_P1, C, std::bind(Func3D(lerp), _1, _2, static_cast<T>(0.5)));
    }

    /// @brief Writes all @a 8 corners of the bounding box to a matrix @p P .
    void getCorners(T P[][3]) const noexcept
    {
        assignVector(P[0], m_P0[0], m_P0[1], m_P0[2]);
        assignVector(P[1], m_P0[0], m_P0[1], m_P1[2]);
        assignVector(P[2], m_P0[0], m_P1[1], m_P0[2]);
        assignVector(P[3], m_P0[0], m_P1[1], m_P1[2]);
        assignVector(P[4], m_P1[0], m_P0[1], m_P0[2]);
        assignVector(P[5], m_P1[0], m_P0[1], m_P1[2]);
        assignVector(P[6], m_P1[0], m_P1[1], m_P0[2]);
        assignVector(P[7], m_P1[0], m_P1[1], m_P1[2]);
    }
};


#endif // !BOUNDINGBOX_H
