//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      ReferenceLink.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-08-07
/// @brief     Support for glTF extension OpenMaterial_reference_link

#ifndef REFERENCELINK_H
#define REFERENCELINK_H

#include "json.hpp"

#include "Utils.h"

/// @brief This class supports the gltf extension. It checks if json object contains
/// OpenMaterial_reference_link.
class ReferenceLink
{
public:
    static bool hasReferenceLink(const nlohmann::json& j);

    /// Constructor for the class
    explicit ReferenceLink(const nlohmann::json& j);

    std::string getTitle() const;
    std::string getUri() const;

private:
    /// Human readable title of referenced asset
    std::string m_sTitle;

    /// URI of referenced asset
    std::string m_sUri;
};

#endif // !REFERENCELINK_H