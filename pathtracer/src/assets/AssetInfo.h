//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      AssetInfo.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-05-15
/// @brief     Information about asset

// Conditional compilation to avoid multiple inclusion:
#ifndef ASSETINFO_H
#define ASSETINFO_H

#include <cstdint>
#include <utility>

#include "AssetBase.h"

/// @brief All available asset categories.
enum AssetCategory
{
    ASSET_CATEGORY_UNKNOWN = 0,
    ASSET_CATEGORY_UNLABELED,
    ASSET_CATEGORY_EGO_VEHICLE,
    ASSET_CATEGORY_RECTIFICATION_BORDER,
    ASSET_CATEGORY_OUT_OF_ROI,
    ASSET_CATEGORY_STATIC,
    ASSET_CATEGORY_DYNAMIC,
    ASSET_CATEGORY_GROUND,
    ASSET_CATEGORY_ROAD,
    ASSET_CATEGORY_SIDEWALK,
    ASSET_CATEGORY_PARKING,
    ASSET_CATEGORY_RAIL_TRACK,
    ASSET_CATEGORY_BUILDING,
    ASSET_CATEGORY_WALL,
    ASSET_CATEGORY_FENCE,
    ASSET_CATEGORY_GUARD_RAIL,
    ASSET_CATEGORY_BRIDGE,
    ASSET_CATEGORY_TUNNEL,
    ASSET_CATEGORY_POLE,
    ASSET_CATEGORY_POLEGROUP,
    ASSET_CATEGORY_TRAFFIC_LIGHT,
    ASSET_CATEGORY_TRAFFIC_SIGN,
    ASSET_CATEGORY_VEGETATION,
    ASSET_CATEGORY_TERRAIN,
    ASSET_CATEGORY_SKY,
    ASSET_CATEGORY_PERSON,
    ASSET_CATEGORY_RIDER,
    ASSET_CATEGORY_CAR,
    ASSET_CATEGORY_TRUCK,
    ASSET_CATEGORY_BUS,
    ASSET_CATEGORY_CARAVAN,
    ASSET_CATEGORY_TRAILER,
    ASSET_CATEGORY_TRAIN,
    ASSET_CATEGORY_MOTORCYCLE,
    ASSET_CATEGORY_BICYCLE,
    ASSET_CATEGORY_LICENSE_PLATE
};


/**
 * @brief Additional information of assets.
 * 
 * @details This class inherits directly from the @c AssetBase class and extends it
 * with additional asset properties (e.g. @a title, @a creator, @a description etc.).
 */
class AssetInfo : public AssetBase
{
    /// @brief Human-readable asset categories as @a strings (must be in the same order as in @ref AssetCategory ).
    static std::vector<std::string> m_assetCategoryString;

protected:
    /// @protectedsection Required properties in @a OpenMaterial_asset_info.

    /// @brief Title of asset.
    std::string m_sTitle;
    /// @brief Creator of asset.
    std::string m_sCreator;

protected:
    /// @protectedsection Optional properties in @a OpenMaterial_asset_info.
    /// @note @b Not implemented: @a asset_parent, @a asset_version, @a asset_variation, @a sources.

    /// @brief Category of asset.
    AssetCategory m_eCategory = ASSET_CATEGORY_UNKNOWN;
    /// @brief Creation date of asset.
    std::string m_sCreationDate;
    /// @brief Human-readable description of the asset.
    std::string m_sDescription;
    /// @brief Tags of the asset.
    std::string m_sTags;

public:
    /// @publicsection Basic interface (e.g. -ctors).

    /// @brief Creates uninitialized @c AssetInfo object.
    AssetInfo() = default;
    /// @brief Creates new @c AssetInfo object with the given @a UUID @p uuid, type @p eType ,
    /// title @p sTitle and creator @p sCreator .
    AssetInfo(const Uuid &uuid, AssetType eType, std::string sTitle = "", std::string sCreator = "") :
        AssetBase(uuid, eType), m_sTitle(std::move(sTitle)), m_sCreator(std::move(sCreator))
    {}

    /// @brief Creates new @c AssetInfo from @a json-object @p j stored in directory @p rcsDirectory .
    explicit AssetInfo(const nlohmann::json& j, std::string rcsDirectory = "") :
        AssetBase(j, std::move(rcsDirectory))
    {
        loadData(j);
    }
    /// @brief Creates new @c AssetInfo from @a glTF file @p crsFilename ( @c std::string ).
    explicit AssetInfo(const std::string& rcsFilename) : AssetInfo(readJsonFile(rcsFilename), utils::path::dirname(rcsFilename)) {}
    /// @brief Creates new @c AssetInfo from @a glTF file @p cpFilename (C-string).
    explicit AssetInfo(const char *cpFilename) : AssetInfo(std::string(cpFilename)) {}

    ~AssetInfo() override = default;

public:
    /// @publicsection Main interaction interface.

    /// @brief Returns asset category for a given number @p eCategory as @a string.
    static std::string assetCategoryToString(AssetCategory eCategory)
    {
        return m_assetCategoryString.at(eCategory);
    }

    /// @brief Returns the number of asset category @p crsCategory (case @b insensitive ).
    static AssetCategory stringToAssetCategory(const std::string &crsCategory)
    {
        auto found = std::find_if(m_assetCategoryString.begin(), m_assetCategoryString.end(),
            std::bind(&utils::string::strcaseequal, _1, crsCategory)
        );

        if (found == m_assetCategoryString.end()) throw GltfError("Unknown asset type " + crsCategory);
        return static_cast<AssetCategory>(std::distance(m_assetCategoryString.begin(), found));
    }

    /// @brief Load asset info data from @a json-object @p j under @a OpenMaterial_asset_info extension.
    void loadData(const nlohmann::json& j)
    {
        if (hasKey(j.at("asset"), "extensions")) {
            if (hasKey(j.at("asset").at("extensions"), "OpenMaterial_asset_info")) {
                // Parse OpenMaterial_asset_info
                const nlohmann::json j_asset_info = j.at("asset").at("extensions").at("OpenMaterial_asset_info");

                // User_preferences.at("material_scheme").Get<std::string>()
                m_sTitle   = j_asset_info.at("title").get<std::string>();
                m_sCreator = j_asset_info.at("creator").get<std::string>();

                // Parse optional properties
                if (hasKey(j_asset_info, "category")) {
                    std::string sCategory = j_asset_info.at("category").get<std::string>();
                    setCategory(stringToAssetCategory(sCategory));
                }
                if (hasKey(j_asset_info, "creation_date")) {
                    std::string sCreationDate = j_asset_info.at("creation_date").get<std::string>();
                    setCreationDate(sCreationDate);
                }
                if (hasKey(j_asset_info, "description")) {
                    std::string sDescription = j_asset_info.at("description").get<std::string>();
                    setDescription(sDescription);
                }
                if (hasKey(j_asset_info, "tags")) {
                    std::string sTags = j_asset_info.at("tags").get<std::string>();
                    setTags(sTags);
                }
            }
        }
    }

public:
    /// @publicsection Getters and setters.

    /// @brief Returns title of asset.
    std::string getTitle() const { return m_sTitle; }
    /// @brief Returns creator of asset.
    std::string getCreator() const { return m_sCreator; }

    /// @brief Sets category of asset to @p eAssetCategory .
    void setCategory(AssetCategory eAssetCategory) noexcept { m_eCategory = eAssetCategory; }
    /// @brief Returns category of asset as number.
    AssetCategory getCategory() const noexcept { return m_eCategory; }
    /// @brief Returns category of asset as string.
    std::string getCategoryString() const { return m_assetCategoryString.at(m_eCategory); }

    /// @brief Sets creation date to @p sCreationDate .
    void setCreationDate(std::string sCreationDate) { m_sCreationDate = std::move(sCreationDate); }
    /// @brief Returns creation date.
    std::string getCreationDate() const { return m_sCreationDate; }

    /// @brief Sets description to @p sDescription .
    void setDescription(std::string sDescription) { m_sDescription = std::move(sDescription); }
    /// @brief Returns description.
    std::string getDescription() const { return m_sDescription; }

    /// @brief Sets tags to @p sTags .
    void setTags(std::string sTags) { m_sTags = std::move(sTags); }
    /// @brief Returns tags.
    std::string getTags() const { return m_sTags; }
};

#endif // !ASSETINFO_H