//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      AssetDefs.cpp
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-05-08
/// @brief     Auxilliary definitions for AssetBase.h and AssetInfo.h

#include "AssetInfo.h"

std::vector<std::string> AssetBase::m_assetTypeString = {
    "unknown",
    "reference",
    "geometry",
    "material",
    "material_ior",
    "scene",
    "sensor"
};

std::vector<std::string> AssetInfo::m_assetCategoryString = {
    "unknown",
    "unlabeled",
    "ego-vehicle",
    "rectification-border",
    "out-of-roi",
    "static",
    "dynamic",
    "ground",
    "road",
    "sidewalk",
    "parking",
    "rail-track",
    "building",
    "wall",
    "fence",
    "guard-rail",
    "bridge",
    "tunnel",
    "pole",
    "polegroup",
    "traffic-light",
    "traffic-sign",
    "vegetation",
    "terrain",
    "sky",
    "person",
    "rider",
    "car",
    "truck",
    "bus",
    "caravan",
    "trailer",
    "train",
    "motorcycle",
    "bicycle",
    "license-plate"
};