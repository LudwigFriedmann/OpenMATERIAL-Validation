//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      PointLight.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2022-08-03
/// @brief     Class for storing data about point light sources

#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include "tiny_gltf.h"

#include "UtilityFunctions.h"

class Node;

/// @brief Point light class.
class PointLight final
{
    /// @brief Light type ( @a point-light is the @b only supported type).
    std::string m_sLightType;
    /// @brief @c Node in which the light is defined (used for correct positioning of the light).
    const Node *m_pNode = nullptr;

    /// @brief Channel values of the light.
    fvec3 m_color = {1.0f, 1.0f, 1.0f};
    /// @brief @a Intensity of the light.
    float m_fIntensity = 1.0f;
    /// @brief @a Range of the light.
    float m_fRange = std::numeric_limits<float>::max();

public:
    /// @publicsection Basic interface.

    /// @details Creates a new @c PointLight object from the provided @a glTF model
    /// @p gltfModel and the index of the light in the @a KHR_lights_punctual extension
    /// as @p lightIndex .
    PointLight(const tinygltf::Model &gltfModel, int lightIndex);

public:
    /// @publicsection Getters and setters.

    /// @brief Sets the node that holds the light extension to @p pNode .
    void setNode(const Node *pNode);
    /// @brief Returns the node that holds the light extension.
    const Node *getNode() const noexcept;

    /// @brief Get light @a red channel value.
    float getLightRed() const noexcept { return m_color[0]; }
    /// @brief Get light @a blue channel value.
    float getLightBlue() const noexcept { return m_color[1]; }
    /// @brief Get light @a green channel value.
    float getLightGreen() const noexcept { return m_color[2]; }

    /// @brief Get light @a intensity value.
    float getLightIntensity() const noexcept { return m_fIntensity; }
    /// @brief Get light @a range value.
    float getLightRange() const noexcept { return m_fRange; }
};

#endif // !POINTLIGHT_H
