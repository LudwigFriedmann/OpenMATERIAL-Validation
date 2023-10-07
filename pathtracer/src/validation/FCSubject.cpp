//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      FCSubject.cpp
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     False color rendering subjects

#ifdef FC_VALIDATION

#include <cassert>
#include <set>

#include "doctest.h"

#include "AssetGeometry.h"
#include "FCSubject.h"
#include "materials/PbrMaterial.h"
#include "Random.h"

struct Fvec3Wrapper final
{
    fvec3 d = {0.0f};

    Fvec3Wrapper() = default;
    Fvec3Wrapper(const float * p) { std::copy_n(p, 3, d); }
    operator float *() noexcept { return d; }
    operator const float *() const noexcept { return d; }
};

namespace projectors {

    struct GetAsEnumerator final
    {
        using type = typename GenericFCArgument::Enumerator;
        type operator()(const GenericFCArgument &arg) const { return arg.getAsEnumerator(); }
    };

    struct GetAsClampedFloat final
    {
        using type = typename GenericFCArgument::ClampedFloat;
        type operator()(const GenericFCArgument &arg) const { return arg.getAsClampedFloat(); }
    };

    struct GetAsBoolean final
    {
        using type = typename GenericFCArgument::Boolean;
        type operator()(const GenericFCArgument &arg) const { return arg.getAsBoolean(); }
    };
} // !projectors

namespace comparators {

    template<typename T, typename Filter, typename Comp>
    struct TypedCompare final
    {
        bool operator()(const T &l, const T &r) const { return Comp{}(Filter{}(l), Filter{}(r)); }
    };

    template<typename T, size_t Size, typename Comp>
    struct LexicographicalTypedCompare final
    {
        bool operator()(const T *l, const T *r) const { return std::lexicographical_compare(l, l + Size, r, r + Size, Comp{}); }
    };
} // !comparators

namespace predicates {

    template<typename T>
    struct LessThan final
    {
        constexpr LessThan(T v = {}) : value(v) {}
        constexpr bool operator()(const T &x) const noexcept { return x < value; }
        T value;
    };

    template<typename T>
    struct EqualsTo final
    {
        constexpr EqualsTo(T v = {}) : value(v) {}
        constexpr bool operator()(const T &x) const noexcept { return !LessThan<T>{value}(x) && !LessThan<T>{x}(value); }
        T value;
    };

    template<typename T>
    struct GreaterThan final
    {
        constexpr GreaterThan(T v = {}) : value(v) {}
        constexpr bool operator()(const T &x) const noexcept { return !LessThan<T>{value}(x) && !EqualsTo<T>{x}(value); }
        T value;
    };
} // !predicates


/// @brief Unit test for @ref PrimitiveIDFC::paint / @ref GeometryIDFC::paint (same structure).
TEST_CASE("Testing PrimitiveIDFC::paint / GeometryIDFC::paint")
{
    using Enumerator = typename PrimitiveIDFC::Enumerator;

    // Assume 3 meshes of the geometry
    constexpr Enumerator categorySize   = 3;
    constexpr Enumerator meshesHit      = 30;
    assert(categorySize > 0 && meshesHit > 0);
    
    GeometryIDFC geometrySubject(categorySize);

    // The indices of hit meshes vary between 0 and 4 inclusive
    std::vector<Enumerator> hitMeshes(meshesHit);
    std::generate_n(hitMeshes.begin(), hitMeshes.size(), std::bind(&Random::uniformIntDistribution<Enumerator>, 0, 4));
    std::for_each(hitMeshes.begin(), hitMeshes.end(), [&geometrySubject](Enumerator i) {
        fvec3 color;
        geometrySubject.paint(i, color);
        if (i < geometrySubject.size()) {
            CHECK(std::any_of(color, color + 3, predicates::GreaterThan<float>{}));
        }
        else {
            CHECK(std::all_of(color, color + 3, predicates::EqualsTo<float>{}));
        }
    });
}

/// @brief Unit test for @ref PrimitiveIDFC::paint / @ref GeometryIDFC::paint (same functionality).
TEST_CASE("Testing PrimitiveIDFC::paint / GeometryIDFC::paint")
{
    using Enumerator = typename PrimitiveIDFC::Enumerator;

    // Assume 4 faces at most found within meshes
    constexpr Enumerator categorySize   = 4;
    constexpr Enumerator imageSize      = 30;
    assert(categorySize > 0 && imageSize > 0);

    // Assume some random primitive id to be at each image pixel (including invalid ones, e.g. overflowed)
    PrimitiveIDFC subject(categorySize);
    std::multiset<GenericFCArgument, comparators::TypedCompare<GenericFCArgument, projectors::GetAsEnumerator, std::less<Enumerator>>> args;
    std::generate_n(std::inserter(args, args.begin()), imageSize, std::bind(&Random::uniformIntDistribution<Enumerator>, 0, categorySize));

    std::multiset<Fvec3Wrapper, comparators::LexicographicalTypedCompare<float, 3, std::less<float>>> colors;
    std::transform(args.begin(), args.end(), std::inserter(colors, colors.begin()), [&subject](const GenericFCArgument &arg) -> Fvec3Wrapper {
        fvec3 color;
        subject.paint(arg.getAsEnumerator(), color);
        return color;
    });

    const fvec3 black = {0.0f, 0.0f, 0.0f};
    CHECK(args.count(Enumerator(4)) == colors.count(black));

    // Count unique colors
    Enumerator uniqueColors = 0;
    for (auto cit = colors.begin(), end = colors.end(); cit != end; cit = colors.upper_bound(cit->d)) ++uniqueColors;
    CHECK(categorySize == uniqueColors - 1); // All non-black colors
}

/// @brief Unit test for @ref MaterialIDFC::paint .
TEST_CASE("Testing MaterialIDFC::paint")
{
    using Enumerator = typename MaterialIDFC::Enumerator;

    // Assume 3 valid materials loaded
    constexpr Enumerator categorySize = 3;
    assert(categorySize > 0);

    MaterialIDFC subject(categorySize);

    // Assume 5 different materials in total being hit (including the missing material)
    std::vector<Enumerator> hitMaterials(5, -1);
    std::iota(std::next(hitMaterials.begin()), hitMaterials.end(), Enumerator{});

    fvec3 color;
    const fvec3 black = {0.0f, 0.0f, 0.0f}, mpink = {BDPT_MISSING_MATERIAL_COLOR};

    // Missing material check
    subject.paint(hitMaterials.front(), color);
    CHECK(std::equal(color, color + 3, mpink));

    // Normal material check
    std::for_each(std::next(hitMaterials.begin()), std::prev(hitMaterials.end()), [&subject, &color](Enumerator i) {
        subject.paint(i, color);
        CHECK(std::any_of(color, color + 3, predicates::GreaterThan<float>{}));
    });

    // Out-ranged material index check
    subject.paint(hitMaterials.back(), color);
    CHECK(std::equal(color, color + 3, black));
}

/// @brief Unit test for @ref MetallicCftFC::paint .
TEST_CASE("Testing MetallicCftFC::paint")
{
    typename MetallicCftFC::ClampedFloat metallicCft = 0.5f;

    MetallicCftFC subject;

    fvec3 color;
    const fvec3 lightblue = {0.5f, 0.5f, 1.0f};

    subject.paint(metallicCft, color);
    CHECK(std::equal(color, color + 3, lightblue));
}

/// @brief Unit test for @ref RoughnessCftFC::paint .
TEST_CASE("Testing RoughnessCftFC::paint")
{
    typename RoughnessCftFC::ClampedFloat roughnessCft = 1.0f;

    RoughnessCftFC subject;
    
    fvec3 color;
    const fvec3 green = {0.0f, 1.0f, 0.0f};

    subject.paint(roughnessCft, color);
    CHECK(std::equal(color, color + 3, green));
}

/// @brief Unit test for @ref MeshDensityFC::paint .
TEST_CASE("Testing MeshDensityFC::paint")
{
    // Triangular prism:
    //
    //            Z |
    //              |       (3)
    //              |      .ᐟ'
    //              |    .ᐟ '|
    //              |  .ᐟ  ' |
    //              |.ᐟ  .'  |
    //             (0)_.'___(2)___ Y
    //             / .'  _.-'
    //            /.'_ -'
    //          (1)'
    //        X /

    const fvec3 V[4] = {
        {0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 1.0f}
    };

    const ivec3 F[4] = {
        {0, 2, 1},
        {0, 1, 3},
        {1, 2, 3},
        {0, 3, 2}
    };

    auto densityFunction = [](float a) {
        return 1.0f / (1e-3f + a);
    };

    fvec4 D;
    // Compute areas
    std::transform(F, F + 4, D, [V](const ivec3 f) { return triangleArea(V[f[0]], V[f[1]], V[f[2]]); });
    CHECK(std::max_element(D, D + 4) == D + 1); // face with max area has an index 1

    // Compute densities
    float *maxDensityIt = nullptr;
    std::transform(D, D + 4, D, densityFunction);
    CHECK((maxDensityIt = std::max_element(D, D + 4)) == D); // face with max density has an index 0

    // Compute relative densities
    std::transform(D, D + 4, D, std::bind(std::divides<float>{}, _1, *maxDensityIt));

    MeshDensityFC subject;
    fvec3 c0, c1;
    subject.paint(D[0], c0);
    subject.paint(D[1], c1);
    // Fase with higher density is more redish
    CHECK(std::equal(c0 + 1, c0 + 3, c1 + 1, std::less<float>{}));
}

/// @brief Unit test for @ref MaterialNameFC::paint .
TEST_CASE("Testing MaterialNameFC::paint")
{
    using ByteSocket = typename MaterialNameFC::ByteSocket;

    const ByteSocket mat1 = {"mat1", {0.5f, 0.0f, 1.0f}};
    const ByteSocket mat2 = {"mat2", {0.0f, 0.8f, 1.0f}};

    MaterialNameFC subject({mat1, mat2});
    
    fvec3 color;
    const fvec3 mpink = {BDPT_MISSING_MATERIAL_COLOR};

    // Check material 1
    subject.paint("mat1", color);
    CHECK(std::equal(color, color + 3, mat1.color));

    // Check material 2
    subject.paint("mat2", color);
    CHECK(std::equal(color, color + 3, mat2.color));

    // Check missing material
    subject.paint("mat3", color);
    CHECK(std::equal(color, color + 3, mpink));
}

/// @brief Unit test for @ref InvertedNormalFC::paint .
TEST_CASE("Testing InvertedNormalFC::paint")
{
    const fvec3 inV = {-1.0f, -1.0f, 0.0f};
    const fvec3 n1 = {1.0f, 1.0f, 1.0f}, n2 = {-1.0f, 0.0f, 0.0f};
    
    InvertedNormalFC subject;
    fvec3 color;
    const fvec3 red = {1.0f, 0.0f, 0.0f}, green = {0.0f, 1.0f, 0.0f};

    predicates::LessThan<float> lt;

    subject.paint(lt(-dot3(inV, n1)), color);
    CHECK(std::equal(color, color + 3, green));

    subject.paint(lt(-dot3(inV, n2)), color);
    CHECK(std::equal(color, color + 3, red));
}

#endif // !FC_VALIDATION