//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      AssetMaterial.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-08-01
/// @brief     Material properties and methods to load from glTF model

#ifndef ASSETMATERIAL_H
#define ASSETMATERIAL_H

#include "AssetMaterialIor.h"
#include "Material.h"
#include "Uuid.h"

class AssetMaterial;

/// @brief Material schemes.
enum MaterialScheme
{
    MATERIAL_SCHEME_SURFACE,        // Surface interaction
    MATERIAL_SCHEME_SUB_SURFACE,    // Sub-surface interaction
    MATERIAL_SCHEME_VOLUME,         // Volume interaction
};

/// @brief List of applicable @a sensors.
struct ApplicableSensors final
{
    /// @brief Material is applicable for @a camera sensor (visible light).
    bool bCamera        = false;
    /// @brief Material is applicable for @a LIDAR sensor.
    bool bLidar         = false;
    /// @brief Material is applicable for @a RADAR sensor.
    bool bRadar         = false;
    /// @brief Material is applicable for @a ultrasound sensor.
    bool bUltrasound    = false;
};

/// @brief Type of material.
struct MaterialType final
{
    /// @brief Set to @c true if material is @a isotropic, @c false if @a anisotropic.
    bool bIsotropic     = true;
    /// @brief Set to @c true if material is @a homogeneous, @c false if @a nonhomogeneous.
    bool bHomogeneous   = true;
    /// @brief Set to @c true if material is @a magnetic, @c false if @a nonmagnetic.
    bool bMagnetic      = false;
};

/// @brief @a Subsurface interaction properties.
struct Subsurface final
{
    /// @brief Set to @c true , if subsurface interactions is to be considered in addition to
    /// main surface interactions, @c false otherwise.
    bool bSubsurface = false;
    /// @brief @a Thickness of volume to be considered as @a subsurface. The default value
    /// is the penetration depth which will be calculated by material model.
    float fSubsurfaceThickness = 0.0f;
};

/// @brief @a Coating refers to a layer of transparent or semi-transparent material on top
/// of another material (e.g. layer of oil or water on top of asphalt).
struct CoatingMaterial final
{
    /// @brief Reference to external material (i.e. via @a uri ).
    std::string sMaterialRef;
    /// @brief Pointer to external material with @a coating.
    AssetMaterial *pCoatingMaterial = nullptr;

    /// @brief @a Thickness of the coating layer in @b micrometer .
    float fLayerThickness;
};

/// @brief @a Ingredients are considered as impurities on top of the main material
/// (e.g. @a oxidization might be an ingredient of a @a metal ).
struct Ingredient final
{
    /// @brief Reference to external material (i.e. via @a uri ).
    std::string sMaterialRef;
    /// @brief Pointer to external material with @a ingredient.
    AssetMaterial *pMaterialIngredient = nullptr;

    /// @brief Link to external map of material distribution which describes how material
    /// and the @a ingredients are distributed over the geometry.
    std::string sDistributionPatternUri;
};

/// @brief Struct for roughness parameters.
struct SurfaceRoughness final
{
    /// @brief Surface height @a root-mean-square in @b micrometer .
    float fSurfaceHeight            = 0.f;
    /// @brief Surface @a correlation-length in @b micrometer .
    float fSurfaceCorrelationLength = 0.f;
};


/**
 * @brief @c AssetMaterial class provides support for material asset and stores all parameters
 * of an asset of type @a material. The class is a data container and allows reading material
 * parameters from the glTF extension @a OpenMaterial_material_parameters, and the
 * access to the parameters.
 */
class AssetMaterial final : public AssetInfo, public Material
{
    /// @brief Name of material.
    std::string m_sName;
    /// @brief @a Filename of material (used for checking if material is previously loaded).
    std::string m_sFilename;
    /// @brief Classification of the material (e.g. @a "solid-metal-aluminum" ).
    std::string m_sMaterialClassification = "unknown";

    /// @brief Indicates whether @a geometrical-optics approximation is to be used.
    bool m_bGeometricalOptics           = true;
    /// @brief Indicates whether @a diffraction computations is be included.
    bool m_bIncludeDiffraction          = false;
    /// @brief Indicates whether numerical methods (like @a BEM, @a FEM, @a FDTD, or @a FMM )
    /// are to be used.
    bool m_bIncludeNumericalSimulation  = false;

    /// @brief Material scheme of type @ref MaterialScheme .
    MaterialScheme m_eMaterialScheme = MATERIAL_SCHEME_SURFACE;
    /// @brief Material type of type @ref MaterialType .
    MaterialType m_stMaterialType;
    /// @brief Applicable @a sensors.
    ApplicableSensors m_stApplicableSensors;

    /// @brief @a Temperature of the asset in @b Kelvin .
    float m_fTemperature            = 300.0f;
    /// @brief @a Mean-free path in @b micrometer (for volumetric materials).
    float m_fMeanFreePath           = 0.0f;
    /// @brief @a Density of scatterers in the volume (in @b micrometer^-3 ).
    float m_fParticleDensity        = 0.0f;
    /// @brief Effective @a cross-section (in @b micrometer^2 ) of scatterers in the volume.
    float m_fParticleCrossSection   = 0.0f;
    /// @brief @a Emissivity factor of material as a @a Lambertian emitter.
    float m_fLambertEmission        = 0.0f;

    /// @brief @a URI to file with @a emissivity coefficient values.
    std::string m_sEmissivityCoefficientUri;
    /// @brief @a URI to an external file with @a displacement data.
    std::string m_sSurfaceDisplacementUri;

    /// @brief @a Subsurface interaction properties.
    Subsurface m_stSubsurface;
    /// @brief Surface @a roughness parameters.
    SurfaceRoughness m_stSurfaceRoughness;

    /// @brief Material @a coating properties.
    std::vector<CoatingMaterial> m_stCoatingMaterial;
    /// @brief Material @a ingredients.
    std::vector<Ingredient> m_stIngredient;

    /// @brief @a URI to @a glTF file with @a index-of-refraction (aka @a IOR ) data.
    std::string m_sIndexOfRefractionUri;
    /// @brief Pointer to @a IOR data.
    AssetMaterialIor *m_pMaterialIor = nullptr;

private:
    /// @publicsection @a Radar implementation.

    /// @brief Effective particle area of a material.
    float m_fEffectiveParticleArea = 0.0f;

    /// @brief Ratio of the capacitance of a capacitor using that material as a dielectric,
    /// compared with a similar capacitor that has vacuum as its dielectric.
    /// @a URI to an external file with @a permittivity data.
    std::string m_sRelativePermittivityUri;
    /// @brief Ratio of the permeability of a specific medium to the permeability of free space.
    /// @a URI to an external file with @a permeability data.
    std::string m_sRelativePermeabilityUri;
    /// @brief Quantifies how the material conducts electric current.
    /// @a URI to an external file with @a conductivity data.
    std::string m_sConductivityUri;

private:
    /// @publicsection @a Ultrasound sensor implementation.

    /// @brief Physical property of tissue indicating how much resistance an @a ultrasound beam
    /// encounters as it passes through a tissue (in @b kg/(m^2*s) ).
    float m_fAcousticImpedance  = 0.0f;
    /// @brief @a Shear velocity is used to describe @a shear-related motion in moving fluids.
    float m_fShearVelocity      = 0.0f;

private:
    /// @publicsection Helping interface.

    void loadPropertiesFromJson(const nlohmann::json& j);
    void print(std::ostream& os) const override;

public:
    /// @publicsection Basic interface (e.g. -ctors).

    /// @brief Creates uninitialized @c AssetMaterial object.
    AssetMaterial();
    /// @brief Creates new @c AssetMaterial from @a json-object @p j stored in directory @p rcsDirectory .
    explicit AssetMaterial(const nlohmann::json &j, const std::string &rcsDirectory = "");
    /// @brief Creates new @c AssetMaterial from @a glTF file @p rcsFilename ( @c std::string ).
    explicit AssetMaterial(const std::string &rcsFilename);
    /// @brief Creates new @c AssetMaterial from @a glTF file @p cpFilename (C-string).
    explicit AssetMaterial(const char *cpFilename);

    BDPT_DEFINE_COPY_MODE(AssetMaterial, delete)
    BDPT_DEFINE_MOVE_MODE(AssetMaterial, default)

    ~AssetMaterial() override;

public:
    /// @publicsection Getters.

    /// @brief Returns human-readable @a name of material.
    std::string getName() const;
    /// @brief Returns material @a filename.
    std::string getFilename() const override;
    /// @brief Returns @a classification of the material.
    std::string getMaterialClassification() const;

    /// @brief Checks whether @a geometrical-optics approximation is to be used.
    bool getGeometricalOptics() const noexcept;
    /// @brief Checks whether @a diffraction computations is be included.
    bool getIncludeDiffraction() const noexcept;
    /// @brief Checks whether numerical methods are to be used.
    bool getIncludeNumericalSimulation() const noexcept;

    /// @brief Returns material scheme of type @ref MaterialScheme .
    MaterialScheme getMaterialScheme() const noexcept;
    /// @brief Returns material scheme in a form of string.
    std::string getMaterialSchemeAsString() const;
    /// @brief Returns material type of type @ref MaterialType .
    MaterialType getMaterialType() const noexcept;
    /// @brief Returns applicable @a sensors.
    ApplicableSensors getApplicableSensors() const noexcept;

    /// @brief Returns temperature of the asset in @b Kelvin .
    float getTemperature() const noexcept;
    /// @brief Returns mean-free path in @b micrometer (for volumetric materials).
    float getMeanFreePath() const noexcept;
    /// @brief Returns density of scatterers in the volume (in @b micrometer^-3 ).
    float getParticleDensity() const noexcept;
    /// @brief Returns effective @a cross-section (in @b micrometer^2 ) of scatterers in the volume.
    float getParticleCrossSection() const noexcept;
    /// @brief Returns @a emissivity factor of material as a @a Lambertian emitter.
    float getLambertEmission() const noexcept;

    /// @brief Returns @a URI to file with @a emissivity coefficient values.
    std::string getEmissivityCoefficientUri() const;
    /// @brief Returns @a URI to an external file with @a displacement data.
    std::string getSurfaceDisplacementUri() const;

    /// @brief Returns @a subsurface interaction properties.
    Subsurface getSubsurface() const noexcept;
    /// @brief Returns surface @a roughness parameters.
    SurfaceRoughness getSurfaceRoughness() const noexcept;

    /// @brief Returns material @a coating properties.
    std::vector<CoatingMaterial> getCoatingMaterials() const;
    /// @brief Returns material @a ingredients.
    std::vector<Ingredient> getIngredients() const;

    /// @brief Returns @a URI to @a glTF file with @a index-of-refraction (aka @a IOR ) data.
    std::string getIndexOfRefractionUri() const;
    /// @brief Returns pointer to @a IOR data.
    const AssetMaterialIor *getIorPointer() const noexcept;

public:
    /// @publicsection @a Radar implementation.

    /// @brief Returns effective particle area of a material.
    float getEffectiveParticleArea() const noexcept;
    /// @brief Returns @a URI to an external file with @a permittivity data.
    std::string getRelativePermittivityUri() const;
    /// @brief Returns @a URI to an external file with @a permeability data.
    std::string getRelativePermeabilityUri() const;
    /// @brief Returns @a URI to an external file with @a conductivity data.
    std::string getConductivityUri() const;

public:
    /// @publicsection @a Ultrasound sensor implementation.

    /// @brief Returns @a acoustic @a impedance of the @a ultrasound beam (in @b kg/(m^2*s) ).
    float getAcousticImpedance() const noexcept;
    /// @brief Returns @a shear velocity.
    float getShearVelocity() const noexcept;
};

#endif // !ASSETMATERIAL_H