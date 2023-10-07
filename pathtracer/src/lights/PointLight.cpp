//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      PointLight.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2022-08-03
/// @brief     Class for storing data about point light sources

#include "AssetBase.h"
#include "Node.h"
#include "PointLight.h"

PointLight::PointLight(const tinygltf::Model &gltfModel, int lightIndex)
{
    // Get to light object
    auto lightsObject = gltfModel.j.at("extensions").at("KHR_lights_punctual").at("lights");
    auto light = lightsObject.at(lightIndex);

    // Check type, it's the only required property
    if (AssetBase::hasKey(light, "type")) {
        std::string tmp = light.at("type");
        m_sLightType.swap(tmp);
    }
    else {
        std::cout << "Required property 'type' is missing for light " << lightIndex << std::endl;
        return;
    }

    // Check other properties
    if (AssetBase::hasKey(light, "color")) {
        std::copy_n(light.at("color").begin(), 3, m_color);
    }
    if (AssetBase::hasKey(light, "intensity")) {
        m_fIntensity = light.at("intensity");
    }
    if (AssetBase::hasKey(light, "range")) {
        m_fRange = light.at("range");
    }
}

void PointLight::setNode(const Node *pNode) { m_pNode = pNode; }

const Node *PointLight::getNode() const noexcept { return m_pNode; }
