//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      FCHandler.cpp
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     False color handler for asset validation

#ifdef FC_VALIDATION

#include <iostream>

#include "doctest.h"

#include "FCHandler.h"

/// @brief Unit test for @ref fcSetSubject .
TEST_CASE("Testing fcSetSubject")
{
    struct DummySubject final : GenericFCSubject
    {
        DummySubject(int, bool, float) {}
        ~DummySubject() override = default;

    public:
        void paint(const GenericFCArgument &, fvec3) const noexcept override {}
        const std::string name() const noexcept override { return "dummy"; }
    };

    FCHandler fcHandler;
    fcSetSubject(&fcHandler, "dummy", {
        OPTIONAL_FC_SUBJECT(PrimitiveIDFC),
        OPTIONAL_FC_SUBJECT(MeshDensityFC),
        OPTIONAL_FC_SUBJECT(DummySubject, 42, true, 69.0f),
        OPTIONAL_FC_SUBJECT(InvertedNormalFC)
    });

    auto subject = dynamic_cast<const DummySubject *>(fcHandler.getSubject());
    CHECK(subject != nullptr);
}

/// @brief Unit test for @ref FCHandler::setEmpty.
TEST_CASE("Testing FCHandler::setEmpty")
{
    FCHandler fcHandler;
    fcHandler.setEmpty();

#if FC_NO_EMPTY_SUBJECTS
    CHECK(fcHandler.getSubject() == nullptr);
#else
    auto subject = dynamic_cast<const EmptySubject *>(fcHandler.getSubject());

    fvec3 color;
    subject->paint({}, color);
    CHECK(std::all_of(color, color + 3, [](float v) { return v == 0.0f; }));
#endif // !FC_NO_EMPTY_SUBJECTS
}


FCHandler::~FCHandler()
{
    delete m_pSubject;
}

void FCHandler::reset() noexcept
{
    if (m_pSubject) {
        delete m_pSubject;
        m_pSubject = nullptr;
    }
}


void FCHandler::setEmpty() noexcept
{
    reset();
#if FC_NO_EMPTY_SUBJECTS
    m_pSubject = nullptr;
#else
    m_pSubject = new EmptySubject;
#endif // !FC_NO_EMPTY_SUBJECTS
}


void fcSetSubject(FCHandler *fchandler, const char *name, const std::initializer_list<FCSubjectProvider> &list)
{
    fchandler->reset();

    GenericFCSubject *subject = nullptr;
    return (std::find_if(list.begin(), list.end(), std::bind(&FCSubjectProvider::operator(), _1, name, std::ref(subject))) != list.end()) ?
        fchandler->setSubject(subject) :
        fchandler->setEmpty();
}

#endif // !FC_VALIDATION