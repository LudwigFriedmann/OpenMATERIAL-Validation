//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      Material.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2022-04-26
/// @brief     Abstract definition of the material interface

#ifndef MATERIAL_H
#define MATERIAL_H

#include <string>

/**
 * @brief Class @c Material is an abstract definition of the material interface,
 * used in the scope of @a bidirectional-pathtracing workflow.
 */
struct Material
{
    /// @publicsection Default methods and basic interface (e.g. @a -ctor and @a dtor).
    
    /// @brief Creates new empty instance of @c Material .
    explicit Material() = default;
    /// @brief Default -vdtor.
    virtual ~Material() = default;

    /// @brief Dumps material @p rcMaterial to @a output stream @p os .
    friend std::ostream& operator<<(std::ostream &os, const Material &rcMaterial)
    {
        rcMaterial.print(os);
        return os;
    }

    /// @brief Gets @a filename of material (only useful for @a OpenMaterial materials).
    virtual std::string getFilename() const { return ""; }

protected:
    /// @protectedsection Helping interface.

    /// @brief Dumps current material to @a output stream @p os .
    virtual void print(std::ostream& os) const = 0;
};

#endif // !MATERIAL_H
