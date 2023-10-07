//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      PbrMaterial.cpp
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2022-04-22
/// @brief     Material properties and methods to load from glTF model

#include <iostream>

#include "doctest.h"

#include "PbrMaterial.h"


namespace {

    PbrMaterial::AlphaMode getEnumeratedAlphaMode(const char *mode)
    {
        if (mode) {
            if (std::strcmp(mode, "BLEND") == 0) {
                return PbrMaterial::Blend;
            }
            else if (std::strcmp(mode, "MASK") == 0) {
                return PbrMaterial::Mask;
            }
            else if (std::strcmp(mode, "OPAQUE") == 0) {
                return PbrMaterial::Opaque;
            }
        }
        
        throw ValueError("Invalid alpha-blending mode");
    }

    void getTextureTransform(const tinygltf::ExtensionMap &extensions, fmat3 &TM)
    {
        fvec2 offset = {0.0f, 0.0f};
        fvec2 scale = {1.0f, 1.0f};
        float rotation = 0.0f;

        if (!extensions.empty()) {
            auto khrTexTransform = extensions.at("KHR_texture_transform");

            if (khrTexTransform.IsObject()) {
                if (khrTexTransform.Has("scale")) {
                    auto scaleTransform = khrTexTransform.Get("scale");
                    scale[0] = static_cast<float>(scaleTransform.Get(0).Get<double>());
                    scale[1] = static_cast<float>(scaleTransform.Get(1).Get<double>());
                }
                if (khrTexTransform.Has("offset")) {
                    auto offsetTransform = khrTexTransform.Get("offset");
                    offset[0] = static_cast<float>(offsetTransform.Get(0).Get<double>());
                    offset[1] = static_cast<float>(offsetTransform.Get(1).Get<double>());
                }
                if (khrTexTransform.Has("rotation")) {
                    auto rotationTransform = khrTexTransform.Get("rotation");
                    rotation = static_cast<float>(rotationTransform.Get<double>());
                }
            }
            else return;
        }
        else return;

        const float c = std::cos(rotation), s = std::sin(rotation);
        assignVector(TM[0], scale[0]*c, scale[0]*s, offset[0]);
        assignVector(TM[1],-scale[1]*s, scale[1]*c, offset[1]);
        assignVector(TM[2], 0.0f, 0.0f, 1.0f);
    }
} // !anonymous namespace


/// @brief Unit test for PbrMaterial::PbrMaterial 
TEST_CASE("Testing PbrMaterial::PbrMaterial")
{
	std::string filename = "../objects/torus_pbr.gltf";
	#ifdef _WIN32
		std::replace(filename.begin(), filename.end(), '/', '\\');
	#endif

    AssetGeometry assetGeometry(filename);
    auto pMaterial = assetGeometry.m_vpMaterials[0];
    auto pPbrMaterial = dynamic_cast<const PbrMaterial*>(pMaterial);
    auto fRoughness = pPbrMaterial->getRoughnessFactor();
    CHECK(fRoughness == 0);
}


PbrMaterial::PbrMaterial()
{
    reset();
}

PbrMaterial::PbrMaterial(const tinygltf::Model &gltfModel, int materialIndex) try : PbrMaterial()
{
    loadPropertiesFromJson(gltfModel, materialIndex);
}
catch (...)
{
    std::cerr << "Material cannot be loaded, at index: " << materialIndex << std::endl;
}

void PbrMaterial::reset() noexcept
{
    m_bDoubleSided = false;
    m_eAlphaMode = Opaque;
    m_fAlphaCutOff = 0.5f;

    std::for_each(std::begin(m_textureData), std::end(m_textureData), std::mem_fn(&TextureData::reset));
    std::fill_n(m_baseColorFactors, 4, 1.0f);
    std::fill_n(m_emissiveFactors, 3, 0.0f);

    m_fNormalScale = 1.0f;
    m_fOcclusionStrength = 1.0f;

    std::fill_n(m_uMetRough.data, 2, 1.0f);

    m_fIor = 1.5f;
}

void PbrMaterial::loadPropertiesFromJson(const tinygltf::Model &gltfModel, int materialIndex)
{
    if (materialIndex < 0) return;
    auto material = gltfModel.materials[static_cast<size_t>(materialIndex)];

    m_sName = material.name;
    m_bDoubleSided = material.doubleSided;

    m_eAlphaMode = getEnumeratedAlphaMode(material.alphaMode.data());
    m_fAlphaCutOff = clamp01f(static_cast<float>(material.alphaCutoff));

    // Base color data
    {
        int iBaseColorTextureIndex = material.pbrMetallicRoughness.baseColorTexture.index;

        auto &baseColorTextData = m_textureData[BaseColor];
        baseColorTextData.m_iChannelId = material.pbrMetallicRoughness.baseColorTexture.texCoord;
        if (iBaseColorTextureIndex > -1) {
            baseColorTextData.m_iMapId = gltfModel.textures[static_cast<size_t>(iBaseColorTextureIndex)].source;
            getTextureTransform(material.pbrMetallicRoughness.baseColorTexture.extensions, baseColorTextData.m_transformation);
        }
    }

    // Metallic-roughness data
    {
        int iMetallicRoughnessMapIndex = material.pbrMetallicRoughness.metallicRoughnessTexture.index;

        auto &metRoughTextData = m_textureData[MetallicRoughness];
        metRoughTextData.m_iChannelId = material.pbrMetallicRoughness.metallicRoughnessTexture.texCoord;
        if (iMetallicRoughnessMapIndex > -1) {
            metRoughTextData.m_iMapId = gltfModel.textures[static_cast<size_t>(iMetallicRoughnessMapIndex)].source;
            getTextureTransform(material.pbrMetallicRoughness.metallicRoughnessTexture.extensions, metRoughTextData.m_transformation);
        }
    }

    // Normal data
    {
        int iNormalMapIndex = material.normalTexture.index;

        auto &normalTextData = m_textureData[Normal];
        normalTextData.m_iChannelId = material.normalTexture.texCoord;
        if (iNormalMapIndex > -1) {
            normalTextData.m_iMapId = gltfModel.textures[static_cast<size_t>(iNormalMapIndex)].source;
            getTextureTransform(material.normalTexture.extensions, normalTextData.m_transformation);
        }
    }

    // Emissivity data
    {
        int iEmissiveMapIndex = material.emissiveTexture.index;

        auto &emissiveTextData = m_textureData[Emissivity];
        emissiveTextData.m_iChannelId = material.emissiveTexture.texCoord;
        if (iEmissiveMapIndex > -1) {
            emissiveTextData.m_iMapId = gltfModel.textures[static_cast<size_t>(iEmissiveMapIndex)].source;
            getTextureTransform(material.emissiveTexture.extensions, emissiveTextData.m_transformation);
        }
    }

    // Occlusion data
    {
        int iOcclusionMapIndex = material.occlusionTexture.index;

        auto &occlusionTextData = m_textureData[Occlusion];
        occlusionTextData.m_iChannelId = material.occlusionTexture.texCoord;
        if (iOcclusionMapIndex > -1) {
            occlusionTextData.m_iMapId = gltfModel.textures[static_cast<size_t>(iOcclusionMapIndex)].source;
            getTextureTransform(material.occlusionTexture.extensions, occlusionTextData.m_transformation);
        }
    }
    
    castVector(material.pbrMetallicRoughness.baseColorFactor.data(), m_baseColorFactors, 4);
    std::for_each(m_baseColorFactors, m_baseColorFactors + 4, clamp01f);

    castVector(material.emissiveFactor.data(), m_emissiveFactors, 3);
    std::for_each(m_emissiveFactors, m_emissiveFactors + 3, clamp01f);

    m_uMetRough.m_fMetFactor = clamp01f(static_cast<float>(material.pbrMetallicRoughness.metallicFactor));
    m_uMetRough.m_fRoughFactor = clamp01f(static_cast<float>(material.pbrMetallicRoughness.roughnessFactor));

    m_fNormalScale = static_cast<float>(material.normalTexture.scale);
    m_fOcclusionStrength = static_cast<float>(material.occlusionTexture.strength);
}

void PbrMaterial::print(std::ostream& os) const
{
    fvec4 rgba;
    getBaseColorFactors(rgba);

    using std::endl;

    os <<  "    name: " << this->getMaterialName() << endl
        << "    pbr_properties:" << endl
        << "        redChannelValue: " << rgba[0] << endl
        << "        greenChannelValue: " << rgba[1] << endl
        << "        blueChannelValue: " << rgba[2] << endl
        << "        alphaChannelValue: " << rgba[3] << endl
        << "        roughnessValue: " << this->getRoughnessFactor() << endl
        << "        metallicValue: " << this->getMetallicFactor() << endl;
}


std::string PbrMaterial::getMaterialName() const
{
    return m_sName;
}


bool PbrMaterial::isDoubleSided() const noexcept
{
    return m_bDoubleSided;
}

PbrMaterial::AlphaMode PbrMaterial::getAlphaMode() const noexcept
{
    return m_eAlphaMode;
}

float PbrMaterial::getAlphaCutOff() const noexcept
{
    return m_fAlphaCutOff;
}


int PbrMaterial::getTextureMapId(Texture text) const noexcept
{
    return m_textureData[text].m_iMapId;
}

int PbrMaterial::getTextureChannelId(Texture text) const noexcept
{
    return m_textureData[text].m_iChannelId;
}

size_t PbrMaterial::getTexturesUsed() const noexcept
{
    return std::count_if(std::begin(m_textureData), std::end(m_textureData), [](const TextureData &td) {
        return td.m_iMapId != -1;
    });
}


void PbrMaterial::getTextureTransformation(Texture text, fmat3 &TM) const noexcept
{
    std::memcpy(TM, m_textureData[text].m_transformation, 9 * sizeof(float));
}


void PbrMaterial::getBaseColorFactors(fvec4 dest) const noexcept
{
    assert(dest);
    assignVector4(dest, m_baseColorFactors);
}

float PbrMaterial::getBaseColorChannel(ColorChannel channel) const noexcept
{
    return m_baseColorFactors[channel];
}


void PbrMaterial::getEmissiveFactors(fvec3 dest) const noexcept
{
    assert(dest);
    assignVector3(dest, m_emissiveFactors);
}

float PbrMaterial::getEmissiveFactor(ColorChannel channel) noexcept
{
    return m_emissiveFactors[channel];
}

bool PbrMaterial::isEmissive() const noexcept
{
    return length3(m_emissiveFactors) > fEpsilon;
}


void PbrMaterial::getMetallicRoughnessFactors(fvec2 dest) const noexcept
{
    assert(dest);
    assignVector2(dest, m_uMetRough.data);
}

float PbrMaterial::getMetallicFactor() const noexcept { return m_uMetRough.m_fMetFactor; }

float PbrMaterial::getRoughnessFactor() const noexcept { return m_uMetRough.m_fRoughFactor; }


float PbrMaterial::getNormalScale() const noexcept { return m_fNormalScale; }

float PbrMaterial::getOcclusionStrength() const noexcept { return m_fOcclusionStrength; }


float PbrMaterial::getIor() const noexcept { return m_fIor; }


bool PbrMaterial::isOcclusionSeparateMap() const noexcept
{
    return m_textureData[Occlusion].m_iMapId != m_textureData[MetallicRoughness].m_iMapId;
}