//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      RenderMesh.cpp
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Rendering scene mesh object

//#define FC_VALIDATION

#include <iostream>
#include <map>
#include <numeric>
#include <set>

#include "RenderMesh.h"
#include "embree4/rtcore_scene.h"

#define MIN_AREA_MARGIN 0*1e-3f

#define FC_SG_POINTWISE 0   // gradient is a measure of interpolated normals' deviation from the flat normal
#define FC_SG_FLATTENED 1   // gradient is an average angle difference between normals over adjacent edges
#define FC_SG_PROJECTED 2   // gradient is a measure of face alignment with respect to a give direction, i.e. w.r.t. FC_SG_PROJECTED_TO_DIR
#define FC_SG_PROJECTED_TO_DIR  1.0f, 0.0f, 0.0f    // X-axis by default

// Configure surface gradient measure
#ifdef FC_VALIDATION
#   define FC_SG_TYPE FC_SG_POINTWISE
#else
#   define FC_SG_TYPE -1
#endif

namespace mesh {

    constexpr float densityFunction(float area) noexcept { return 1.0f / (MIN_AREA_MARGIN + area); }

#if FC_SG_TYPE == FC_SG_FLATTENED
    void gradientWeightFunctions(float u, float v, fvec3 result) noexcept { assignVector(result, 1.0f/3.0f - v, u + v - 2.0f/3.0f, 1.0f/3.0f - u); }
#endif
} // !mesh

RenderMesh::RenderMesh()
{    
    reset();    
}

void RenderMesh::allocate(int index, int vertN, int facesN, int texChannelsN, int matIndex)
{
    free();
    m_iId=index;
    m_iVN=vertN;
    m_iFN=facesN;
    m_iTexChN=texChannelsN<m_iMaxTexChannels?texChannelsN:m_iMaxTexChannels;
    m_iStride=10+m_iTexChN*2;
    m_pF=new uivec3[m_iFN];
    m_pfV=new float[m_iVN*m_iStride];
    std::memset(m_pF,0,sizeof(uivec3)*static_cast<size_t>(m_iFN));
    std::memset(m_pfV,0,sizeof(float)*static_cast<size_t>(m_iVN*m_iStride));
#if FC_SG_TYPE == FC_SG_FLATTENED
    m_pA=new ivec3[m_iFN];
    std::for_each(m_pA, m_pA + m_iFN, std::bind(&std::fill_n<int *, int, int>, _1, 3, -1));
#endif

    m_iVertexOffs=0;
    m_iNormalOffs=3;
    m_iTangentOffs=6;
    for (int i=0; i<m_iTexChN; ++i) m_iTexOffs[i]=10+i*2;

    m_iMatId=matIndex;
}

void RenderMesh::setVertex(int vid, fvec3 v)
{
    if (vid<0 || vid>=m_iVN) return;
    m_bVertexDef=true;
    assignVector3(m_pfV+(m_iStride*vid+m_iVertexOffs),v);
}

void RenderMesh::setNormal(int nid, fvec3 n)
{
    if (nid<0 || nid>=m_iVN) return;
    m_vNormalDef=true;
    assignVector3(m_pfV+(m_iStride*nid+m_iNormalOffs),n);
}

void RenderMesh::setTangent(int tid, fvec4 t)
{
    if (tid<0 || tid>=m_iVN) return;
    m_bTangentDef=true;
    assignVector4(m_pfV+(m_iStride*tid+m_iTangentOffs),t);
}

void RenderMesh::setTextureCoordinate(int chid, int tcid, fvec2 tc)
{
    if (tcid<0 || tcid>=m_iVN || chid>=m_iMaxTexChannels) return;
    if (m_iTexMap[chid]==-1) m_iTexMap[chid]=m_iFilledTexChN++;
    chid=m_iTexMap[chid];
    m_bTexDef[chid]=true;
    assignVector2(m_pfV+(m_iStride*tcid+m_iTexOffs[chid]),tc);
}

void RenderMesh::setFace(int fid, uivec3 f)
{
    assignVector3(m_pF[fid],f);
}

void RenderMesh::setVertices(const float* v, int vstride)
{
    m_bVertexDef=true;
    float* thisV=m_pfV+m_iVertexOffs;
    for (int i=0; i<m_iVN; thisV+=m_iStride, v+=vstride, ++i) assignVector3(thisV,v);
}

void RenderMesh::setNormals(const float* n, int nstride)
{
    m_vNormalDef=true;
    float* thisN=m_pfV+m_iNormalOffs;
    for (int i=0; i<m_iVN; thisN+=m_iStride, n+=nstride, ++i) assignVector3(thisN,n);
}

void RenderMesh::setTangents(const float* t, int tstride)
{
    m_bTangentDef=true;
    float* thisT=m_pfV+m_iTangentOffs;
    for (int i=0; i<m_iVN; thisT+=m_iStride, t+=tstride, ++i) assignVector4(thisT,t);
}

void RenderMesh::setTextureCoordinates(int chid, const float* tc, int tcstride)
{
    if (chid>=m_iMaxTexChannels) return;
    if (m_iTexMap[chid]==-1) m_iTexMap[chid]=m_iFilledTexChN++;
    chid=m_iTexMap[chid];
    m_bTexDef[chid]=true;
    float* thisTC=m_pfV+m_iTexOffs[chid];
    for (int i=0; i<m_iVN; thisTC+=m_iStride, tc+=tcstride, ++i) assignVector2(thisTC,tc);
}

void RenderMesh::setFaces(unsigned int* f)
{
    std::memcpy(m_pF,f,sizeof(uivec3)*static_cast<size_t>(m_iFN));
}

bool RenderMesh::getTexCoord(const fvec3 bc, int faceId, fvec3 tc, int channel)
{
    tc[0]=0.0f;
    tc[1]=0.0f;
    if (faceId<0 || faceId>=m_iFN || channel<0 || channel>=m_iMaxTexChannels || m_iTexMap[channel]<0) return false;
    unsigned int* fi=m_pF[faceId];
    float* uvptr=m_pfV+m_iTexOffs[m_iTexMap[channel]];
    auto s=static_cast<unsigned int>(m_iStride);
    float* uv[3]={uvptr+fi[0]*s,uvptr+fi[1]*s,uvptr+fi[2]*s};
    tc[0]=uv[0][0]*bc[0]+uv[1][0]*bc[1]+uv[2][0]*bc[2];
    tc[1]=uv[0][1]*bc[0]+uv[1][1]*bc[1]+uv[2][1]*bc[2];
    return true;
}

void RenderMesh::computeSurfacePoint(SurfacePoint& sp, const fmat4 TM, const fmat3 normM)
{
    sp.m_bIsTexNorm=false;
    float* b=sp.m_bary;

    unsigned int* fi=m_pF[sp.m_iFaceId];
    const float* v[3]={m_pfV+static_cast<int>(fi[0])*m_iStride,m_pfV+static_cast<int>(fi[1])*m_iStride,m_pfV+static_cast<int>(fi[2])*m_iStride};

    fvec3 pp,pn,pbn;
    fvec4 pt;
    for (int i=0; i<3; ++i)
    {
        pp[i]=b[0]*v[0][m_iVertexOffs+i]+b[1]*v[1][m_iVertexOffs+i]+b[2]*v[2][m_iVertexOffs+i];
        pn[i]=b[0]*v[0][m_iNormalOffs+i]+b[1]*v[1][m_iNormalOffs+i]+b[2]*v[2][m_iNormalOffs+i];
        pt[i]=b[0]*v[0][m_iTangentOffs+i]+b[1]*v[1][m_iTangentOffs+i]+b[2]*v[2][m_iTangentOffs+i];
    }
    pt[3]=b[0]*v[0][m_iTangentOffs+3]+b[1]*v[1][m_iTangentOffs+3]+b[2]*v[2][m_iTangentOffs+3];
    pt[3]=pt[3]>=0.0f?1.0f:-1.0f;
    normalizeIfNeeded3(pn);
    float tn=dot3(pn,pt);
    pt[0]-=tn*pn[0];
    pt[1]-=tn*pn[1];
    pt[2]-=tn*pn[2];
    normalizeIfNeeded3(pt);
    cross3(pn,pt,pbn);
    pbn[0]*=pt[3];
    pbn[1]*=pt[3];
    pbn[2]*=pt[3];
    multVector3(TM,pp,sp.m_position);
    multVector3(normM,pn,sp.m_normal);
    multVector3(normM,pt,sp.m_tangent);
    multVector3(normM,pbn,sp.m_binormal);
    //If normM contains scale normalization must be performed
    normalizeIfNeeded3(sp.m_normal);
    normalizeIfNeeded3(sp.m_tangent);
    normalizeIfNeeded3(sp.m_binormal);
    sp.m_tangent[3]=pt[3];

#ifdef FC_VALIDATION
    fvec3 V0, V1, V2, tfn;
    multVector3(TM, v[0] + m_iVertexOffs, V0);
    multVector3(TM, v[1] + m_iVertexOffs, V1);
    multVector3(TM, v[2] + m_iVertexOffs, V2);
    triangleNormal(V0, V1, V2, tfn);
    sp.m_fRefDensity = mesh::densityFunction(length3(tfn) / 2.0f);
    normalize3(tfn);

#   if FC_SG_TYPE == FC_SG_POINTWISE
    sp.m_fAvgGradient = std::pow(clamp(std::accumulate(v, v + 3, 0.0f, [this, tfn, normM](float ag, const float *vd) {
        fvec3 tvn;
        multVector3(normM, vd + m_iNormalOffs, tvn);
        normalizeIfNeeded3(tvn);
        return ag + std::sqrt((1.0f - clamp(dot3(tvn, tfn), -1.0f, 1.0f)) / 2.0f);
    }) / 3.0f, 1e-6f, 1.0f), 0.286f);
#   elif FC_SG_TYPE == FC_SG_FLATTENED
    fvec3 weight, angles;
    mesh::gradientWeightFunctions(b[1], b[2], weight);
    std::transform(m_pA[sp.m_iFaceId], m_pA[sp.m_iFaceId] + 3, angles, [this, normM, tfn](int ni) {
        if (ni != -1) {
            unsigned int *n = m_pF[ni];
            const float* nv[3]={m_pfV+static_cast<int>(n[0])*m_iStride,m_pfV+static_cast<int>(n[1])*m_iStride,m_pfV+static_cast<int>(n[2])*m_iStride};
            fvec3 fnn, tfnn;
            triangleNormal(nv[0], nv[1], nv[2], fnn);
            multVector3(normM, fnn, tfnn);
            normalize3(tfnn);
            return std::sqrt(lerp(-clamp(dot3(tfn, tfnn), -1.0f, 1.0f), 1.0f, 0.5f));
        }
        else return 1.0f;
    });
    sp.m_fAvgGradient = 1.0f - lerp(2.25f * std::pow(dot3(weight, angles), 2.0f), std::sqrt(lerp(-clamp(dot3(tfn, sp.m_normal), -1.0f, 1.0f), 1.0f, 0.5f)), 0.0f);
#   elif FC_SG_TYPE == FC_SG_PROJECTED
    const fvec3 pdir = {FC_SG_PROJECTED_TO_DIR};
    sp.m_fAvgGradient = std::sqrt((1.0f - clamp(dot3(tfn, pdir), -1.0f, 1.0f)) / 2.0f);
#   endif
#endif // !FC_VALIDATION
}

void RenderMesh::computeSurfacePoint(int fid, fvec3 bc, const fmat4 TM, fvec3 pos) const
{
    unsigned int* fi=m_pF[fid];
    float* v[3]={m_pfV+static_cast<int>(fi[0])*m_iStride,m_pfV+static_cast<int>(fi[1])*m_iStride,m_pfV+static_cast<int>(fi[2])*m_iStride};
    fvec3 pp;
    for (int i=0; i<3; ++i)
        pp[i]=bc[0]*v[0][m_iVertexOffs+i]+bc[1]*v[1][m_iVertexOffs+i]+bc[2]*v[2][m_iVertexOffs+i];
    multVector3(TM,pp,pos);
}

bool RenderMesh::commitMesh(int normalTexCh)
{
    if (!isValid())
    {
        free();
        return false;
    }
    if (!m_vNormalDef) computeNormals();    
    if (!m_bTangentDef)
    {
        int ch=(normalTexCh>=0 && m_iTexMap[normalTexCh]>=0)?m_iTexMap[normalTexCh]:-1;
        if (ch>=0 && m_bTexDef[ch]) computeTangents(ch);
        else computeTangents();        
    }

#if FC_SG_TYPE == FC_SG_FLATTENED
    computeNeighbors();
#endif //
    return true;
}

bool RenderMesh::isValid() const noexcept
{
    return m_iVN>0 && m_iFN>0 && m_pfV && m_pF && m_bVertexDef
#if FC_SG_TYPE == FC_SG_FLATTENED
    && m_pA
#endif
    ;
}

#ifdef FC_VALIDATION
std::pair<float, float> RenderMesh::getMinMaxDensity(fmat4 TM) const
{
    float minDensity = fInfinity;
    float maxDensity = 0.0f;
    fvec3 N;
    for (int i=0; i<m_iFN; ++i)
    {
        fvec3 V0, V1, V2;
        multVector3(TM, m_pfV+static_cast<int>(m_pF[i][0])*m_iStride+m_iVertexOffs, V0);
        multVector3(TM, m_pfV+static_cast<int>(m_pF[i][1])*m_iStride+m_iVertexOffs, V1);
        multVector3(TM, m_pfV+static_cast<int>(m_pF[i][2])*m_iStride+m_iVertexOffs, V2);
        triangleNormal(V0, V1, V2, N);
        float area = length3(N) / 2.0f;
        if (area > fEpsilon) {
            float density = mesh::densityFunction(area);
            minDensity = std::min(minDensity, density);
            maxDensity = std::max(maxDensity, density);
        }
    }
    return {minDensity, maxDensity};
}
#endif // !FC_VALIDATION

void RenderMesh::computeNormals()
{
    float* cn=m_pfV+m_iNormalOffs;
    for (int i=0; i<m_iVN; cn+=m_iStride, ++i)
    {
        assignVector(cn, 0.0f, 0.0f, fEpsilon);
    }
    float *p0,*p1,*p2;
    fvec3 v0,v1,N;
    for (int i=0; i<m_iFN; ++i)
    {
        p0=m_pfV+static_cast<int>(m_pF[i][0])*m_iStride+m_iVertexOffs;
        p1=m_pfV+static_cast<int>(m_pF[i][1])*m_iStride+m_iVertexOffs;
        p2=m_pfV+static_cast<int>(m_pF[i][2])*m_iStride+m_iVertexOffs;
        assignVector(v0,p1[0]-p0[0],p1[1]-p0[1],p1[2]-p0[2]);
        assignVector(v1,p2[0]-p1[0],p2[1]-p1[1],p2[2]-p1[2]);
        cross3(v0,v1,N);
        if (N[0]*N[0]+N[1]*N[1]+N[2]*N[2]>fEpsilon) normalize3(N);

        for (int j=0; j<3; ++j)
        {
            cn=m_pfV+static_cast<int>(m_pF[i][j])*m_iStride+m_iNormalOffs;
            cn[0]+=N[0];
            cn[1]+=N[1];
            cn[2]+=N[2];
        }
    }
    cn=m_pfV+m_iNormalOffs;
    for (int i=0; i<m_iVN; cn+=m_iStride, ++i) normalize3(cn);
    m_vNormalDef=true;
}

void RenderMesh::computeTangents(int tid)
{
    auto tv=new fvec3[m_iVN];
    auto bv=new fvec3[m_iVN];
    std::memset(tv,0,sizeof(fvec3)*static_cast<size_t>(m_iVN));
    std::memset(bv,0,sizeof(fvec3)*static_cast<size_t>(m_iVN));
    float *X0,*X1,*X2,*U0,*U1,*U2,r;
    int k=m_iTexOffs[tid];
    fvec3 v0,v1;
    fvec2 t0,t1;
    fmat2 SI;
    fvec3 udir,vdir;
    for (int i=0; i<m_iFN; ++i)
    {
        X0=m_pfV+static_cast<int>(m_pF[i][0])*m_iStride+m_iVertexOffs;
        X1=m_pfV+static_cast<int>(m_pF[i][1])*m_iStride+m_iVertexOffs;
        X2=m_pfV+static_cast<int>(m_pF[i][2])*m_iStride+m_iVertexOffs;
        U0=m_pfV+static_cast<int>(m_pF[i][0])*m_iStride+k;
        U1=m_pfV+static_cast<int>(m_pF[i][1])*m_iStride+k;
        U2=m_pfV+static_cast<int>(m_pF[i][2])*m_iStride+k;

        assignVector(v0,X1[0]-X0[0],X1[1]-X0[1],X1[2]-X0[2]);
        assignVector(v1,X2[0]-X0[0],X2[1]-X0[1],X2[2]-X0[2]);
        assignVector(t0,U1[0]-U0[0],U1[1]-U0[1]);
        assignVector(t1,U2[0]-U0[0],U2[1]-U0[1]);

        float det=t0[0]*t1[1]-t1[0]*t0[1];
        if (det<fEpsilon) continue;
        r=1.0f/det;
        SI[0][0]=r*t1[1]; SI[0][1]=-r*t1[0];
        SI[1][0]=-r*t0[1]; SI[1][1]=r*t0[0];
        assignVector(udir,v0[0]*SI[0][0]+v1[0]*SI[1][0],
                          v0[1]*SI[0][0]+v1[1]*SI[1][0],
                          v0[2]*SI[0][0]+v1[2]*SI[1][0]);
        assignVector(vdir,v0[0]*SI[0][1]+v1[0]*SI[1][1],
                          v0[1]*SI[0][1]+v1[1]*SI[1][1],
                          v0[2]*SI[0][1]+v1[2]*SI[1][1]);
        for (int j=0; j<3; ++j)
        {
            tv[m_pF[i][0]][j]+=udir[j];
            tv[m_pF[i][1]][j]+=udir[j];
            tv[m_pF[i][2]][j]+=udir[j];
            bv[m_pF[i][0]][j]+=vdir[j];
            bv[m_pF[i][1]][j]+=vdir[j];
            bv[m_pF[i][2]][j]+=vdir[j];
        }
    }
    float* tan;
    float* nv;
    float nv_tv;
    fvec3 nvtv;
    float len;
    for (int i=0; i<m_iVN; ++i)
    {
        tan=m_pfV+i*m_iStride+m_iTangentOffs;
        nv=m_pfV+i*m_iStride+m_iNormalOffs;
        nv_tv=dot3(nv,tv[i]);
        tan[0]=tv[i][0]-nv[0]*nv_tv;
        tan[1]=tv[i][1]-nv[1]*nv_tv;
        tan[2]=tv[i][2]-nv[2]*nv_tv;
        len=length3(tan);
        if (len<=fEpsilon)
        {
            randomTangent(nv,tan);
        }
        else
        {
            tan[0]/=len;
            tan[1]/=len;
            tan[2]/=len;
        }
        cross3(nv,tan,nvtv);
        tan[3]=dot3(nvtv,bv[i])<0.0f?-1.0f:1.0f;
    }
    delete[] bv;
    delete[] tv;
    m_bTangentDef=true;
}

void RenderMesh::computeTangents()
{    
    for (int i=0; i<m_iVN; ++i)    
        randomTangent(m_pfV+i*m_iStride+m_iNormalOffs,m_pfV+i*m_iStride+m_iTangentOffs);
    m_bTangentDef=true;
}

void RenderMesh::randomTangent(const fvec3 nv, fvec3 tv, float scale)
{
    fvec3 Zv={0.0f,0.0f,1.0f};
    fvec3 Yv={0.0f,1.0f,0.0f};
    fvec3 Xv={1.0f,0.0f,0.0f};
    if (nv[2]<0.95f && nv[2]>-0.95f) cross3(Zv,nv,tv); //Use z vector to align all tangent vectors
    else if (nv[1]<0.95f && nv[1]>-0.95f) cross3(Yv,nv,tv); //Use y vector to align all tangent vectors
        else cross3(Xv,nv,tv); //Use x vector to align all tangent vectors
    normalize3(tv);
    tv[3]=1.0f;
    tv[0]*=scale;
    tv[1]*=scale;
    tv[2]*=scale;
    m_bTangentDef=true;
}

void RenderMesh::computeNeighbors()
{
#if FC_SG_TYPE == FC_SG_FLATTENED
    std::map<int, std::set<int>> vftable;
    for (int i=0; i<m_iFN; ++i)
        for (int v=0; v<3; ++v)
            vftable[m_pF[i][v]].insert(i);

    for (int i=0; i<m_iFN; ++i)
    {
        for (int v=0; v<3; ++v)
        {
            for (int j : vftable[m_pF[i][v]])
            {
                if (i==j) continue;
                uivec3 fi, fj;
                assignVector3(fi, m_pF[i]);
                assignVector3(fj, m_pF[j]);
                std::sort(fi, fi + 3);
                std::sort(fj, fj + 3);
                uivec2 is = {-1U, -1U};
                std::set_intersection(fi, fi + 3, fj, fj + 3, is);
                if (is[0] != -1U && is[1] != -1U) {
                    if (is[0] == m_pF[i][0]) {
                        if (is[1] == m_pF[i][1]) m_pA[i][0] = j;
                        else if (is[1] == m_pF[i][2]) m_pA[i][2] = j;
                    }
                    else if (is[0] == m_pF[i][1]) {
                        if (is[1] == m_pF[i][0]) m_pA[i][0] = j;
                        else if (is[1] == m_pF[i][2]) m_pA[i][1] = j;
                    }
                    else if (is[0] == m_pF[i][2]) {
                        if (is[1] == m_pF[i][0]) m_pA[i][2] = j;
                        else if (is[1] == m_pF[i][1]) m_pA[i][1] = j;
                    }
                }
            }
        }
    }
#endif
}

int RenderMesh::testTangent() const noexcept
{
    float* tg;
    int n=0;
    for (int i=0; i<m_iVN; ++i)
    {
        tg=m_pfV+i*m_iStride+m_iTangentOffs;
        if (std::any_of(tg, tg + 3, static_cast<bool(*)(float)>(std::isnan))) ++n;
    }
    return n;
}

void RenderMesh::reset()
{
    m_iId=-1;
    m_iVN=0;
    m_iFN=0;
    m_pF=nullptr;
    m_pfV=nullptr;
#ifdef FC_VALIDATION
    m_pA=nullptr;
#endif // !FC_VALIDATION
    m_iStride=0;

    m_iVertexOffs=0;
    m_iNormalOffs=0;
    m_iTangentOffs=0;
    for (int i=0; i<m_iMaxTexChannels; ++i)
    {
        m_iTexOffs[i]=0;
        m_iTexMap[i]=-1;
        m_bTexDef[i]=false;
    }
    m_iTexChN=0;
    m_iFilledTexChN=0;
    m_bVertexDef=false;
    m_vNormalDef=false;
    m_bTangentDef=false;
    m_iMatId=-1;    
    m_embreeScene=nullptr;
}

void RenderMesh::free()
{
    if (m_embreeScene) rtcReleaseScene(m_embreeScene);
    delete[] m_pF;
    delete[] m_pfV;
#ifdef FC_VALIDATION
    delete[] m_pA;
#endif // !FC_VALIDATION
    reset();
}

RenderMesh::~RenderMesh()
{
    free();
}
