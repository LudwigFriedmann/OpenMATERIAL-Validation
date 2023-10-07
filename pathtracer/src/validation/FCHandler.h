//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      FCHandler.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     False color handler for asset validation

#if defined(FC_VALIDATION) && !defined(FCHANDLER_H)
#define FCHANDLER_H

#include "FCSubject.h"

/**
 * @brief @c FCHandler class manages the @a false-color rendering subjects
 * in the frame of @a false-color asset validation.
 */
class FCHandler final
{
    /// @brief @a Polymorphic false-color rendering subject.
    GenericFCSubject *m_pSubject = nullptr;

public:
    /// @publicsection Default methods and basic interface (e.g. @a -ctors).

    FCHandler() = default;
    ~FCHandler();

    /// @brief Removes existing @a false-color rendering subject.
    void reset() noexcept;

public:
    /// @publicsection Getters and setters.

    /// @brief Returns current @a false-color rendering subject.
    const GenericFCSubject *getSubject() const noexcept { return m_pSubject; }
    /// @brief Sets @a false-color rendering subject to @p subject .
    void setSubject(GenericFCSubject *subject) noexcept { m_pSubject = subject; }
    /// @brief Sets @a empty @a false-color rendering.
    void setEmpty() noexcept;
};

/// @brief Sets the @a false-color rendering subject based on its @p name to @a false-color handler
/// @p fchandler . The appropriate subject is chosen out of the provide list of optional subjects @p list .
void fcSetSubject(FCHandler *fchandler, const char *name, const std::initializer_list<FCSubjectProvider> &list);

#endif // !FCHANDLER_H