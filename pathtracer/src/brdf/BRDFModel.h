//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      BRDFModel.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Generic implementation of the bidirectional distribution function model

#ifndef BRDFMODEL_H
#define BRDFMODEL_H

#include <numeric>

#include "RandomSampler.h"
#include "UtilityFunctions.h"

#define MIN_DIELECTRICS_F0                  0.04f
#define CORRELATED_G2                       1
#define NON_SPECULAR_PART_SAMPLING_RATIO    1.0f


/// @brief Utility call to transform a vector @p cft to the basis @p basis.
inline void transformToNormalSpace(const float *basis[], const fvec3 cft, fvec3 R)
{
    // Basically: R = cft[0] * basis[0] + cft[1] * basis[1] + cft[2] * basis[2]
    std::fill_n(R, 3, 0.0f);
    std::inner_product(basis, basis + 3, cft, 0, [](int, void*) { return 0; },
        [R](const float *vBase, float k) {
            return std::transform(vBase, vBase + 3, R, R, std::bind(Func3D(linearFunction), _1, _2, k));
        }
    );
}


namespace ndf {

    /// @brief @a GGX sampling model.
    struct GGX final
    {
        /// @brief Returns the value of @a probability-distribution function
        /// evaluated for angle @p cos and squared roughness value @p alpha .
        static float pdf(float cos, float alpha) noexcept
        {
            auto cos2 = cos * cos;
            auto alpha2 = alpha * alpha;
            return alpha2 / (M_PIf * std::pow(lerp(1.0f, alpha2, cos2), 2.0f));
        }

        /// @brief Returns the value of @a lambda-function, specific for current
        /// distribution, evaluated at @p a .
        static float lambda(float a) noexcept
        {
            return 0.5f * (std::sqrt(1.0f + std::pow(a, -2.0f)) - 1.0f);
        }
    };
}


/// @brief @a Smith's G-function.
struct SmithG final
{
    template<typename NDFLambda>
    static float G1(NDFLambda &&lambda, float cos, float alpha) noexcept
    {
        auto a = cos / (alpha * std::sqrt(1.0f - cos * cos));
        return (alpha > 0.0f || cos < 0.99f) ? 1.0f / (1.0f + std::forward<NDFLambda>(lambda)(a)) : 1.0f;
    }

    static float G2(float shadow, float mask) noexcept
    {
        auto p = shadow * mask;
#if CORRELATED_G2
        return p / (shadow + mask - p);
#else
        return p;
#endif // !CORRELATED_G2
    }
};

/// @brief @c BSDFData structure stores all necessary information for @a BSDF computation.
struct BSDFData final
{
    /// @brief @a Incoming vector.
    fvec3 I;
    /// @brief @a Outcoming vector.
    fvec3 O;
    /// @brief @a Normal vector.
    fvec3 N;
    /// @brief @a Half-vector.
    fvec3 H;
    /// @brief @a Tangent.
    fvec3 T;

    /// @brief Material base color.
    fvec4 color;
    /// @brief A flag which enables transmissivity of the material.
    bool isTransmissive;

    /// @brief Material @a metallness.
    float metallness;
    /// @brief Material @a roughness.
    float roughness;
    /// @brief Material @a roughness squared.
    float alpha;

    /// @brief Material @a index-of-refraction.
    float eta;

    /// @brief Creates new empty instance of @c BSDFData .
    inline BSDFData() noexcept
    {
        std::fill_n(I, 3, 0.0f);
        std::fill_n(O, 3, 0.0f);
        std::fill_n(N, 3, 0.0f);
        std::fill_n(H, 3, 0.0f);
        std::fill_n(T, 3, 0.0f);

        std::fill_n(color, 4, 1.0f);
        isTransmissive = false;

        metallness = 0.5f;
        alpha = 0.25f;

        eta = 1.0f;
    }
};

/**
 * @brief @c BSDFModel class provides a convenient functionality to compute
 * @a BSDF factors in case of @a direct connection or @a indirect sampling of
 * the light.
 */
template<class NDFModel>
class BSDFModel final
{
    template<typename ...Args> // Generic implementation, open for extensions
    static void evaluate(
        const fvec3 vIn, const fvec3 vOut, const fvec3 norm, const fvec3 color, float metallness, float alpha,
        fvec3 brdf, Args ...args
    ) noexcept
    {
        auto extraArgs = std::make_tuple(args...);
        BDPT_UNUSED(extraArgs);

        const bool specular     = std::get<0>(extraArgs);
        const bool transmitted  = std::get<1>(extraArgs);

        float VdotN;
        float LdotN;
        float NdotH;
        float VdotH;
        float LdotH;

        {
            fvec3 H;
            if (!transmitted) {
                std::transform(vOut, vOut + 3, vIn, H, std::minus<float>{});
                normalize3(H);
            }
            else {
                std::copy_n(std::get<2>(extraArgs), 3, H);
            }

            VdotN = clamp(std::abs(dot3(vIn, norm)), 0.0001f, 1.0f);
            LdotN = clamp(std::abs(dot3(vOut, norm)), 0.0001f, 1.0f);
            NdotH = std::abs(dot3(norm, H));
            VdotH = std::abs(dot3(vIn, H));
            LdotH = std::abs(dot3(vOut, H));
        }

        fvec3 cDiff;
        std::transform(color, color + 3, cDiff, Scalef{1.0f - metallness});

        fvec3 F0;
        std::transform(color, color + 3, F0, std::bind(Func3D(lerp), MIN_DIELECTRICS_F0, _1, metallness));

        fvec3 F;
        std::transform(F0, F0 + 3, F, std::bind(Func3D(lerp), std::pow(clamp01f(1.0f - VdotH), 5.0f), 1.0f, _1));

        fvec3 fDiff;
        std::transform(F, F + 3, cDiff, fDiff, [](float Fv, float cDiffv) {
            return (1.0f - Fv) * cDiffv / M_PIf;
        });

        fvec3 fSpec;
        {
            float G = SmithG::G2(
                SmithG::G1(&NDFModel::lambda, VdotN, alpha),
                SmithG::G1(&NDFModel::lambda, LdotN, alpha)
            );

            std::transform(F, F + 3, fSpec, [VdotN, LdotN, G, transmitted](float Fv) {
                return clamp(0.25f * (transmitted ? 1.0f - Fv : Fv) * G / (VdotN * LdotN), 0.0f, std::numeric_limits<float>::max());
            });
        }

        if (specular) {
            return BDPT_UNUSED(std::transform(fSpec, fSpec + 3, brdf, Scalef{4.0f * LdotH * VdotN / NdotH / LdotN / (1.0f - std::get<3>(extraArgs))}));
        }
        else {
            return (transmitted) ?
                BDPT_UNUSED(std::transform(fSpec, fSpec + 3, brdf, Scalef{4.0f * LdotH * VdotN / NdotH / LdotN / std::get<3>(extraArgs)})) :
                BDPT_UNUSED(std::transform(fDiff, fDiff + 3, brdf, Scalef{M_PIf / LdotN / std::get<3>(extraArgs)}));
        }
    }

public:
    /// @brief Evaluates @a BRDF factors based on provided @p data and writes the result to @p brdf .
    static void evaluateDirect(const BSDFData &data, fvec3 brdf) noexcept
    {
        float VdotN = -dot3(data.I, data.N);
        float LdotN = dot3(data.O, data.N);
        float VdotH;
        float NdotH;

        fvec3 H;
        std::transform(data.O, data.O + 3, data.I, H, std::minus<float>{});
        normalize3(H);

        //VdotN = clamp(std::abs(VdotN), 0.0001f, 1.0f);
        //LdotN = clamp(std::abs(LdotN), 0.0001f, 1.0f);
        VdotH = std::abs(dot3(data.I, H));
        NdotH = std::abs(dot3(data.N, H));

        fvec3 F0;
        std::transform(data.color, data.color + 3, F0, std::bind(Func3D(lerp), MIN_DIELECTRICS_F0, _1, data.metallness));

        fvec3 F;
        std::transform(F0, F0 + 3, F, std::bind(Func3D(lerp), std::pow(clamp01f(1.0f - VdotH), 5.0f), 1.0f, _1));


        fvec3 cDiff;
        std::transform(data.color, data.color + 3, cDiff, Scalef{1.0f - data.metallness});

        fvec3 fDiff;
        if (VdotN > 0.0f && LdotN > 0.0f) {
            std::transform(F, F + 3, cDiff, fDiff, [](float Fv, float cDiffv) {
                return (1.0f - Fv) * cDiffv / M_PIf;
            });
        }
        else std::fill_n(fDiff, 3, 0.0f);

        fvec3 fSpec;
        if (VdotN > 0.0f && LdotN > 0.0f) {

            float D = NDFModel::pdf(clamp01f(NdotH), data.alpha);
            float G = SmithG::G2(
                SmithG::G1(&NDFModel::lambda, VdotN, data.alpha),
                SmithG::G1(&NDFModel::lambda, LdotN, data.alpha)
            );

            std::transform(F, F + 3, fSpec, [VdotN, LdotN, D, G](float Fv) {
                return clamp(0.25f * Fv * D * G / (VdotN * LdotN), 0.0f, std::numeric_limits<float>::max());
            });
        }
        else std::fill_n(fSpec, 3, 0.0f);

        std::transform(fDiff, fDiff + 3, fSpec, brdf, std::plus<float>{});
    }

    /// @brief Evaluates @a BSDF factors based on provided @p data with the help of the @a sampler @p rs
    /// and writes the result to @p brdf .
    static void sampleIndirect(BSDFData &data, RandomSampler *rs, fvec3 brdf) noexcept
    {
        float VdotN = -dot3(data.I, data.N);

        fvec3 binorm;
        cross3(data.N, data.T, binorm);
        const float *basis[3] = {data.T, binorm, data.N};

        if (VdotN < 0.0f) {
            if (data.isTransmissive) {
                std::swap(basis[0], basis[1]);
                std::transform(data.N, data.N + 3, data.N, std::negate<float>{});
            }
            else {
                std::fill_n(brdf, 3, 0.0f);
                return;
            }
        }

        const float NON_SPECULAR_PART_SAMPLING = 0.8f * std::abs(VdotN) + 0.1f;

        fvec3 randDirector;
        float rnd = rs->rand();
        if (rnd < NON_SPECULAR_PART_SAMPLING) {
            // non-specular

            if (rnd < NON_SPECULAR_PART_SAMPLING * (1.0f - data.color[3])) {
                // transmission

                if (VdotN < 0.0f) {
                    data.eta = 1.0f / data.eta;
                }

                rs->GGXHemisphere(randDirector, std::pow(data.alpha, 2.0f));
                transformToNormalSpace(basis, randDirector, data.H);
                normalize3(data.H);

                if (refract3(data.I, data.H, data.eta, data.O) && dot3(data.N, data.O) < 0.0f) {
                    normalize3(data.O);
                    evaluate(data.I, data.O, data.N, data.color, data.metallness, data.alpha, brdf, false, true, data.H, NON_SPECULAR_PART_SAMPLING);
                }
                else {
                    std::fill_n(brdf, 3, 0.0f);
                    return;
                }
            }
            else {
                // diffusion

                rs->cosineSqrtHemisphere(randDirector);
                transformToNormalSpace(basis, randDirector, data.O);
                normalize3(data.O);
                evaluate(data.I, data.O, data.N, data.color, data.metallness, data.alpha, brdf, false, false, data.H, NON_SPECULAR_PART_SAMPLING);
            }
        }
        else {
            // specular

            rs->GGXHemisphere(randDirector, std::pow(data.alpha, 2.0f));
            transformToNormalSpace(basis, randDirector, data.H);
            normalize3(data.H);
            reflect3(data.I, data.H, data.O);

            if (dot3(data.N, data.O) > 0.0f) {
                normalize3(data.O);
                evaluate(data.I, data.O, data.N, data.color, data.metallness, data.alpha, brdf, true, false, data.H, NON_SPECULAR_PART_SAMPLING);
            }
            else {
                std::fill_n(brdf, 3, 0.0f);
                return;
            }
        }
    }
};

#endif // !BRDFMODEL_H
