//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      RenderScene.cpp
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Rendering scene model

#include "RenderScene.h"
#include "DiffuseColorMaterial.h"

void embreeErrorHandler(void *userPtr, RTCError, const char *str)
{
    auto RS=reinterpret_cast<RenderScene*>(userPtr);
    RS->getRunErrors().emplace_back(str);
}

RenderScene::RenderScene() : m_sampler(3254638)
{    
    m_iMeshesN=0;
    m_pMeshes=nullptr;
    m_iInstancesN=0;
    m_pInstances=nullptr;
    m_iMaterialsN=0;
    m_ppMaterials=nullptr;
    m_pTextures=nullptr;
    m_iTexturesN=0;
    m_iLightsN=0;
    m_ppLights=nullptr;
    m_pBackground=nullptr;
#ifdef FC_VALIDATION
    fchandler=nullptr;
#endif // !FC_VALIDATION

    m_embreeDevice = rtcNewDevice(nullptr);
    m_embreeScene=nullptr;
    m_bTraceReady=false;
    if (m_embreeDevice)
    {
        rtcSetDeviceErrorFunction(m_embreeDevice, embreeErrorHandler, this);
    }
    
#ifdef FC_VALIDATION
    m_fMaxDensity = 0.0f;
    m_fMinDensity = fInfinity;
#endif // !FC_VALIDATION
}

void RenderScene::allocate(int meshN, int instN, int matN, int textN, int ligN)
{
    free();
    m_iMeshesN=meshN;
    if (m_iMeshesN>0)
        m_pMeshes=new RenderMesh[m_iMeshesN];

    m_iInstancesN=instN;
    if (m_iInstancesN>0)
        m_pInstances=new RenderInstance[m_iInstancesN];

    m_iMaterialsN=matN+1;
    m_ppMaterials=new RenderMaterial*[m_iMaterialsN+1];
    std::memset(m_ppMaterials,0,sizeof(RenderMaterial*)*static_cast<size_t>(m_iMaterialsN+1));
    fvec4 noMatColor = {BDPT_MISSING_MATERIAL_COLOR, 1.0f};
    // Missing material pointer is always treated as a hypothetical diffuse color material
    // and is being subscripted with an index one bigger than the actual number of materials
    m_ppMaterials[this->m_iMaterialsN-1]=new DiffuseColorMaterial(noMatColor);

    m_iLightsN=ligN;
    if (m_iLightsN>0)
    {
        m_ppLights=new RenderLight*[m_iLightsN];
        std::memset(m_ppLights,0,sizeof(RenderLight*)*static_cast<size_t>(m_iLightsN));
    }

    m_iTexturesN=textN;
    if (m_iTexturesN>0)
        m_pTextures=new BitmapTexture[m_iTexturesN];
}

void RenderScene::setMesh(int meshId, int vertN, int facesN, int texN, int matId, unsigned int* faces,
                                float* vert, int vstride,
                                float* norm, int nstride,
                                float* tans, int tanstride,
                                float* tex[], int texstride)
{
    if (meshId<0 || meshId>=m_iMeshesN) return;
    RenderMesh& m=m_pMeshes[meshId];
    int utexN=0;
    for (int i=0; i<texN; ++i) utexN+=tex[i]?1:0;
    m.allocate(meshId,vertN,facesN,utexN,matId);
    m.setFaces(faces);
    if (vert) m.setVertices(vert,vstride);
    if (norm) m.setNormals(norm,nstride);
    if (tans) m.setTangents(tans,tanstride);
    for (int i=0; i<texN; ++i)
        if (tex[i]) m.setTextureCoordinates(i,tex[i],texstride);
}

void RenderScene::setInstance(int instId, const fmat4 trans, int meshId)
{
    if (instId<0 || instId>=m_iInstancesN) return;
    RenderInstance& ri=m_pInstances[instId];
    ri.m_iInstId=instId;
    ri.m_iMeshId=meshId;
    std::memcpy(ri.m_transformation,trans,sizeof(fmat4));
    ri.computeNormalMatrix();
}

void RenderScene::setMaterial(int matId, RenderMaterial* newMaterial)
{
    if (matId<0 || matId>=m_iMaterialsN-1) return;
    m_ppMaterials[matId]=newMaterial;
}

void RenderScene::setLight(int lightId, RenderLight* newLight)
{
    if (lightId<0 || lightId>=m_iLightsN) return;
    m_ppLights[lightId]=newLight;
}

void RenderScene::setTexture(int texId, int tw, int th, ubvec4* data, bool copy)
{
    if (texId<0 || texId>=m_iTexturesN) return;
    m_pTextures[texId].set(tw,th,data,copy);
}

#ifdef FC_VALIDATION
void RenderScene::setFalseColorSubject(const char *name)
{
    if (fchandler) removeFalseColorSubject();
    fchandler = new FCHandler;

    fcSetSubject(fchandler, name, {
        OPTIONAL_FC_SUBJECT(PrimitiveIDFC, std::accumulate(m_pMeshes, m_pMeshes + m_iMeshesN, 0, [](int maxFN, const RenderMesh &m) { return maxFN + m.m_iFN; })),
        OPTIONAL_FC_SUBJECT(GeometryIDFC, m_iMeshesN),
        OPTIONAL_FC_SUBJECT(MaterialIDFC, m_iMaterialsN),
        OPTIONAL_FC_SUBJECT(MetallicCftFC),
        OPTIONAL_FC_SUBJECT(RoughnessCftFC),
        OPTIONAL_FC_SUBJECT(MeshDensityFC),
        OPTIONAL_FC_SUBJECT(SurfaceGradientFC),
        OPTIONAL_FC_SUBJECT(MaterialNameFC, {{"om", {0.0f, 1.0f, 0.0f}}, {"pbr", {0.0f, 0.0f, 1.0f}}}),
        OPTIONAL_FC_SUBJECT(InvertedNormalFC)
    });
}

void RenderScene::removeFalseColorSubject()
{
    delete fchandler;
    fchandler=nullptr;
}
#endif // !FC_VALIDATION

bool RenderScene::commitScene(std::list<std::string>* log)
{
    if (m_bTraceReady) return true;
    for (int i=0; i<m_iMeshesN; ++i)
    {
        int normChId=-1;
        if (m_pMeshes[i].m_iMatId>=0 && m_pMeshes[i].m_iMatId<m_iMaterialsN)
        {
            RenderMaterial* rm=m_ppMaterials[m_pMeshes[i].m_iMatId];
            if (rm)
            {
                normChId=rm->getNormalTextureChannel();
            }
            else
            {
                if (log) log->push_back(std::to_string(i)+" mesh refers to undefined material"+
                                        std::to_string(m_pMeshes[i].m_iMatId));
                m_pMeshes[i].m_iMatId=m_iMaterialsN-1;
            }
        }
        else m_pMeshes[i].m_iMatId=m_iMaterialsN-1;
        if (!m_pMeshes[i].commitMesh(normChId))
        {
            if (log) log->push_back(std::to_string(i)+" mesh is inconsistent");
        }
    }
    int falseInst=0;
    for (int i=0; i<m_iInstancesN; ++i)
    {
        if (m_pInstances[i].m_iMeshId<0 || m_pInstances[i].m_iMeshId>=m_iMeshesN ||
            !m_pMeshes[m_pInstances[i].m_iMeshId].isValid())
        {
            m_pInstances[i].m_iMeshId=-1;
            ++falseInst;
        }
    }
    if (falseInst && log) log->push_back(std::to_string(falseInst)+" from "+
                                         std::to_string(m_iInstancesN)+" instances are invalid");

    if (falseInst==m_iInstancesN) return false; // Nothing to render

#ifdef FC_VALIDATION
    for (int i=0; i<m_iInstancesN; ++i)
    {
        auto meshMinMaxDensity = m_pMeshes[m_pInstances[i].m_iMeshId].getMinMaxDensity(m_pInstances[i].m_transformation);
        m_fMinDensity = std::min(m_fMinDensity, meshMinMaxDensity.first);
        m_fMaxDensity = std::max(m_fMaxDensity, meshMinMaxDensity.second);
    }
#endif // !FC_VALIDATION

    for (int i=0; i<m_iMaterialsN; ++i)
    {
        m_ppMaterials[i]->setTextures(m_pTextures,m_iTexturesN);
        m_ppMaterials[i]->setRandomSampler(&m_sampler);
#ifdef FC_VALIDATION
        m_ppMaterials[i]->setFalseColorHandler(fchandler);
#endif // !FC_VALIDATION
    }
    m_lightsSampler.setCount(m_iLightsN);
    for (int i=0; i<m_iLightsN; ++i)
    {
        m_ppLights[i]->m_iIndex=i;
        m_ppLights[i]->setRandomSampler(&m_sampler);
        m_lightsSampler.setDistance(i,m_ppLights[i]->getPower());
    }
    if (m_iLightsN>0) m_lightsSampler.calculate();

    return buildEmbreeScene(log);
}

bool RenderScene::buildEmbreeScene(std::list<std::string>* log)
{
    if (!m_embreeDevice)
    {
        if (log) log->push_back("Embree devide is not initialized");
        return false;
    }
    m_embreeScene=rtcNewScene(m_embreeDevice);
    if (!m_embreeScene)
    {
        if (log) log->push_back("Can not create embree scene");
        return false;
    }
    rtcSetSceneFlags(m_embreeScene, RTC_SCENE_FLAG_ROBUST);
    rtcSetSceneBuildQuality(m_embreeScene, RTC_BUILD_QUALITY_HIGH);

    for (int i=0; i<m_iMeshesN; ++i)
    {
        RenderMesh& m=m_pMeshes[i];
        if (!m.isValid()) continue;
        m.m_embreeScene=rtcNewScene(m_embreeDevice);
        RTCGeometry embreeMesh=rtcNewGeometry(m_embreeDevice,RTC_GEOMETRY_TYPE_TRIANGLE);
        rtcSetSharedGeometryBuffer(embreeMesh,RTC_BUFFER_TYPE_INDEX,0,RTC_FORMAT_UINT3,m.m_pF,0,sizeof(uivec3),static_cast<size_t>(m.m_iFN));
        rtcSetSharedGeometryBuffer(embreeMesh,RTC_BUFFER_TYPE_VERTEX,0,RTC_FORMAT_FLOAT3,m.m_pfV,static_cast<size_t>(m.m_iVertexOffs)*sizeof(float),
                                   static_cast<size_t>(m.m_iStride)*sizeof(float),static_cast<size_t>(m.m_iVN));
        rtcCommitGeometry(embreeMesh);
        rtcAttachGeometryByID(m.m_embreeScene,embreeMesh,static_cast<unsigned int>(i)); // returned in RTCRayHit.hit.geomID
        rtcReleaseGeometry(embreeMesh);
        rtcCommitScene(m.m_embreeScene);
    }
    for (int i=0; i<m_iInstancesN; ++i)
    {
        RenderInstance& ri=m_pInstances[i];
        if (ri.m_iMeshId<0) continue;
        if (!m_pMeshes[ri.m_iMeshId].m_embreeScene) continue;
        RTCGeometry geoInst=rtcNewGeometry(m_embreeDevice,RTC_GEOMETRY_TYPE_INSTANCE);
        rtcSetGeometryInstancedScene(geoInst,m_pMeshes[ri.m_iMeshId].m_embreeScene);
        rtcSetGeometryTransform(geoInst,0,RTC_FORMAT_FLOAT3X4_ROW_MAJOR,ri.m_transformation);
        rtcCommitGeometry(geoInst);
        rtcAttachGeometryByID(m_embreeScene,geoInst,static_cast<unsigned int>(i));
        rtcReleaseGeometry(geoInst);
    }    


    rtcCommitScene(m_embreeScene);
    m_bTraceReady=true;
    return true;
}

RenderLight* RenderScene::sampleLight(float &pdf)
{
    int id=m_lightsSampler.getRandom(m_sampler.rand());
    pdf=m_lightsSampler.getPdf(id);
    return id>=0?m_ppLights[id]:nullptr;
}

void RenderScene::free()
{
    delete[] m_pMeshes;
    m_pMeshes=nullptr;
    m_iMeshesN=0;
    if (m_embreeScene) rtcReleaseScene(m_embreeScene);
    m_embreeScene=nullptr;

    delete[] m_pInstances;
    m_pInstances=nullptr;
    m_iInstancesN=0;

    for (int i=0; i<m_iMaterialsN; ++i) delete m_ppMaterials[i];
    delete[] m_ppMaterials;
    m_ppMaterials=nullptr;
    m_iMaterialsN=0;    
    delete[] m_pTextures;
    m_pTextures=nullptr;
    m_iTexturesN=0;
    for (int i=0; i<m_iLightsN; ++i) delete m_ppLights[i];
    delete[] m_ppLights;
    m_ppLights=nullptr;
    m_iLightsN=0;
    m_lightsSampler.setCount(0);
    delete m_pBackground;
    m_pBackground=nullptr;
    m_embreeErrors.clear();
    m_bTraceReady=false;
#ifdef FC_VALIDATION
    removeFalseColorSubject();
#endif
}

RenderScene::~RenderScene()
{
    free();
    if (m_embreeDevice) rtcReleaseDevice(m_embreeDevice);
}
