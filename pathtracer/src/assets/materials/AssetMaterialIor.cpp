//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      AssetMaterialIor.cpp
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-08-02
/// @brief     Data for index of refraction

#include "doctest.h"

#include "AssetMaterialIor.h"
#include "Exceptions.h"
#include "UtilityFunctions.h"

/// @brief Unit test for AssetMaterialIor::AssetMaterialIor 
TEST_CASE("Testing AssetMaterialIor::AssetMaterialIor")
{
    std::string filename = "../materials/data/aluminium_ior.gltf";
    #ifdef _WIN32
        std::replace(filename.begin(), filename.end(), '/', '\\');
    #endif  

    // Check if correct IOR table is loaded
    auto *pMaterialIor = new AssetMaterialIor(filename);
    std::string title = "IOR aluminium";
    CHECK(title.compare(pMaterialIor->getTitle()) == 0);
}

AssetMaterialIor::AssetMaterialIor(const nlohmann::json& j, const std::string &rcsDirectory) : AssetInfo(j,rcsDirectory)
{
    // check if asset is of type material
    if (!typeIsMaterialIor())
        throw GltfError(getUuidAsString().append(": asset is not of type materialior"));

    try
    {
        loadPropertiesFromJson(j);
    }
    catch(const nlohmann::detail::out_of_range& exception)
    {
        throw GltfError(getUuidAsString().append(": ").append(exception.what()));
    }
}

AssetMaterialIor::AssetMaterialIor(const std::string &rcsFilename)
    : AssetMaterialIor(readJsonFile(rcsFilename), utils::path::dirname(rcsFilename))
{}

AssetMaterialIor::AssetMaterialIor(const char *psFilename)
    : AssetMaterialIor(std::string(psFilename))
{}


void AssetMaterialIor::loadPropertiesFromJson(const nlohmann::json& j)
{
    using nlohmann::json;
    using std::string;

    const json& jData = j.at("extensions").at("OpenMaterial_ior_data");

    // For all temperatures
    for (const auto &it : jData.at("data"))
    {
        const float fTemperature = it.at("temperature").get<float>();

        if(fTemperature < 0)
            throw GltfError(getUuidAsString().append(": temperature must be non-negative"));

        m_iorData.emplace_back();
        IorData& data = m_iorData.back();
        data.fTemperature = fTemperature;


        // Read IOR data if present
        if(it.find("n") != it.end() && it.find("k") != it.end())
        {
            data.interpolationN.setInterpolationType(INTERPOLATION_LINEAR);
            data.interpolationK.setInterpolationType(INTERPOLATION_LINEAR);

            // Read in real part n of IOR (index of refraction)
            for(const auto &v : it.at("n"))
            {
                auto wl = static_cast<float>(v.at(0).get<double>());
                auto n  = static_cast<float>(v.at(1).get<double>());

                data.interpolationN.addPoint(wl,n);
            }

            // Read in imaginary part k of IOR (index of refraction)
            for(const auto &v : it.at("k"))
            {
                auto wl = static_cast<float>(v.at(0).get<double>());
                auto k  = static_cast<float>(v.at(1).get<double>());

                data.interpolationK.addPoint(wl,k);
            }

            // Sort the interpolation objects
            data.interpolationN.sort();
            data.interpolationK.sort();

            // Set minimum and maximum wavelength
            data.fIorMin = std::max(data.interpolationN.xMin(), data.interpolationK.xMin());
            data.fIorMax = std::min(data.interpolationN.xMax(), data.interpolationK.xMax());
        }
    }

    // Sort according to temperatures
    std::sort(m_iorData.begin(), m_iorData.end());
}

/// @brief Find index of the temperature closest to @p fTemp.
size_t AssetMaterialIor::findClosest(float fTemp) const
{
    size_t index = 0;
    float delta = std::fabs(m_iorData.at(0).fTemperature-fTemp);

    for(size_t i = 1; i < m_iorData.size(); i++)
    {
        float delta_i = std::fabs(m_iorData.at(i).fTemperature-fTemp);
        if(delta_i < delta)
        {
            index = i;
            delta = delta_i;
        }
    }

    return index;
}

float AssetMaterialIor::getIor(float fTemp, float wl, float& n, float& k) const
{
    // Find closest temperature
    size_t index = findClosest(fTemp);

    const auto& data = m_iorData[index];

    if(data.fIorMin <= wl && wl <= data.fIorMax)
    {
        // Perform linear interpolation of optical data
        n = data.interpolationN.get(wl);
        k = data.interpolationK.get(wl);
    }
    else if(data.fLOMin <= wl && wl <= data.fLOMax)
    {
        // Use lorentz oscillator model
        const float omega = 2.0f*M_PIf*physicalconstants::c/wl;
        const float omega2 = omega*omega;

        // Dielectric function
        Complex eps(1);
        Complex imag(0,1);

        for(size_t i = 0; i < data.vLoData.size()/3; i++)
        {
            const float omegap2 = data.vLoData[3*i+0];
            const float omega1  = data.vLoData[3*i+1];
            const float gamma   = data.vLoData[3*i+2];

            eps += omegap2/(omega1-imag*omega*gamma-omega2);
        }

        const float eps_real = eps.real();
        const float eps_imag = eps.imag();
        const float s = std::hypot(eps_real,eps_imag);

        // Real part of IOR
        n = std::sqrt(0.5f * (s + eps_real));
        // Imaginary part of IOR
        k = std::sqrt(0.5f * (s - eps_real));
    }
    else
        throw RuntimeError(getUuidAsString().append(": no IOR available for wavelength ").append(std::to_string(wl)).append(" and temperature ").append(std::to_string(fTemp)));

    return data.fTemperature;
}
