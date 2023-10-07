//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      RandomSampler.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-25
/// @brief     Random number generator

#ifndef RANDOMSAMPLER_H
#define RANDOMSAMPLER_H

#include <random>

#include "UtilityFunctions.h"

/**
 * @brief @c RandomSampler class provides wide range of utility calls,
 * suitable for work with @a random numbers and sequences.
 */
class RandomSampler final
{
    /// @brief Alias to @c std::mt19937 .
    using Generator     = std::mt19937;
    /// @brief Alias to @c std::uniform_real_distribution<float> .
    using Distribution  = std::uniform_real_distribution<float>;

    /// @brief Classic @a Mersenne @a Twister generator.
    Generator m_generator;
    /// @brief Uniform @b continuous distribution for random numbers.
    Distribution m_distribution;

public:
    /// @publicsection Basic interface (e.g. @a -ctors).

    /// @brief Constructs @c RandomSampler with random seed @p seed .
    explicit RandomSampler(unsigned int seed)
    {
        m_generator = Generator(seed);
        m_distribution = Distribution(0.0f, 1.0f);
    }

    /// @brief Sets random seed to @p seed .
    void setSeed(unsigned int seed)
    {
        m_generator.seed(seed);
        m_distribution.reset();
    }
    
public:
    /// @publicsection Getters.

    /// @brief Generates a single random @a floating-point number between 0 and 1.
    float rand() { return m_distribution(m_generator); }
    /// @brief Generates a single random @a floating-point number between @p min and @p max .
    float rand(float min, float max) { return lerp(min, max, rand()); }
    /// @brief Generates a sequence of @p N random @a floating-point numbers between 0 and 1
    /// and writes the result into @p v .
    void randN(float* v, int N) { std::generate_n(v, N, [=] { return rand(); }); }
    /// @brief Generates a sequence of @p N random @a floating-point numbers between @p min and @p max .
    /// and writes the result into @p v .
    void randN(float* v, int N, float min, float max) { std::generate_n(v, N, [=] { return rand(min, max); }); }

    /// @brief Generates a uniform random vector on the @a unit hemi-sphere.
    void uniformHemisphere(fvec3 v)
    {
        float phi = rand(0.0f, 2.0f*M_PIf);
        float cth = rand();
        float sth = std::sqrt(std::max(1.0f - cth*cth, 0.0f));
        assignVector(v, sth * std::cos(phi), sth * std::sin(phi), cth);
    }

    /// @brief Generates a uniform random vector on the @a unit sphere.
    void uniformSphere(fvec3 v)
    {
        float sl;
        do {
            randN(v, 3);
            sl = dot3(v, v);
        }
        while (sl > 1.0f || sl < fEpsilon);
        std::transform(v, v + 3, v, Scalef{1.0f / std::sqrt(sl)});
    }

    /// @brief Generates a @a GGX-based random vector on the @a unit sphere.
    void GGXHemisphere(fvec3 v, float alpha2)
    {
        float phi = rand(0.0f, 2.0f*M_PIf);
        float rnd = rand();
        float cth = std::sqrt((1.f - rnd) / (1.f - (1.0f - alpha2)*rnd));
        float sth = std::sqrt(std::max(1.0f - cth*cth, 0.0f));
        assignVector(v, sth * std::cos(phi), sth * std::sin(phi), cth);
    }

    /// @brief Generates a @a cosine-weighted random vector on the @a unit sphere.
    void cosineSqrtHemisphere(fvec3 v)
    {
        float phi = rand(0.0f, 2.0f*M_PIf);
        float rnd = rand();
        float sth = std::sqrt(rnd);
        assignVector(v, sth * std::cos(phi), sth * std::sin(phi), std::sqrt(1.0f - rnd));
    }
};

#endif // !RANDOMSAMPLER_H
