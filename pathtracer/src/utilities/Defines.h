//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      defines.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-25
/// @brief     Preprocessor defines

#ifndef DEFINES_H
#define DEFINES_H

#include <cstring>
#include <type_traits>
#include <complex>

using namespace std::placeholders;

/* FOREGROUND */
#define BDPT_RST    "\x1B[0m"
#define BDPT_KRED   "\x1B[31m"
#define BDPT_KGRN   "\x1B[32m"
#define BDPT_KYEL   "\x1B[33m"
#define BDPT_KBLU   "\x1B[34m"
#define BDPT_KMAG   "\x1B[35m"
#define BDPT_KCYN   "\x1B[36m"
#define BDPT_KWHT   "\x1B[37m"

#define BDPT_FRED(x)    BDPT_KRED x BDPT_RST
#define BDPT_FGRN(x)    BDPT_KGRN x BDPT_RST
#define BDPT_FYEL(x)    BDPT_KYEL x BDPT_RST
#define BDPT_FBLU(x)    BDPT_KBLU x BDPT_RST
#define BDPT_FMAG(x)    BDPT_KMAG x BDPT_RST
#define BDPT_FCYN(x)    BDPT_KCYN x BDPT_RST
#define BDPT_FWHT(x)    BDPT_KWHT x BDPT_RST

#define BDPT_BOLD(x)    "\x1B[1m" x BDPT_RST
#define BDPT_UNDL(x)    "\x1B[4m" x BDPT_RST

#define BDPT_COMMAND_LINE_DEFAULT   BDPT_BOLD(BDPT_FCYN("default:"))
#define BDPT_COMMAND_UNSPECIFIED    BDPT_FYEL("unspecified")

#define BDPT_UNUSED(expr)   static_cast<void>(expr)

#define BDPT_DEFINE_COPY_MODE(Class, mode)      \
    Class(const Class &) = mode;                \
    Class &operator=(const Class &) = mode;

#define BDPT_DEFINE_MOVE_MODE(Class, mode)      \
    Class(Class &&) = mode;                     \
    Class &operator=(Class &&) = mode;

/// @brief Enumerator for a unified color channel matching.
enum ColorChannel : size_t
{
    Red,
    Green,
    Blue,
    Alpha
};

#endif // !DEFINES_H