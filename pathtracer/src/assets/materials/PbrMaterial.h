//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      PbrMaterial.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2022-04-22
/// @brief     Material properties and methods to load from glTF model

#ifndef PBRMATERIAL_H
#define PBRMATERIAL_H

#include "AssetGeometry.h"
#include "Material.h"

/**
 * @brief @c PbrMaterial class provides support for @a Physically-Based @a Rendering materials.
 * The class is a data container and allows reading material parameters from the @a glTF extension
 * @a pbrMetallicRoughness, and the access to the parameters.
 */
class PbrMaterial final : public Material
{
    struct TextureData
    {
        int m_iMapId, m_iChannelId;
        fmat3 m_transformation;

        void reset() noexcept
        {
            m_iMapId = -1; m_iChannelId = 0;
            assignIdentityMatrix(m_transformation);
        }
    };

public:
    /// @publicsection Special @a enums.

    /// @brief Transparency mode for @a alpha-blending.
    enum AlphaMode : unsigned char
    {
        Blend,
        Mask,
        Opaque
    };

    /// @brief Texture properties suppored by a @a glTF material.
    enum Texture : size_t
    {
        BaseColor,
        MetallicRoughness,
        Normal,
        Emissivity,
        Occlusion,

        TexCount
    };

public:
    /// @publicsection Basic interface (e.g. -ctors).

    /// @brief Creates new empty instance of @c PbrMaterial .
    PbrMaterial();
    /// @brief Creates new empty instance of @c PbrMaterial out of @a glTF-material @c gltfModel
    /// with corresponding material index @c materialIndex .
    PbrMaterial(const tinygltf::Model &gltfModel, int materialIndex);
    /// @brief Default -dtor.
    ~PbrMaterial() override = default;

private:
    /// @publicsection Helping interface.

    void reset() noexcept;
    void loadPropertiesFromJson(const tinygltf::Model &gltfModel, int materialIndex);
    void print(std::ostream& os) const override;

public:
    /// @publicsection Getters.

    /// @brief Returns human-readable @a name of material.
    std::string getMaterialName() const;

    /// @brief Checks whether material is @a double-sided. 
    bool isDoubleSided() const noexcept;

    /// @brief Returns material @a alpha-mode.  
    AlphaMode getAlphaMode() const noexcept;
    /// @brief Returns material @a alpha-cut-off. 
    float getAlphaCutOff() const noexcept;

    /// @brief Returns an index of the map of the corresponding texture in @c Texture ,
    /// or -1, if it does not exist.
    int getTextureMapId(Texture) const noexcept;
    /// @brief Returns an index of the channel of the corresponding texture in @c Texture ,
    /// or -1, if it does not exist.
    int getTextureChannelId(Texture) const noexcept;
    /// @brief Returns the number of used textures. 
    size_t getTexturesUsed() const noexcept;

    /// @brief Returns transformation for the given texture in @c Texture .
    void getTextureTransformation(Texture, fmat3 &) const noexcept;

    /// @brief Returns material @a base-color factors in one array.
    void getBaseColorFactors(fvec4) const noexcept;
    /// @brief Returns material @a base-color factor for the specified channel in
    /// @c ColorChannel . 
    float getBaseColorChannel(ColorChannel) const noexcept;

    /// @brief Returns material @a emissivity factors in one array.
    void getEmissiveFactors(fvec3) const noexcept;
    /// @brief Returns material @a emissivity factor for the specified channel in
    /// @c ColorChannel .
    float getEmissiveFactor(ColorChannel) noexcept;
    /// @brief Checks whether material is @a emissive. 
    bool isEmissive() const noexcept;

    /// @brief Returns material @a metalness and @a roughness factors in one array. 
    void getMetallicRoughnessFactors(fvec2) const noexcept;
    /// @brief Returns material @a metalness factor. 
    float getMetallicFactor() const noexcept;
    /// @brief Returns material @a roughness factor. 
    float getRoughnessFactor() const noexcept;

    /// @brief Returns material @a texture-normal scale. 
    float getNormalScale() const noexcept;
    /// @brief Returns material @a occlusion-strength factor. 
    float getOcclusionStrength() const noexcept;

    /// @brief Returns material @a index-of-refraction. 
    float getIor() const noexcept;

    /// @brief Checks whether material is @a occlusive. 
    bool isOcclusionSeparateMap() const noexcept;

private:
    /// @brief Name of material.
    std::string m_sName;

    /// @brief A flag showing whether the material allows the light
    /// reflect in case when @b inverted normal met.
    bool m_bDoubleSided;

    /// @brief A value, which specifies the mode of the @a alpha channel.
    AlphaMode m_eAlphaMode;
    /// @brief A threshold defining full-transparency of the material.
    float m_fAlphaCutOff;

    /// @brief Information on the texture maps, channels and applicable transformations.
    TextureData m_textureData[TexCount];

    /// @brief An array of @a base-color factors.
    fvec4 m_baseColorFactors;
    /// @brief An array of @a emissive factors. 
    fvec3 m_emissiveFactors;

    /// @brief A scaling for @a x and @a y components in the texture @a normal.
    float m_fNormalScale;
    /// @brief The strength factor for the @a occlusion.
    float m_fOcclusionStrength;

    /// @brief Material @a metallness and @a roughness factors.
    union
    {
        struct {
            /// @brief Metallic coefficient.
            float m_fMetFactor;
            /// @brief Roughness coefficient.
            float m_fRoughFactor;
        };
        /// @brief Metallic and roughness coefficients.
        fvec2 data;
    } m_uMetRough;

    /// @brief Material @a index-of-refraction value.
    float m_fIor;
};

#endif // !PBRMATERIAL_H