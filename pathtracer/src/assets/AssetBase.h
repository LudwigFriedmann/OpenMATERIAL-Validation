//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      AssetBase.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-05-08
/// @brief     Main asset class description

#ifndef ASSETBASE_H
#define ASSETBASE_H

#include <fstream>
#include <iostream>

#include "json.hpp"

#include "Defines.h"
#include "Utils.h"
#include "Uuid.h"

/// @brief All available asset types.
enum AssetType
{
    ASSET_TYPE_UNKNOWN = 0,
    ASSET_TYPE_REFERENCE,
    ASSET_TYPE_GEOMETRY,
    ASSET_TYPE_MATERIAL,
    ASSET_TYPE_MATERIAL_IOR,
    ASSET_TYPE_SCENE,
    ASSET_TYPE_SENSOR
};

/**
 * @brief Main asset class.
 * 
 * @details This class should not be directly instanciated, but derived classes
 * should be used. Other asset implementations should inherit from this class.
 * Each asset has a unique @a UUID.
 */
class AssetBase
{
    /// @brief Human-readable asset types as @a strings (must be in the same order as in @ref AssetType ).
    static std::vector<std::string> m_assetTypeString;

protected:
    /// @protectedsection Unified properties of the asset.

    /// @brief Unique @a UUID of asset.
    Uuid m_uId;
    /// @brief Type of asset.
    AssetType m_eType = ASSET_TYPE_UNKNOWN;
    /// @brief Directory containing the loaded @a glTF file, if set.
    std::string m_sDirectory;

public:
    /// @publicsection Basic interface (e.g. -ctors).

    /// @brief Creates uninitialized @c AssetBase object.
    AssetBase() = default;
    /// @brief Creates new @c AssetBase object with the given @a UUID @p uuid and
    /// type @p eType .
    explicit AssetBase(const Uuid &uuid, AssetType eType = ASSET_TYPE_UNKNOWN) : m_uId(uuid), m_eType(eType) {}

    /// @brief Creates new @c AssetBase from @a json-object @p j stored in directory @p rcsDirectory .
    explicit AssetBase(const nlohmann::json &j, std::string rcsDirectory = "")
    {
        loadData(j, std::move(rcsDirectory));
    }
    /// @brief Creates new @c AssetBase from @a glTF file @p crsFilename ( @c std::string ).
    explicit AssetBase(const std::string& crsFilename) : AssetBase(readJsonFile(crsFilename), utils::path::dirname(crsFilename)) {}
    /// @brief Creates new @c AssetBase from @a glTF file @p cpFilename (C-string).
    explicit AssetBase(const char *cpFilename) : AssetBase(std::string(cpFilename)) {}

    virtual ~AssetBase() = default;

public:
    /// @publicsection Main interaction interface.

    /// @brief Reads file @p crsFilename and returns corresponding @a json-object.
    static nlohmann::json readJsonFile(const std::string &crsFilename)
    {
        nlohmann::json j;
        std::ifstream gltfFile(crsFilename);

        if (!gltfFile.is_open())
            throw OSError("Cannot open file '" + crsFilename + "' for reading");

        gltfFile >> j;
        gltfFile.close();

        return j;
    }

    /// @brief Returns @c true if @a json-object @p j contains @p key , otherwise returns @c false .
    static bool hasKey(const nlohmann::json& j, std::string key)
    {
        return j.find(std::move(key)) != j.end();
    }

    /// @brief Returns asset type for a given number @p eType as @a string.
    static std::string assetTypeToString(AssetType eType)
    {
        return m_assetTypeString.at(eType);
    }

    /// @brief Returns the number of asset type @p crsType (case @b insensitive ).
    static AssetType stringToAssetType(const std::string &crsType)
    {
        auto found = std::find_if(m_assetTypeString.begin(), m_assetTypeString.end(),
            std::bind(&utils::string::strcaseequal, _1, crsType)
        );

        if (found == m_assetTypeString.end()) throw GltfError("Unknown asset type " + crsType);
        return static_cast<AssetType>(std::distance(m_assetTypeString.begin(), found));
    }

    /// @brief Load asset info data from @a json-object @p j stored in directory @p rcsDirectory
    /// under @a OpenMaterial_asset_info extension.
    void loadData(const nlohmann::json &j, std::string rcsDirectory = "")
    {
        m_sDirectory = rcsDirectory + utils::path::getFileSeparator();

        if (hasKey(j.at("asset"), "extensions")) {
            if (hasKey(j.at("asset").at("extensions"), "OpenMaterial_asset_info")) {
                nlohmann::json j_asset_info = j.at("asset").at("extensions").at("OpenMaterial_asset_info");

                if (hasKey(j_asset_info, "id")) {
                    auto sUuid = j_asset_info.at("id").get<std::string>();
                    m_uId = Uuid(sUuid);
                }
                
                if (hasKey(j_asset_info, "asset_type")) {
                    auto sType = j_asset_info.at("asset_type").get<std::string>();
                    m_eType = stringToAssetType(sType);
                }
            }
        }
    }

public:
    /// @publicsection Getters.

    /// @brief Returns @a UUID of asset.
    Uuid getUuid() const noexcept { return m_uId; }
    /// @brief Returns @a UUID of asset as string.
    std::string getUuidAsString() const { return m_uId.toString(); }

    /// @brief Returns @ref AssetType type of asset.
    AssetType getType() const noexcept { return m_eType; }
    /// @brief Returns type of asset as string.
    std::string getTypeString() const { return m_assetTypeString.at(m_eType); }

    /// @brief Returns directory of the source @a jsson file. 
    std::string getDirectory() const { return m_sDirectory; }

public:
    /// @publicsection Asset type checks.

    /// @brief Returns @c true if type of asset is @a unknown, otherwise returns @c false .
    bool typeIsUnknown() const noexcept { return m_eType == ASSET_TYPE_UNKNOWN; }
    /// @brief Returns @c true if type of asset is @a reference, otherwise returns @c false .
    bool typeIsReference() const noexcept { return m_eType == ASSET_TYPE_REFERENCE; }
    /// @brief Returns @c true if type of asset is @a geometry, otherwise returns @c false .
    bool typeIsGeometry() const noexcept { return m_eType == ASSET_TYPE_GEOMETRY; }
    /// @brief Returns @c true if type of asset is @a material, otherwise returns @c false .
    bool typeIsMaterial() const noexcept { return m_eType == ASSET_TYPE_MATERIAL; }
    /// @brief Returns @c true if type of asset is @a materialior, otherwise returns @c false .
    bool typeIsMaterialIor() const noexcept { return m_eType == ASSET_TYPE_MATERIAL_IOR; }
    /// @brief Returns @c true if type of asset is @a scene, otherwise returns @c false .
    bool typeIsScene() const noexcept { return m_eType == ASSET_TYPE_SCENE; }
    /// @brief Returns @c true if type of asset is @a sensor, otherwise returns @c false .
    bool typeIsSensor() const noexcept { return m_eType == ASSET_TYPE_SENSOR; }
};

#endif // !ASSETBASE_H
