//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      RenderScene.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Rendering scene model

#ifndef RENDERSCENE_H
#define RENDERSCENE_H

#include <list>

#include "embree4/rtcore.h"

#include "Background.h"
#include "DistanceRandom.h"
#include "RenderLight.h"
#include "RenderMaterial.h"
#include "RenderMesh.h"

/**
 * @brief @c RenderScene class represents a global rendering @a scene and accommodates all
 * rendering objects (@a lights, @a meshes, @a instances, @a materials, @a background).
 */
class RenderScene final
{
    /// @privatesection @a Embree API.

    /// @brief @a Embree rendering device.
    RTCDevice m_embreeDevice;
    /// @brief @a Embree rendering scene.
    RTCScene m_embreeScene;
    /// @brief @a Embree error-log list.
    std::list<std::string> m_embreeErrors;

    /// @brief Internal @a Embree function which build @a Embree.
    /// Errors are reported to error @p log .
    bool buildEmbreeScene(std::list<std::string>* log);


    /// @privatesection Scene @a attributes.

    /// @brief List of scene meshes, avaliable in the scene.
    RenderMesh *m_pMeshes;
    /// @brief Number of scene meshes, avaliable in the scene.
    int m_iMeshesN;

    /// @brief List of scene instances, avaliable in the scene.
    RenderInstance *m_pInstances;
    /// @brief Number of scene instances, avaliable in the scene.
    int m_iInstancesN;

    /// @brief List of scene materials, avaliable in the scene.
    RenderMaterial **m_ppMaterials;
    /// @brief Number of scene materials, avaliable in the scene.
    int m_iMaterialsN;

    /// @brief List of scene lights, avaliable in the scene.
    RenderLight **m_ppLights;
    /// @brief Number of scene lights, avaliable in the scene.
    int m_iLightsN;
    /// @brief Class to store @a CDF of lights for sampling based on their intensities.
    DistanceRandom<float> m_lightsSampler;

    /// @brief Scene @a background.
    Background *m_pBackground;

    /// @brief List of textures, avaliable in the scene.
    BitmapTexture *m_pTextures;
    /// @brief Number of textures, avaliable in the scene.
    int m_iTexturesN;
    /// @brief Class for generating random values.
    RandomSampler m_sampler;

#ifdef FC_VALIDATION
    /// @brief @a False-color rendering manager.
    FCHandler* fchandler;
#endif // !FC_VALIDATION

    /// @brief Indicates the state of the scene being possibly rendered.
    bool m_bTraceReady;

#ifdef FC_VALIDATION
    /// @brief Maximum mesh density.
    float m_fMaxDensity;
    /// @brief Minimum mesh density.
    float m_fMinDensity;
#endif // !FC_VALIDATION

public:
    /// @publicsection Basic interface (e.g. @a -ctors).

    /// @brief Creates new empty instance of @c RenderScene .
    RenderScene();
    /// @brief -Dtor.
    ~RenderScene();

    /** 
     * @brief Allocates the space on the rendering scene for rendering entities.
     * @param meshesN      Number of scene meshes.
     * @param instancesN   Number of scene instances.
     * @param materialsN   Number of scene materials.
     * @param texturesN    Number of scene textures.
     * @param lightsN      Number of scene lights.
     */
    void allocate(int meshesN, int instancesN, int materialsN, int texturesN, int lightsN);
    /// @brief Releases all allocated on the rendering scene capacity.
    void free();

public:
    /// @publicsection Getters and setters.

    /** 
     * @brief Sets a mesh on the scene with geometry, material and texture data.
     * 
     * @param meshId    Mesh index: [0, meshesN);
     * @param vertN     Number of vertices;
     * @param facesN    Number of faces;
     * @param texN      Number of texture coordinate blocks used for model;
     * @param matId     Material index;
     * @param faces     Raw buffer of faces;
     * @param vert      Raw buffer of vertices;
     * @param vstride   Vertex stride;
     * @param norm      Raw buffer of normals;
     * @param nstride   Normal stride;
     * @param tans      Raw buffer of tangents;
     * @param tanstride Tangent stride;
     * @param tex       Array [0, texN) of buffers of texture coordinates (unused buffers could be @c nullptrs );
     * @param texstride Stride for blocks of texture coordinates (all blocks should have same stride);
     */
    void setMesh(int meshId, int vertN, int facesN, int texN, int matId, unsigned int* faces,
                float* vert, int vstride,
                float* norm, int nstride,
                float* tans, int tanstride,
                float* tex[], int texstride);

    /// @brief Binds the mesh index @p meshId and transformation @p trans
    /// to the scene instance @p instId .
    void setInstance(int instId, const fmat4 trans, int meshId);
    /// @brief Sets the material under index @p matId to the scene material @p newMaterial.
    void setMaterial(int matId, RenderMaterial* newMaterial);
    /// @brief Sets the light object under index @p lightId to the scene light @p newLight.
    void setLight(int lightId, RenderLight* newLight);
    /// @brief Acquise source image @p data with a resolution @p tw x @p th to the
    /// texture under @p texId index or creates its copy, if @p copy is set to @c true .
    void setTexture(int texId, int tw, int th, ubvec4* data, bool copy = true);
    /// @brief Sets the scene background to @p b.
    void setBackground(Background* b){m_pBackground=b;}
    
#ifdef FC_VALIDATION
    /// @brief Sets the @a false-color-rendering subject.
    void setFalseColorSubject(const char *);
    /// @brief Removes @a false-color-rendering subject.
    void removeFalseColorSubject();
#endif // !FC_VALIDATION

    /// @brief Returns @a Embree scene.
    RTCScene getEmbreeScene(){return m_embreeScene;}    
    /// @brief Returns @a Embree error-log list. 
    std::list<std::string>& getRunErrors(){return m_embreeErrors;}
    /// @brief Returns scene background.
    Background* getBackgound(){return m_pBackground;}
    /// @brief Returns random generator. 
    RandomSampler& getSampler(){return m_sampler;}
    /// @brief Returns an array of scene meshes. 
    RenderMesh* getMeshes(){return m_pMeshes;}
    /// @brief Returns the number of scene meshes. 
    int meshesCount()const{return m_iMeshesN;}
    /// @brief Returns an array of scene instances. 
    RenderInstance* getInstances(){return m_pInstances;}
    /// @brief Returns the number of scene instances.
    int instancesCount()const{return m_iInstancesN;}
    /// @brief Returns an array of scene materials. 
    RenderMaterial** getMaterials(){return m_ppMaterials;}
    /// @brief Returns the number of scene materials.
    int materialsCount()const{return m_iMaterialsN;}
    /// @brief Returns an array of scene lights. 
    RenderLight** getLights(){return m_ppLights;}
    /// @brief Returns the number of scene lights.
    int lightsCount()const{return m_iLightsN;}
    /// @brief Returns an array of scene lights. 
    RenderLight* sampleLight(float &pdf);
    /// @brief Returns an array of available textures. 
    BitmapTexture* getTextures(){return m_pTextures;}
    /// @brief Returns the number of available textures.
    int texturesCount()const{return m_iTexturesN;}

#ifdef FC_VALIDATION
    /// @brief Returns the maximum density within the scene. 
    float getMaxDensity() const noexcept {return m_fMaxDensity;}
    /// @brief Returns the maximum density within the scene. 
    float getMinDensity() const noexcept {return m_fMinDensity;}
#endif // !FC_VALIDATION

public:
    /// @publicsection Interaction interface.

    /// @brief Commits scene to the rendering pipeline and writes all
    /// errors to the error log @p log . Returns @c true , if the commitment
    /// was successful.
    bool commitScene(std::list<std::string>* log = nullptr);
};

#endif // !RENDERSCENE_H