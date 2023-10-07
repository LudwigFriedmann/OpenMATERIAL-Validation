//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      AssetMaterial.cpp
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-08-01
/// @brief     Material properties and methods to load from glTF model

#include "AssetMaterial.h"
#include "doctest.h"

/// @brief Unit test for AssetMaterial::AssetMaterial 
TEST_CASE("Testing AssetMaterial::AssetMaterial")
{
	std::string filename = "../materials/aluminium.gltf";
	#ifdef _WIN32
		std::replace(filename.begin(), filename.end(), '/', '\\');
	#endif

	// Check if correct material is loaded
	auto *pMaterial = new AssetMaterial(filename);
	std::string title = "material_aluminium";
	CHECK(title.compare(pMaterial->getTitle()) == 0);
}

AssetMaterial::~AssetMaterial()
{
	delete m_pMaterialIor;
}

AssetMaterial::AssetMaterial() = default;

AssetMaterial::AssetMaterial(const nlohmann::json& j, const std::string& rcsDirectory)
    : AssetInfo(j, rcsDirectory)
{
    // Check if asset is of type material
    if(!typeIsMaterial()) {
        throw GltfError(getUuidAsString().append(": asset is not of type material"));
    }
    try {
        loadPropertiesFromJson(j);
    }
    catch (const nlohmann::detail::out_of_range& exception) {
        throw GltfError(getUuidAsString().append(": ").append(exception.what()));
    }
}

AssetMaterial::AssetMaterial(const std::string& rcsFilename)
    : AssetMaterial(readJsonFile(rcsFilename), utils::path::dirname(rcsFilename))
{
    m_sFilename = rcsFilename;
}

AssetMaterial::AssetMaterial(const char *cpFilename) : AssetMaterial(std::string(cpFilename)) {}


std::string AssetMaterial::getName() const
{
    return m_sName;
}

std::string AssetMaterial::getFilename() const
{
    return m_sFilename;
}

std::string AssetMaterial::getMaterialClassification() const
{
    return m_sMaterialClassification;
}


bool AssetMaterial::getGeometricalOptics() const noexcept
{
    return m_bGeometricalOptics;
}

bool AssetMaterial::getIncludeDiffraction() const noexcept
{
    return m_bIncludeDiffraction;
}

bool AssetMaterial::getIncludeNumericalSimulation() const noexcept
{
    return m_bIncludeNumericalSimulation;
}


MaterialScheme AssetMaterial::getMaterialScheme() const noexcept
{
    return m_eMaterialScheme;
}

std::string AssetMaterial::getMaterialSchemeAsString() const
{
    switch(m_eMaterialScheme)
    {
        case MATERIAL_SCHEME_SURFACE:     return "surface";
        case MATERIAL_SCHEME_SUB_SURFACE: return "subsurface";
        case MATERIAL_SCHEME_VOLUME:      return "volume";
        default: return "";
    }
}

MaterialType AssetMaterial::getMaterialType() const noexcept
{
    return m_stMaterialType;
}

ApplicableSensors AssetMaterial::getApplicableSensors() const noexcept
{
    return m_stApplicableSensors;
}


float AssetMaterial::getTemperature() const noexcept
{
    return m_fTemperature;
}

float AssetMaterial::getMeanFreePath() const noexcept
{
    return m_fMeanFreePath;
}

float AssetMaterial::getParticleDensity() const noexcept
{
    return m_fParticleDensity;
}

float AssetMaterial::getParticleCrossSection() const noexcept
{
    return m_fParticleCrossSection;
}

float AssetMaterial::getLambertEmission() const noexcept
{
    return m_fLambertEmission;
}


std::string AssetMaterial::getEmissivityCoefficientUri() const
{
    return m_sEmissivityCoefficientUri;
}

std::string AssetMaterial::getSurfaceDisplacementUri() const
{
    return m_sSurfaceDisplacementUri;
}


Subsurface AssetMaterial::getSubsurface() const noexcept
{
    return m_stSubsurface;
}

SurfaceRoughness AssetMaterial::getSurfaceRoughness() const noexcept
{
    return m_stSurfaceRoughness;
}


std::vector<CoatingMaterial> AssetMaterial::getCoatingMaterials() const
{
    return m_stCoatingMaterial;
}

std::vector<Ingredient> AssetMaterial::getIngredients() const
{
    return m_stIngredient;
}


std::string AssetMaterial::getIndexOfRefractionUri() const
{
    return m_sIndexOfRefractionUri;
}

const AssetMaterialIor *AssetMaterial::getIorPointer() const noexcept
{
    return m_pMaterialIor;
}


float AssetMaterial::getEffectiveParticleArea() const noexcept
{
    return m_fEffectiveParticleArea;
}

std::string AssetMaterial::getRelativePermittivityUri() const
{
    return m_sRelativePermittivityUri;
}

std::string AssetMaterial::getRelativePermeabilityUri() const
{
    return m_sRelativePermeabilityUri;
}

std::string AssetMaterial::getConductivityUri() const
{
    return m_sConductivityUri;
}


float AssetMaterial::getAcousticImpedance() const noexcept
{
    return m_fAcousticImpedance;
}

float AssetMaterial::getShearVelocity() const noexcept
{
    return m_fShearVelocity;
}


void AssetMaterial::loadPropertiesFromJson(const nlohmann::json& j)
{
    using nlohmann::json;
    using std::string;

    const json jMaterial = j.at("materials").at(0);

    m_sName = jMaterial.at("name").get<std::string>();

    const json jMaterialParameters = jMaterial.at("extensions").at("OpenMaterial_material_parameters");

    // User preferences
    const json jUserPreferences = jMaterialParameters.at("user_preferences");
    m_bGeometricalOptics = jUserPreferences.at("geometrical_optics").get<bool>();

    m_bIncludeDiffraction = jUserPreferences.at("include_diffraction").get<bool>();
    m_bIncludeNumericalSimulation = jUserPreferences.at("include_numerical_simulation").get<bool>();

    auto sMaterialScheme = jUserPreferences.at("material_scheme").get<std::string>();
    if (sMaterialScheme == "surface")
        m_eMaterialScheme = MATERIAL_SCHEME_SURFACE;
    else if (sMaterialScheme == "subsurface")
        m_eMaterialScheme = MATERIAL_SCHEME_SUB_SURFACE;
    else if (sMaterialScheme == "volume")
        m_eMaterialScheme = MATERIAL_SCHEME_VOLUME;
    else
        throw GltfError(getUuidAsString().append(": unknown material scheme"));

    m_sMaterialClassification = jUserPreferences.at("material_classification").get<std::string>();

    const json jMaterialType = jUserPreferences.at("material_type");
    m_stMaterialType.bIsotropic = jMaterialType.at("isotropic").get<bool>();
    m_stMaterialType.bHomogeneous = jMaterialType.at("homogeneous").get<bool>();
    m_stMaterialType.bMagnetic = jMaterialType.at("magnetic").get<bool>();

    m_fTemperature = jUserPreferences.at("temperature").get<float>();
    if (m_fTemperature < 0)
        throw GltfError(getUuidAsString().append(": temperature must be non-negative"));

    m_sSurfaceDisplacementUri = jUserPreferences.at("surface_displacement_uri").get<std::string>();
    
    const json jSurfaceRoughness = jUserPreferences.at("surface_roughness");
    m_stSurfaceRoughness.fSurfaceHeight = jSurfaceRoughness.at("surface_height_rms").get<float>();
    m_stSurfaceRoughness.fSurfaceCorrelationLength = jSurfaceRoughness.at("surface_correlation_length").get<float>();
    if(m_stSurfaceRoughness.fSurfaceHeight < 0)
        throw GltfError(getUuidAsString().append(": surface_height must be non-negative"));
    if(m_stSurfaceRoughness.fSurfaceCorrelationLength < 0)
        throw GltfError(getUuidAsString().append(": surface_correlation_length must be non-negative"));

    // Coating
    const json jCoating = jUserPreferences.at("coating_materials");
    for(const json& jC : jCoating)
    {
        CoatingMaterial mC;
        mC.sMaterialRef = jC.at("material_ref").get<std::string>();
        mC.fLayerThickness = jC.at("layer_thickness").get<float>();
        m_stCoatingMaterial.push_back(mC);
    }
    // Ingredients
    const json jIngredient = jUserPreferences.at("ingredients");
    for(const json& jC : jIngredient)
    {
        Ingredient mC;
        mC.sMaterialRef = jC.at("material_ref").get<std::string>();
        mC.sDistributionPatternUri = jC.at("order").get<std::string>();
        m_stIngredient.push_back(mC);
    }
    // Note: coating_materials is not supported yet

    m_fLambertEmission = jUserPreferences.at("lambert_emission").get<float>();
    if(m_fLambertEmission < 0)
        throw GltfError(getUuidAsString().append(": lambert_emission must be non-negative"));

    const json jSubsurface = jUserPreferences.at("subsurface");
    m_stSubsurface.bSubsurface = jSubsurface.at("subsurface").get<bool>();
    m_stSubsurface.fSubsurfaceThickness = jSubsurface.at("subsurface_thickness").get<float>();
    if(m_stSubsurface.bSubsurface && m_stSubsurface.fSubsurfaceThickness < 0)
        throw GltfError(getUuidAsString().append(": subsurface_thickness must be non-negative"));

    // Note: ingredients is not supported yet

    // Physical properties
    const json jPhysicalProperties = jMaterialParameters.at("physical_properties");

    m_sIndexOfRefractionUri = jPhysicalProperties.at("refractive_index_uri").get<std::string>();
    if(m_sIndexOfRefractionUri.length() > 0)
    {
        const string csFilename = getDirectory() + m_sIndexOfRefractionUri;
        m_pMaterialIor = new AssetMaterialIor(csFilename);
    }

    m_fMeanFreePath = jPhysicalProperties.at("mean_free_path").get<float>();
    if(m_fMeanFreePath < 0)
        throw GltfError(getUuidAsString().append(": mean_free_path must be non-negative"));

    m_fParticleDensity = jPhysicalProperties.at("particle_density").get<float>();
    if(m_fParticleDensity < 0)
        throw GltfError(getUuidAsString().append(": particle_density must be non-negative"));

    m_fParticleCrossSection = jPhysicalProperties.at("particle_cross_section").get<float>();
    if(m_fParticleCrossSection < 0)
        throw GltfError(getUuidAsString().append(": particle_cross_section must be non-negative"));

    m_sEmissivityCoefficientUri = jPhysicalProperties.at("emissive_coefficient_uri").get<std::string>();

    const json jApplicableSensors = jPhysicalProperties.at("applicable_sensors");
    for (const json& jSensor : jApplicableSensors)
    {
        const string sSensor = jSensor.get<std::string>();
        if (sSensor == "camera")
            m_stApplicableSensors.bCamera = true;
        else if (sSensor == "lidar")
            m_stApplicableSensors.bLidar = true;
        else if (sSensor == "radar")
            m_stApplicableSensors.bRadar = true;
        else if (sSensor == "ultrasound")
            m_stApplicableSensors.bUltrasound = true;
        else
            throw GltfError(getUuidAsString().append(": unknown sensor type ").append(sSensor));
    }

    // Radar
    m_fEffectiveParticleArea = jPhysicalProperties.at("effective_particle_area").get<float>();
    m_sRelativePermittivityUri = jPhysicalProperties.at("relative_permittivity_uri").get<std::string>();
    m_sRelativePermeabilityUri = jPhysicalProperties.at("relative_permeability_uri").get<std::string>();
    m_sConductivityUri = jPhysicalProperties.at("conductivity_uri").get<std::string>();

    // Ultrasound
    m_fAcousticImpedance = jPhysicalProperties.at("acoustic_impedance").get<float>();
    m_fShearVelocity = jPhysicalProperties.at("shear_velocity").get<float>();
}

void AssetMaterial::print(std::ostream& os) const
{
    using std::endl;

    const MaterialType type = this->getMaterialType();
    const SurfaceRoughness roughness = this->getSurfaceRoughness();
    const Subsurface subsurface = this->getSubsurface();
    const ApplicableSensors applicableSensors = this->getApplicableSensors();

    os << std::boolalpha
        << "    name: " << this->getName() << endl
        << "    user_preferences:" << endl
        << "        geometricalOptics: " << this->getGeometricalOptics() << endl
        << "        includeDiffraction: " << this->getIncludeDiffraction() << endl
        << "        includeNumericalSimulation: " << this->getIncludeNumericalSimulation() << endl
        << "        materialScheme: " << this->getMaterialSchemeAsString() << endl
        << "        materialClassification: \"" << this->getMaterialClassification() << "\"" << endl
        << "        materialType:" << endl
        << "            isotropic: " << type.bIsotropic << endl
        << "            homogeneous: " << type.bHomogeneous << endl
        << "            magnetic: " << type.bMagnetic << endl
        << "        temperature: " << this->getTemperature() << endl
        << "        surfaceDisplacementUri: \"" << this->getSurfaceDisplacementUri() << "\"" << endl
        << "        surfaceRoughness:" << endl
        << "            surfaceHeight: " << roughness.fSurfaceHeight << endl
        << "            surfaceCorrelationLength: " << roughness.fSurfaceCorrelationLength << endl
        << "        lambertEmission: " << this->getLambertEmission() << endl
        << "        subsurface:" << endl
        << "            subsurface: " << subsurface.bSubsurface << endl
        << "            subsurfaceThickness: " << subsurface.fSubsurfaceThickness << endl
        << "        coatingMaterials:" << endl
        << "            coatingMaterial.materialRef: \"" << this->getCoatingMaterials()[0].sMaterialRef << "\"" << endl
        << "            coatingMaterial.fLayerThickness: " << this->getCoatingMaterials()[0].fLayerThickness << endl
        << "        ingredients:" << endl
        << "            ingredient.materialRef: \"" << this->getIngredients()[0].sMaterialRef << "\"" << endl
        << "            ingredient.order: \"" << this->getIngredients()[0].sDistributionPatternUri << "\"" << endl
        << "    physical_properties:" << endl
        << "        refractiveIndexUri: \"" << this->getIndexOfRefractionUri() << "\"" << endl
        << "        meanFreePath: " << this->getMeanFreePath() << endl
        << "        particleDensity: " << this->getParticleDensity() << endl
        << "        particleCrossSection: " << this->getParticleCrossSection() << endl
        << "        emissivityCoefficientUri: \"" << this->getEmissivityCoefficientUri() << "\"" << endl
        << "        applicableSensors:" << endl
        << "            camera: " << applicableSensors.bCamera << endl
        << "            lidar: " << applicableSensors.bLidar << endl
        << "            radar: " << applicableSensors.bRadar << endl
        << "        radar_related_properties:" << endl
        << "            effectiveParticleArea: " << this->getEffectiveParticleArea() << endl
        << "            relativePremittivityUri: \"" << this->getRelativePermittivityUri() << "\"" << endl
        << "            relativePermeabilityUri: \"" << this->getRelativePermeabilityUri() << "\"" << endl
        << "        ultrasound_related_properties:" << endl
        << "            acousticImpedance: " << this->getAcousticImpedance() << endl
        << "            shearVelocity: " << this->getShearVelocity() << endl
        << std::noboolalpha;
}
