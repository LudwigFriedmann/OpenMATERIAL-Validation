//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      HDRBackground.cpp
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-17
/// @brief     HDR background representation

#define _USE_MATH_DEFINES

#include "HdrBackground.h"
#include "doctest.h"
#include <numeric>

#define UNIFORM_HDR 0

/// @brief Unit test for HDRBackground::HDRBackground 
TEST_CASE("Testing HDRBackground::HDRBackground")
{
	std::string filename = "../hdr/green_point_park_4k.hdr";
	#ifdef _WIN32
		std::replace(filename.begin(), filename.end(), '/', '\\');
	#endif
	
    bool pass = true;
	// Check for errors thrown if hdr isn't loaded
    try {
        HDRBackground background(filename.data()); 
    }
	catch (...) {
        pass = false;
    }
	CHECK(pass);
}

HDRBackground::HDRBackground(const char *rsHdr)
{
    m_fRadianceScale = 1.0f;
    int n;
    m_pImage = reinterpret_cast<fvec3*>(stbi_loadf(rsHdr, &m_iWidth, &m_iHeight, &n, 0));
    if (!m_pImage) {
        m_pImage = new fvec3[1];
        assignVector3(m_pImage[0], m_emitRadiance);
        m_iWidth = m_iHeight = 1;
    }
    computeAverage();

#if UNIFORM_HDR
    std::for_each(m_pImage, m_pImage + m_iWidth*m_iHeight, std::bind(&std::fill_n<float *, int, float>, _1, 3, 0.5f));
#endif
}

void HDRBackground::getRadiance(fvec3 dir, fvec3 val)
{
    fvec3 rdir;
    multVector3(dir,m_rotation,rdir); //Applying inverse rotation of background to the ray
    fvec3 r_xy={rdir[0],rdir[1],0.0f};
    normalize3(r_xy);
    float xang=std::acos(clamp(r_xy[0],-1.0f,1.0f));
    xang=r_xy[1]<0.0f?2.0f*M_PIf-xang:xang;
    float yang=std::asin(clamp(rdir[2],-1.0f,1.0f));
    yang+=0.5f*M_PIf;    
    float u=xang/(2.0f*M_PIf);
    float v=1.0f-yang/M_PIf;
    ivec4 id;
    fvec4 w;
    float upage=std::floor(u-0.5f/static_cast<float>(m_iWidth));
    float vpage=std::floor(v-0.5f/static_cast<float>(m_iHeight));
    u-=upage;
    v-=vpage;
    u*=static_cast<float>(m_iWidth);
    v*=static_cast<float>(m_iHeight);
    int iu=static_cast<int>(u);
    int iv=static_cast<int>(v);
    float kui=u-static_cast<float>(iu);
    float kvi=v-static_cast<float>(iv);
    float ku=1.0f-kui;
    float kv=1.0f-kvi;
    int l=iu%m_iWidth;
    int r=(iu+1)%m_iWidth;
    int b=iv%m_iHeight;
    int t=(iv+1)%m_iHeight;
    assignVector(id,b*m_iWidth+l,b*m_iWidth+r,t*m_iWidth+l,t*m_iWidth+r);
    assignVector(w,ku*kv,kui*kv,ku*kvi,kui*kvi);

    for (int i=0; i<3; ++i)
        val[i]=static_cast<float>(m_pImage[id[0]][i])*w[0]+
               static_cast<float>(m_pImage[id[1]][i])*w[1]+
               static_cast<float>(m_pImage[id[2]][i])*w[2]+
               static_cast<float>(m_pImage[id[3]][i])*w[3];

    val[0]*=m_fRadianceScale;
    val[1]*=m_fRadianceScale;
    val[2]*=m_fRadianceScale;
}

void HDRBackground::computeAverage()
{
    assignVector(m_avgRadiance,0.0f,0.0f,0.0f);
    int n=m_iHeight*m_iWidth;
    for (int i=0; i<n; ++i)
    {
        for (int j=0; j<3; ++j)
            m_avgRadiance[j]+=m_pImage[i][j];
    }
    float scaler=1.0f/static_cast<float>(n);
    m_avgRadiance[0]*=scaler;
    m_avgRadiance[1]*=scaler;
    m_avgRadiance[2]*=scaler;
}

HDRBackground::~HDRBackground()
{
    free(m_pImage);
}
