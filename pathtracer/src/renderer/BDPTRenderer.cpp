//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      BDPTRenderer.cpp
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Bidirectional pathtracing rederer class

#include <iostream>
#include <cassert>

#include "BDPTRenderer.h"

BDPTRenderer::BDPTRenderer()
{
    m_pParams=nullptr;
    m_pScene=nullptr;
    m_ppCPBuf=nullptr;
    m_ppLPBuf=nullptr;   
    m_pPathMem=nullptr;
}

void BDPTRenderer::setParameters(RendererParameters* p) noexcept
{
    m_pParams=p;
}

void BDPTRenderer::setScene(RenderScene* s) noexcept
{
    m_pScene=s;
}

float BDPTRenderer::maxAbsInVector(fvec3 v, float th)
{
    fvec3 va={std::abs(v[0]),std::abs(v[1]),std::abs(v[2])};
    float mv=va[0]>va[1]&&va[0]>va[2]?va[0]:va[1]>va[2]?va[1]:va[2];
    return mv<th?th:mv;
}

void BDPTRenderer::shiftRayAlongDirection(RTCRayHit &ray, float epsScale)
{
    fvec3 rvec={ray.ray.org_x,ray.ray.org_y,ray.ray.org_z};
    float v=maxAbsInVector(rvec);
    float delta=epsScale*fEpsilon*v;
    ray.ray.org_x+=delta*ray.ray.dir_x;
    ray.ray.org_y+=delta*ray.ray.dir_y;
    ray.ray.org_z+=delta*ray.ray.dir_z;
}

float BDPTRenderer::intensity(const fvec3 r)
{
    return 0.299f*r[0]+0.587f*r[1]+0.114f*r[2];
}

void BDPTRenderer::computeSurfacePoint(RTCRayHit& hit, SurfacePoint& sp)
{
    auto meshes = m_pScene->getMeshes();
    sp.m_pMesh=&meshes[hit.hit.geomID];
    sp.m_pInst=&m_pScene->getInstances()[hit.hit.instID[0]];
    sp.m_iFaceId=static_cast<int>(hit.hit.primID);
#ifdef FC_VALIDATION
    sp.m_iGlobalFaceId=sp.m_iFaceId+std::accumulate(meshes, meshes + sp.m_pMesh->m_iId, 0, [](int maxFN, const RenderMesh &m) { return maxFN + m.m_iFN; });
#endif // !FC_VALIDATION
    assignVector(sp.m_bary,1.0f-hit.hit.u-hit.hit.v,hit.hit.u,hit.hit.v);
    sp.m_pMesh->computeSurfacePoint(sp,sp.m_pInst->m_transformation,sp.m_pInst->m_normMatrix);
#ifdef FC_VALIDATION
    sp.m_fRefDensity = clamp01f(1.0f -
        std::log(m_pScene->getMaxDensity()/clamp(sp.m_fRefDensity, m_pScene->getMinDensity(), m_pScene->getMaxDensity())) /
        std::log(m_pScene->getMaxDensity()/m_pScene->getMinDensity()));
#endif // !FC_VALIDATION
}

bool BDPTRenderer::sceneIntersect(RTCScene esc, RTCRayHit& rhit, SurfacePoint& sp)
{
    float t1=rhit.ray.tfar;
    constexpr int maxItr=10; //To avoid infinite looping in case of errors in intersection computing
    int itr=maxItr;
    while (itr>0)
    {
        RTCIntersectArguments args;
        rtcInitIntersectArguments(&args);
        rtcIntersect1(esc,&rhit,&args);
        if (rhit.hit.geomID==RTC_INVALID_GEOMETRY_ID) return false;
        computeSurfacePoint(rhit,sp);
        RenderMaterial* mat=m_pScene->getMaterials()[sp.m_pMesh->m_iMatId];
        if (!mat->isMasked(&sp)) return true;
        //Material is masked, need to continue tracing
        fvec3 org={rhit.ray.org_x+rhit.ray.tfar*rhit.ray.dir_x,
                   rhit.ray.org_y+rhit.ray.tfar*rhit.ray.dir_y,
                   rhit.ray.org_z+rhit.ray.tfar*rhit.ray.dir_z};
        float v=maxAbsInVector(org);
        v=v>rhit.ray.tfar?v:rhit.ray.tfar;
        float delta=m_fEpsAcc*fEpsilon*v;
        rhit.ray.tnear=rhit.ray.tfar+delta;
        rhit.ray.tfar=t1;
        if (rhit.ray.tfar<=rhit.ray.tnear) return false;
        --itr;
    }
    return false;
}

float BDPTRenderer::computeLightAttenuation(float d, float attD)
{
    if (attD>=std::numeric_limits<float>::max()) //No specific attenuation distance
    {
        d=d<m_pParams->lightMinDistance?m_pParams->lightMinDistance:d;
        switch (m_pParams->lightDistanceAttenuation)
        {
            case 1: return 1.0f/d;
            case 2: return 1.0f/(d*d);
            default: return 1.0f;
        }
    }
    else
    {
        float rD=max(1.0f-d/attD,0.0f);
        switch (m_pParams->lightDistanceAttenuation)
        {
            case 1: return rD;
            case 2: return std::sqrt(rD);
            default: return d<attD?1.0f:0.0f;
        }
    }
}

bool BDPTRenderer::computePath(BDPTPathPart* P, int max_n, fvec3 O, fvec3 R, fvec3 Rad, int &n, bool inv, float attD)
{
    RTCScene esc=m_pScene->getEmbreeScene();
    RTCRayHit rhit {};
    rhit.ray.tnear=0.0f;
    rhit.ray.time=0.0f;
    rhit.ray.mask=std::numeric_limits<unsigned int>::max();
    rhit.ray.id=0;

    P[0].m_surfPoint.m_pInst=nullptr;
    assignVector3(P[0].m_surfPoint.m_position,O);
    assignVector3(P[0].m_outRay,R);
    assignVector3(P[0].m_outRad,Rad);
    assignVector(P[0].m_outFactor,1.0f,1.0f,1.0f);
    float rDterm=1.0f; //1/distance^2 term for radiance for the ray from light
    fvec3 brdf;
    fvec3 erad;
    float curInt;
    float ct;
    n=1;
    bool sceneExit=false;
    for (int i=1; i<max_n; ++i)
    {        
        setRay(P[i-1].m_surfPoint.m_position,P[i-1].m_outRay,rhit);
        if (!sceneIntersect(esc,rhit,P[i].m_surfPoint))
        {
            sceneExit=true;
            break;
        }
        RenderMaterial* mat=m_pScene->getMaterials()[P[i].m_surfPoint.m_pMesh->m_iMatId];
        mat->modifyFrame(&P[i].m_surfPoint);
        mat->getRayAndBrdf(P[i-1].m_outRay,&P[i].m_surfPoint,P[i].m_outRay,brdf,erad,inv);

        ct=std::abs(inv?dot3(P[i].m_outRay,P[i].m_surfPoint.m_normal):dot3(P[i-1].m_outRay,P[i].m_surfPoint.m_normal));
        assignVector(P[i].m_outFactor,P[i-1].m_outFactor[0]*brdf[0]*ct,P[i-1].m_outFactor[1]*brdf[1]*ct,P[i-1].m_outFactor[2]*brdf[2]*ct);
        if (inv) //Tracing from camera, adding emission directly to Rad
        {
#ifdef FC_VALIDATION
            if (m_pParams->falseColorMode) {
                assignVector3(Rad, brdf);
            }
            else {
#endif // !FC_VALIDATION
                Rad[0]+=P[i].m_outFactor[0]*erad[0];
                Rad[1]+=P[i].m_outFactor[1]*erad[1];
                Rad[2]+=P[i].m_outFactor[2]*erad[2];
                assignVector(P[i].m_outRad,0.0f,0.0f,0.0f);
#ifdef FC_VALIDATION
            }
#endif // !FC_VALIDATION
        }
        else //Tracing from light source, can cut path in case of low radiance
        {
            if (i==1) //First hit after light: considering light attenuation
                rDterm=computeLightAttenuation(distance3(P[i].m_surfPoint.m_position,P[i-1].m_surfPoint.m_position), attD);
            assignVector(P[i].m_outRad,P[i].m_outFactor[0]*Rad[0]*rDterm,
                                     P[i].m_outFactor[1]*Rad[1]*rDterm,
                                     P[i].m_outFactor[2]*Rad[2]*rDterm);
            curInt=intensity(P[i].m_outRad);
            if (curInt<m_pParams->rayCutPixValue) break;
        }
        n++;
    }
    return sceneExit;
}

RenderLight* BDPTRenderer::tryComputeLightPath(int& len, int tid)
{
    float pdf,rpdf; //Find a way to use source and ray pdfs
    RenderLight* L=m_pScene->sampleLight(pdf);
    if (!L) return nullptr;
    fvec3 O,R,radiance;
    L->getRandomRay(O,R,rpdf,radiance);
    radiance[0]*=m_pParams->lightScaleValue;
    radiance[1]*=m_pParams->lightScaleValue;
    radiance[2]*=m_pParams->lightScaleValue;
    computePath(m_ppLPBuf[tid],m_pParams->lightBouncesN+1,O,R,radiance,len,false,L->getAttenuationDistance());
    return L;
}

bool BDPTRenderer::isConnected(fvec3 A, fvec3 B)
{
    fvec3 dir;
    assignVector(dir,B[0]-A[0],B[1]-A[1],B[2]-A[2]);
    float len=length3(dir);
    float maxsz=maxAbsInVector(A);
    float safeD=2.0f*m_fEpsAcc*fEpsilon*maxsz;
    if (len<=safeD) return true;
    dir[0]/=len;
    dir[1]/=len;
    dir[2]/=len;

    RTCScene esc=m_pScene->getEmbreeScene();
    RTCRayHit rhit {};
    rhit.ray.tnear=safeD;
    rhit.ray.time=0.0f;
    rhit.ray.mask=std::numeric_limits<unsigned int>::max();
    rhit.ray.id=0;    
    setRay(A,dir,rhit,0.0f);
    rhit.ray.tfar=len+(0.1f*len>safeD?0.1f*len:safeD);
    SurfacePoint sp {};
    if (!sceneIntersect(esc,rhit,sp)) return true;
    float sd=squareDistance3(sp.m_position,B);
    maxsz=maxAbsInVector(B);
    safeD=10.0f*fEpsilon*maxsz;
    if (sd<=safeD*safeD) return true;
    return false;
}

void BDPTRenderer::computePaths(fvec3 O, fvec3 R, fvec3 radiance, RenderRay &camRay, int tid)
{
    RenderLight* lpsource=nullptr;
    int lp_len=0;
    if (m_pScene->lightsCount()>0)        
            lpsource=tryComputeLightPath(lp_len,tid);
    int cp_len=0;
    assignVector(radiance,0.0f,0.0f,0.0f);
    bool cp_exit=computePath(m_ppCPBuf[tid],m_pParams->cameraBouncesN+1,O,R,radiance,cp_len,true);
    assignVector(camRay.R(),-R[0],-R[1],-R[2]);
    if (cp_len==1) assignVector(camRay.O(),O[0]+R[0],O[1]+R[1],O[2]+R[2]);
    else assignVector3(camRay.O(),m_ppCPBuf[tid][1].m_surfPoint.m_position);

    int totalPN=0;
    if (cp_exit && m_pScene->getBackgound() && cp_len<=m_pParams->maxPathLength)
    {
        fvec3 bgRad;
        m_pScene->getBackgound()->getRadiance(m_ppCPBuf[tid][cp_len-1].m_outRay,bgRad);
        float* kf=m_ppCPBuf[tid][cp_len-1].m_outFactor;
        bgRad[0]*=kf[0]; bgRad[1]*=kf[1]; bgRad[2]*=kf[2];
        increaseRadiance(radiance,bgRad,1.0f); ++totalPN;
    }
    else if (intensity(radiance)>fEpsilon) ++totalPN; //Due to emissive surfaces on camera path
    if (lpsource) //Path to light was computed, producing set of paths, connecting light and camera
    {
        fvec3 LC_dir,lRad,lcRad,brdf,outV;
        float lpdf,c;
        float *lPos,*cPos;
        RenderMaterial* mat;
        for (int i=0; i<lp_len; ++i)
        {
            assignVector3(lRad,m_ppLPBuf[tid][i].m_outRad);
            lPos=m_ppLPBuf[tid][i].m_surfPoint.m_position;
            for (int j=1; j<cp_len && i+j<=m_pParams->maxPathLength; ++j)
            {
                cPos=m_ppCPBuf[tid][j].m_surfPoint.m_position;
                assignVector(LC_dir,cPos[0]-lPos[0],cPos[1]-lPos[1],cPos[2]-lPos[2]);
                normalize3(LC_dir);
                if (i==0)
                {
                    lpsource->getRadianceAlongRay(LC_dir,lpdf,lRad);
                    float lsc=computeLightAttenuation(distance3(cPos,lPos),lpsource->getAttenuationDistance());
                    lRad[0]*=m_pParams->lightScaleValue*lsc;
                    lRad[1]*=m_pParams->lightScaleValue*lsc;
                    lRad[2]*=m_pParams->lightScaleValue*lsc;
                }
                c=std::abs(dot3(LC_dir,m_ppCPBuf[tid][j].m_surfPoint.m_normal));
                assignVector(lcRad,lRad[0]*c*m_ppCPBuf[tid][j-1].m_outFactor[0],lRad[1]*c*m_ppCPBuf[tid][j-1].m_outFactor[1],lRad[2]*c*m_ppCPBuf[tid][j-1].m_outFactor[2]);
                if (intensity(lcRad)<m_pParams->rayCutPixValue) continue; //Max possible radiance through path is less than threshold

                mat=m_pScene->getMaterials()[m_ppCPBuf[tid][j].m_surfPoint.m_pMesh->m_iMatId];
                assignVector(outV,-m_ppCPBuf[tid][j-1].m_outRay[0],-m_ppCPBuf[tid][j-1].m_outRay[1],-m_ppCPBuf[tid][j-1].m_outRay[2]);
                mat->getBrdf(LC_dir,&m_ppCPBuf[tid][j].m_surfPoint,outV,brdf,false);
                brdf[0]*=c; brdf[1]*=c; brdf[2]*=c;
                lcRad[0]=brdf[0]*m_ppCPBuf[tid][j-1].m_outFactor[0]*lRad[0];
                lcRad[1]=brdf[1]*m_ppCPBuf[tid][j-1].m_outFactor[1]*lRad[1];
                lcRad[2]=brdf[2]*m_ppCPBuf[tid][j-1].m_outFactor[2]*lRad[2];
                if (intensity(lcRad)<m_pParams->rayCutPixValue) continue; //Radiance to camera is less than threshold
                if (!isConnected(lPos,cPos)) continue; //Point is not visible from the light source
                increaseRadiance(radiance,lcRad,1.0f);
            }
        }
    }
    float scale=totalPN?1.0f/static_cast<float>(totalPN):1.0f;
    for (int i=0; i<3; ++i) radiance[i]*=scale;
}

void BDPTRenderer::increaseRadiance(fvec3 rad, const fvec3 rad_inc, float kf)
{
    std::transform(rad_inc, rad_inc + 3, rad, rad, std::bind(Func3D(linearFunction), _1, _2, kf));
}

void BDPTRenderer::setRay(const fvec3 O, const fvec3 R, RTCRayHit& rh, float epsShift)
{
    rh.ray.org_x=O[0];
    rh.ray.org_y=O[1];
    rh.ray.org_z=O[2];
    rh.ray.dir_x=R[0];
    rh.ray.dir_y=R[1];
    rh.ray.dir_z=R[2];
    shiftRayAlongDirection(rh,m_fEpsAcc*epsShift); //To avoid self intersection
    rh.hit.instID[0]=RTC_INVALID_GEOMETRY_ID;
    rh.hit.geomID=RTC_INVALID_GEOMETRY_ID;
    rh.ray.tfar=std::numeric_limits<float>::max();
}

void BDPTRenderer::allocatePathBufs(int cpn, int lpn)
{
    int pnPerCore=cpn+lpn;
    m_pPathMem=new BDPTPathPart[pnPerCore*m_iCoresN];
    BDPTPathPart* ptr=m_pPathMem;
    m_ppCPBuf=new BDPTPathPart*[m_iCoresN];
    m_ppLPBuf=new BDPTPathPart*[m_iCoresN];    
    for (int i=0; i<m_iCoresN; ++i)
    {
        m_ppCPBuf[i]=ptr;
        ptr+=cpn;
        m_ppLPBuf[i]=ptr;
        ptr+=lpn;
    }
}

void BDPTRenderer::freePathBuffers()
{
    delete[] m_ppCPBuf;
    delete[] m_ppLPBuf;    
    delete[] m_pPathMem;
    m_ppCPBuf=nullptr;
    m_ppLPBuf=nullptr;    
    m_pPathMem=nullptr;
}

void BDPTRenderer::renderRow(RenderSensor* sensor, int tid)
{
    int row;
    int height=sensor->getHeight();
    int width=sensor->getWidth();
    RenderRay ray,returnRay;
    fvec3 rad;
    while ((row=m_aiCurRow.fetch_add(1))<height)
    {
        for (int j=0; j<width; ++j)
        {
            for (int k=0; k<m_pParams->samplesPerPixel; ++k)
            {
                ray=sensor->getRay(j,row,m_pScene->getSampler());
                returnRay.setIndex(ray.getIndex());
                computePaths(ray.O(),ray.R(),rad,returnRay,tid);
                sensor->hit(rad,returnRay,&ray);
            }
        }
        if (row%50==0) std::cout << row << " rows of " << height << std::endl;
    }
}

void BDPTRenderer::render(RenderSensor* sensor)
{
    if (!m_pParams || !m_pScene || !sensor) return;
    m_iCoresN = clamp(m_pParams->numberOfCores, 1, sensor->getHeight());
    
    allocatePathBufs(m_pParams->cameraBouncesN + 1, m_pParams->lightBouncesN + 1);

    m_aiCurRow = 0;
    sensor->init();

    std::vector<std::thread*> threads(static_cast<size_t>(m_iCoresN));
    int tid = 0;
    std::generate_n(threads.begin(), threads.size(), [this, &sensor, &tid] {
        return new std::thread(&BDPTRenderer::renderRow, this, sensor, tid++);
    });

    std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));
    deleteContainerWithPointers(threads);

    sensor->stop();
    freePathBuffers();
}

BDPTRenderer::~BDPTRenderer()
{
    freePathBuffers();
}

