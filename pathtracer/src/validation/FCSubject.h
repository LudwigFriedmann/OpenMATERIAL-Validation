//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      FCSubject.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     False color rendering subjects

#if defined(FC_VALIDATION) && !defined(FCSUBJECT_H)
#define FCSUBJECT_H

#include <cctype>
#include <cassert>
#include <set>

#include "Random.h"
#include "UtilityFunctions.h"

#define FC_NO_EMPTY_SUBJECTS 0

/// @brief Gets @a red channel of the @p rgb integer.
constexpr unsigned char fcGetRed(unsigned int rgb) noexcept { return static_cast<unsigned char>((rgb >> 16) & 0xFFU); }
/// @brief Gets @a green channel of the @p rgb integer.
constexpr unsigned char fcGetGreen(unsigned int rgb) noexcept { return static_cast<unsigned char>((rgb >> 8) & 0xFFU); }
/// @brief Gets @a blue channel of the @p rgb integer.
constexpr unsigned char fcGetBlue(unsigned int rgb) noexcept { return static_cast<unsigned char>(rgb & 0xFFU); }
/// @brief Gets @a gray representation of the @p rgb integer.
constexpr unsigned int  fcGetGray(unsigned char r, unsigned char g, unsigned char b) noexcept { return (r*11 + g*16 + b*5) / 32; }


/**
 * @brief @c GenericFCArgument class represents a generic argument of the
 * @a false-color handler, as it can store information in different formats.
 * Based on the underlying value, the appropriate treatment for each pixel is
 * chosen later.
 * 
 * @details So far 4 different formats of the generic argument are supported:
 *  @a Enumerator   as @c unsigned @c int ;
 *  @a ClampedFloat as @c float ;
 *  @a Byte         as @c char ;
 *  @a Boolean      as @c bool ;
 */
struct GenericFCArgument final
{
    using Enumerator    = unsigned int;
    using ClampedFloat  = float;
    using Byte          = char;
    using Boolean       = bool;

    /// @publicsection Basic interface (e.g. @a -ctors).

    GenericFCArgument() : m_Data({}) {}

    /// @brief Constructs @c GenericFCArgument out of @c Enumerator @p value . 
    GenericFCArgument(const Enumerator value) noexcept { m_Data.m_uiEnumerator = value; }
    /// @brief Constructs @c GenericFCArgument out of @c ClampedFloat @p value . 
    GenericFCArgument(const ClampedFloat value) noexcept { m_Data.m_fClampedFloat = clamp(value, ClampedFloat(0), ClampedFloat(1)); }
    /// @brief Constructs @c GenericFCArgument out of @c Byte sequence @p value . 
    GenericFCArgument(const Byte value[]) noexcept { setAsByteData(value); }
    /// @brief Constructs @c GenericFCArgument out of @c Boolean sequence @p value . 
    GenericFCArgument(const Boolean value) noexcept { m_Data.m_bFlag = value; }

public:
    /// @publicsection Getters and setters.

    /// @brief Sets underlying value to @c Enumerator @p value . 
    void setAsEnumerator(const Enumerator value) noexcept { m_Data.m_uiEnumerator = value; };
    /// @brief Returns underlying value as @c Enumerator . 
    Enumerator getAsEnumerator() const noexcept { return m_Data.m_uiEnumerator; }

    /// @brief Sets underlying value to @c ClampedFloat @p value . 
    void setAsClampedFloat(const ClampedFloat value) noexcept { m_Data.m_fClampedFloat = clamp<ClampedFloat>(value, 0, 1); };
    /// @brief Returns underlying value as @c ClampedFloat .
    ClampedFloat getAsClampedFloat() const noexcept { return m_Data.m_fClampedFloat; }

    /// @brief Sets underlying value to @c Byte sequence @p value .
    void setAsByteData(const Byte value[]) noexcept { if (value) std::copy_n(value, sizeof(Data), m_Data.m_cByteData); };
    /// @brief Writes underlying value to @c Byte sequence @p value .
    void getAsByteData(Byte value[]) const noexcept { if (value) std::copy_n(m_Data.m_cByteData, sizeof(Data), value); }

    /// @brief Sets underlying value to @c Boolean @p value .
    void setAsBoolean(const Boolean value) noexcept { m_Data.m_bFlag = value; }
    /// @brief Returns underlying value as @c Boolean .
    Boolean getAsBoolean() const noexcept { return m_Data.m_bFlag; }

private:
    /// @brief Independent on the platform the argument is required
    /// to be aligned by 4 bytes.
    union alignas(4) Data
    {
        Enumerator      m_uiEnumerator;
        ClampedFloat    m_fClampedFloat;
        Byte            m_cByteData[4];
        Boolean         m_bFlag;
    } m_Data;
};


/// @brief @c GenericFCSubject class represents the base class for all features,
/// which can be rendered in @a false-color mode.
struct GenericFCSubject
{
    /// @publicsection Virtual interface.

    virtual ~GenericFCSubject() {}

    /// @brief Assigns the color to @p c based on the value stored at @p a .
    virtual void paint(const GenericFCArgument &a, fvec3 c) const noexcept = 0;
    /// @brief Brief subject description.
    virtual const std::string name() const noexcept = 0;
};

/// @brief Polymorphic @a false-color subject provider.
using FCSubjectProvider = std::function<bool (const char *, GenericFCSubject *&)>;

/// @brief This macro definition creates a functional object, which is capable of
/// providing a generic @a false-color rendering subject of type @p type , based on
/// its name @p n . If name matches the type, then the result is writen into @p s and
/// @c true is returned, otherwise the return value is @c false . Variadic argument list
/// should match arguments of @c type constructor.
#define OPTIONAL_FC_SUBJECT(type, ...) FCSubjectProvider([=](const char *n, GenericFCSubject *&s) { \
    static_assert(std::is_base_of<GenericFCSubject, type>::value, "Invalid subject type");          \
    auto subject = type(__VA_ARGS__);                                                               \
    if (subject.name() == n) {                                                                      \
        s = new type(std::move(subject));                                                           \
        return true;                                                                                \
    }                                                                                               \
    else return false;                                                                              \
})


#if !FC_NO_EMPTY_SUBJECTS
/// @brief @c EmptySubject class always assigns pixel with the @a black color.
struct EmptySubject final : GenericFCSubject
{
    EmptySubject() = default;
    ~EmptySubject() override = default;

public:
    /// @publicsection Virtual interface.
    /// @note See the description in the base ( @c GenericFCSubject ) class.

    void paint(const GenericFCArgument &, fvec3 color) const noexcept override { std::fill_n(color, 3, 0.0f); }
    const std::string name() const noexcept override { return "empty"; }
};
#endif // !FC_NO_EMPTY_SUBJECTS


/// @brief @c EnumeratedFCSubject is the base structure that handles those
/// @a false-color rendering categories, which can be enumeratred.
struct EnumeratedFCSubject : GenericFCSubject
{
    /// @publicsection Basic interface (e.g. @a -ctors).

    using Enumerator = typename GenericFCArgument::Enumerator;
    /// @brief Constructs @c EnumeratedFCSubject with a given category size @p categorySize .
    EnumeratedFCSubject(Enumerator categorySize = 0) noexcept
    {
        std::set<Enumerator> hashedColors;
        while (hashedColors.size() < categorySize) {
            hashedColors.insert(Random::uniformIntDistribution(1U, 0xFFFFFFU));
        }
        m_vHashedColors.resize(categorySize);
        std::copy(hashedColors.begin(), hashedColors.end(), m_vHashedColors.begin());
        std::random_shuffle(m_vHashedColors.begin(), m_vHashedColors.end());
    }
    virtual ~EnumeratedFCSubject() {}

    /// @brief Assigns a pre-randomed hashed color to @p color based on index @p enumeratedID .
    virtual void paint(const GenericFCArgument &enumeratedID, fvec3 color) const noexcept
    {
        auto id = enumeratedID.getAsEnumerator();
        auto n = (id < size()) ? m_vHashedColors[id] : Enumerator{};
        assignVector(color, static_cast<float>(fcGetRed(n)) / 255.0f, static_cast<float>(fcGetGreen(n)) / 255.0f, static_cast<float>(fcGetBlue(n)) / 255.0f);
    }

    /// @brief Returns the category size. 
    Enumerator size() const noexcept { return m_vHashedColors.size(); }

protected:
    /// @brief Hashed (into @c unsigned @c int ) random colors.
    std::vector<Enumerator> m_vHashedColors;
};

/// @brief @c PrimitiveIDFC class allows one to render scene objects in
/// @a false-color mode based on their @a primitive id.
struct PrimitiveIDFC final : EnumeratedFCSubject
{
    /// @publicsection Basic interface (e.g. @a -ctors).

    using typename EnumeratedFCSubject::Enumerator;
    PrimitiveIDFC(Enumerator categorySize = 0) noexcept : EnumeratedFCSubject(categorySize) {}
    ~PrimitiveIDFC() override = default;

public:
    /// @publicsection Virtual interface.
    /// @note See the description in the base ( @c GenericFCSubject ) class.

    const std::string name() const noexcept override { return "pid"; }
};

/// @brief @c GeometryIDFC class allows one to render scene objects in
/// @a false-color mode based on their @a geometry/instance id.
struct GeometryIDFC final : EnumeratedFCSubject
{
    /// @publicsection Basic interface (e.g. @a -ctors).

    using typename EnumeratedFCSubject::Enumerator;
    GeometryIDFC(Enumerator categorySize = 0) noexcept : EnumeratedFCSubject(categorySize) {}
    ~GeometryIDFC() override = default;

public:
    /// @publicsection Virtual interface.
    /// @note See the description in the base ( @c GenericFCSubject ) class.

    const std::string name() const noexcept override { return "gid"; }
};

/// @brief @c MaterialIDFC class allows one to render scene objects in
/// @a false-color mode based on their @a material id.
struct MaterialIDFC final : EnumeratedFCSubject
{
    /// @publicsection Basic interface (e.g. @a -ctors).

    using typename EnumeratedFCSubject::Enumerator;
    MaterialIDFC(Enumerator categorySize = 0) noexcept : EnumeratedFCSubject(categorySize) {}
    ~MaterialIDFC() override = default;

public:
    /// @publicsection Virtual interface.
    /// @note See the description in the base ( @c GenericFCSubject ) class.

    /// @brief Assigns a pre-randomed hashed color to @p color based on index @p materialID 
    /// with a @a red channel always assumed to be equal to 0.
    /// A missing material (i.e. of index of -1) always gets @b bright-pink color.
    void paint(const GenericFCArgument &materialID, fvec3 color) const noexcept override
    {
        auto id = materialID.getAsEnumerator();

        if (id != Enumerator(-1)) {
            auto n = (id < size()) ? m_vHashedColors[id] : Enumerator{};
            assignVector(color, 0.0f, static_cast<float>(fcGetGreen(n)) / 255.0f, static_cast<float>(fcGetBlue(n)) / 255.0f);
        }
        else {
            assignVector(color, BDPT_MISSING_MATERIAL_COLOR);
        }
    }

    const std::string name() const noexcept override { return "mid"; }
};


/// @brief @c ClampedFloatSubject is the base structure that handles those
/// @a false-color rendering categories, which can be ascribed with a single
/// clamped @a floating-point value (i.e. between 0 and 1).
struct ClampedFloatSubject : GenericFCSubject
{
    /// @publicsection Basic interface (e.g. @a -ctors).

    using ClampedFloat = typename GenericFCArgument::ClampedFloat;
    virtual ~ClampedFloatSubject() {}
};

/// @brief @c MetallicCftFC class allows one to render scene objects in
/// @a false-color mode based on their material @a metallic value.
struct MetallicCftFC final : ClampedFloatSubject
{
    /// @publicsection Basic interface (e.g. @a -ctors).

    using typename ClampedFloatSubject::ClampedFloat;
    MetallicCftFC() = default;
    ~MetallicCftFC() override = default;

public:
    /// @publicsection Virtual interface.
    /// @note See the description in the base ( @c GenericFCSubject ) class.

    /// @brief Assigns a unique color to @p color based on material @p metallicness value:
    /// the lowest value of @a metallicness is represented by pure @a white color,
    /// the highest one -- by pure @a blue channel.
    void paint(const GenericFCArgument &metallicness, fvec3 color) const noexcept override
    {
        const fvec3 lower = {1.0f, 1.0f, 1.0f}; // white
        const fvec3 upper = {0.0f, 0.0f, 1.0f}; // blue
        std::transform(lower, lower + 3, upper, color, std::bind(Func3D(lerp), _1, _2, metallicness.getAsClampedFloat()));
    }

    const std::string name() const noexcept override { return "mmp"; }
};

/// @brief @c RoughnessCftFC class allows one to render scene objects in
/// @a false-color mode based on their surface @a roughness value.
struct RoughnessCftFC final : ClampedFloatSubject
{
    /// @publicsection Basic interface (e.g. @a -ctors).

    using typename ClampedFloatSubject::ClampedFloat;
    RoughnessCftFC() = default;
    ~RoughnessCftFC() override = default;

public:
    /// @publicsection Virtual interface.
    /// @note See the description in the base ( @c GenericFCSubject ) class.

    /// @brief Assigns a unique color to @p color based on material @p roughness value:
    /// the lowest value of @a roughness is represented by pure @a white color,
    /// the highest one -- by pure @a green channel.
    void paint(const GenericFCArgument &roughness, fvec3 color) const noexcept override
    {
        const fvec3 lower = {1.0f, 1.0f, 1.0f}; // white
        const fvec3 upper = {0.0f, 1.0f, 0.0f}; // green
        std::transform(lower, lower + 3, upper, color, std::bind(Func3D(lerp), _1, _2, roughness.getAsClampedFloat())); 
    }

    const std::string name() const noexcept override { return "rmp"; }
};

/// @brief @c MeshDensityFC class allows one to render scene objects in
/// @a false-color mode based on their surface @a density value.
struct MeshDensityFC final : ClampedFloatSubject
{
    /// @publicsection Basic interface (e.g. @a -ctors).

    using typename ClampedFloatSubject::ClampedFloat;
    MeshDensityFC() = default;
    ~MeshDensityFC() override = default;

public:
    /// @publicsection Virtual interface.
    /// @note See the description in the base ( @c GenericFCSubject ) class.

    /// @brief Assigns a unique one-channel color to @p color based on
    /// mesh density @p normalizedArea value: the lowest value of @a density
    /// is represented by pure @a white color, the highest one -- by pure @a red.
    void paint(const GenericFCArgument &normalizedArea, fvec3 color) const noexcept override
    {
        const fvec3 lower = {1.0f, 1.0f, 1.0f}; // white
        const fvec3 upper = {1.0f, 0.0f, 0.0f}; // red
        std::transform(lower, lower + 3, upper, color, std::bind(Func3D(lerp), _1, _2, normalizedArea.getAsClampedFloat()));
    }

    const std::string name() const noexcept override { return "md"; }
};

/// @brief @c SurfaceGradientFC class allows one to render scene objects in
/// @a false-color mode based on their surface @a gradient value.
struct SurfaceGradientFC final : ClampedFloatSubject
{
    /// @publicsection Basic interface (e.g. @a -ctors).

    using typename ClampedFloatSubject::ClampedFloat;
    SurfaceGradientFC() = default;
    ~SurfaceGradientFC() override = default;

public:
    /// @publicsection Virtual interface.
    /// @note See the description in the base ( @c GenericFCSubject ) class.

    /// @brief Assigns a unique one-channel color to @p color based on
    /// mesh gradient @p gradient value: the lowest value of @a gradient
    /// is represented by pure @a white color (flat surface connection),
    /// the highest one -- by pure @a black (normals are pointing in opposite directions).
    void paint(const GenericFCArgument &gradient, fvec3 color) const noexcept override
    {
        std::fill_n(color, 3, 1.0f - gradient.getAsClampedFloat());
    }

    const std::string name() const noexcept override { return "sg"; }
};


/// @brief @c ByteFCSubject is the base structure that handles those
/// @a false-color rendering categories, which can be distiguished by a
/// short byte sequence.
struct ByteFCSubject : GenericFCSubject
{
    /// @publicsection Basic interface (e.g. @a -ctors).

    using Byte = typename GenericFCArgument::Byte;

    /// @brief An auxilliary value box for a color dictionary.
    struct ByteSocket final
    {
        const Byte *name;
        const fvec3 color;

        bool operator<(const ByteSocket &other) const noexcept
        {
            assert(isValid() && other.isValid());
            const auto size = std::min(std::strlen(name), std::strlen(other.name));
            return std::lexicographical_compare(name, name + size, other.name, other.name + size, std::less<Byte>{});
        }

        bool isValid() const noexcept
        {
            const size_t size = std::strlen(name);
            return static_cast<bool>(name) && size <= sizeof(GenericFCArgument) &&
                   std::all_of(name, name + size, [](unsigned char b) { return std::islower(b) || std::isdigit(b); }) &&
                   std::all_of(color, color + 3, [](float v) { return v == clamp01f(v); });
        }
    };

    ByteFCSubject(const std::initializer_list<ByteSocket> &dict)
    { 
        m_sDict.insert(dict);
        assert(std::all_of(m_sDict.begin(), m_sDict.end(), std::mem_fn(&ByteSocket::isValid)));
    }

    virtual ~ByteFCSubject() {}

protected:
    std::set<ByteSocket> m_sDict;
};

/// @brief @c MaterialNameFC class allows one to render scene objects in
/// @a false-color mode based on material they are made of.
struct MaterialNameFC final : ByteFCSubject
{
    /// @publicsection Basic interface (e.g. @a -ctors).

    using typename ByteFCSubject::Byte;
    using typename ByteFCSubject::ByteSocket;

    MaterialNameFC(const std::initializer_list<ByteSocket> &dict) : ByteFCSubject(dict) {}
    ~MaterialNameFC() override = default;

public:
    /// @publicsection Virtual interface.
    /// @note See the description in the base ( @c GenericFCSubject ) class.

    /// @brief Assigns a unique color to @p color based on material type (name) @p materialName .
    /// All dictionary colors are filled only @b once in advance.
    /// A missing material always gets @b bright-pink color.
    void paint(const GenericFCArgument &materialName, fvec3 color) const noexcept override
    {
        Byte mn[sizeof(GenericFCArgument)];
        materialName.getAsByteData(mn);
        auto found = this->m_sDict.find(ByteSocket{mn, {}});
        return found != this->m_sDict.end() ? assignVector3(color, found->color) : assignVector(color, BDPT_MISSING_MATERIAL_COLOR);
    }

    const std::string name() const noexcept override { return "mn"; }
};


/// @brief @c BooleanFCSubject is the base structure that handles those
/// @a false-color rendering categories, which can be distiguished by a
/// logical value.
struct BooleanFCSubject : GenericFCSubject
{
    /// @publicsection Basic interface (e.g. @a -ctors).

    using Boolean = typename GenericFCArgument::Boolean;
    virtual ~BooleanFCSubject() {}
};

/// @brief @c InvertedNormalFC class allows one to render scene objects in
/// @a false-color mode based on orientation of their normal with respect to
/// a sensor direction.
struct InvertedNormalFC final : BooleanFCSubject
{
    /// @publicsection Basic interface (e.g. @a -ctors).

    InvertedNormalFC() = default;
    ~InvertedNormalFC() override = default;

public:
    /// @publicsection Virtual interface.
    /// @note See the description in the base ( @c GenericFCSubject ) class.

    /// @brief Assigns a binary color to @p color based on the logical value @p negativeCos 
    /// (if inverted normal is met, then @p negativeCos is @c true , otherwise @p false ).
    void paint(const GenericFCArgument &negativeCos, fvec3 color) const noexcept override
    {
        auto inv = static_cast<float>(negativeCos.getAsBoolean());
        assignVector(color, inv, 1.0f - inv, 0.0f);
    }

    const std::string name() const noexcept override { return "in"; }
};

#endif // !FCSUBJECT_H