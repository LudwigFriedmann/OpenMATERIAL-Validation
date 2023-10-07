//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      AssetMaterialIor.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-08-02
/// @brief     Data for index of refraction

#ifndef ASSETMATERIALIOR_H
#define ASSETMATERIALIOR_H

#include "AssetInfo.h"
#include "Defines.h"
#include "Interpolation.h"

/**
 * @brief Class @p AssetMaterialIor provides a support for material IOR assets.
 * 
 * @details This class allows access to the @a index-of-refraction (IOR) assets.
 */
class AssetMaterialIor : public AssetInfo
{
    /// @brief Internal struct to save @a IOR data.
    struct IorData final
    {
        /// @brief Temperature in Kelvin.
        float fTemperature = 0;
        /// @brief Interpolation object for @b real part (i.e. @c n ) of @a IOR.
        Interpolation<float, float> interpolationN;
        /// @brief Interpolation object for @b imaginary part (i.e. @c k ) of @a IOR.
        Interpolation<float, float> interpolationK;
        /// @brief @a Smallest wavelength in meters for which data for @c n and @c k is available.
        float fIorMin = 0;
        /// @brief @a Largest wavelength in meters for which data for @c n and @c k is available.
        float fIorMax = 0;
        /// @brief Data for @a Lorentz oscillator model.
        std::vector<float> vLoData;
        /// @brief Minimum value for which @a Lorentz oscillator model is applicable.
        float fLOMin = 0;
        /// @brief Maximum value for which @a Lorentz oscillator model is applicable.
        float fLOMax = 0;

        /// @brief Utility @a operator for the sorting algorithms.
        constexpr bool operator<(const IorData &other) const noexcept { return fTemperature < other.fTemperature; }
    };

    /// @brief Vector of @a IOR data for different temperatures.
    std::vector<IorData> m_iorData;

    /// @brief Loads from @a json object @p j .
    void loadPropertiesFromJson(const nlohmann::json& j);

    /// @brief Finds index of the temperature closest to @p fTemp .
    size_t findClosest(float fTemp) const;

public:
    /// @brief Creates new @c AssetMaterialIor object from @a json file @p j , stored
    /// in the directory @p rcsDirectory .
    explicit AssetMaterialIor(const nlohmann::json &j, const std::string &rcsDirectory = "");
    /// @brief Creates new @c AssetMaterialIor object from @p rcsFilename ( @c std::string ).
    explicit AssetMaterialIor(const std::string& rcsFilename);
    /// @brief Creates new @c AssetMaterialIor object from @p psFilename (C-string).
    explicit AssetMaterialIor(const char * psFilename);

    /**
     * @brief Gets @a IOR for temperature @p fTemp and wavelength @p wl .
     * 
     * @details The data for the temperature closest to @p fTemp is used. In this data the
     * real part @p n and the imaginary part @p k of the refractive index are obtained
     * using linear interpolation of the data points.
     * 
     * @param fTemp Desired temperature;
     * @param wl    Wavelength in meters;
     * @param n     @b Real part of @a IOR;
     * @param k     @b Imaginary part of @a IOR;
     * 
     * @return Actual temperature of the data.
     */
    float getIor(float fTemp, float wl, float &n, float &k) const;
};

#endif // !ASSETMATERIALIOR_H