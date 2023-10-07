//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      BitmapTexture.cpp
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Structural representation of the texture bitmap

#include "BitmapTexture.h"

BitmapTexture::BitmapTexture() : m_pImage(nullptr), m_iWidth(0), m_iHeight(0) {}

BitmapTexture::BitmapTexture(int w, int h, ubvec4* img, bool copy) : BitmapTexture()
{
    set(w, h, img, copy);
}

BitmapTexture::~BitmapTexture()
{
    delete[] m_pImage;
}

void BitmapTexture::set(int w, int h, ubvec4* img, bool copy)
{
    delete[] m_pImage;
    m_iWidth = w;
    m_iHeight = h;

    if (copy) {
        m_pImage = new ubvec4[m_iWidth*m_iHeight];
        std::memcpy(m_pImage,img,sizeof(ubvec4)*static_cast<size_t>(m_iWidth*m_iHeight));
    }
    else {
        m_pImage = img;
    }
}

void BitmapTexture::texelFetch(int x, int y, ubvec4 tex) const
{
    if (m_pImage && x>=0 && x<m_iWidth && y>=0 && y<m_iHeight) {
        assignVector4(tex, m_pImage[y*m_iWidth+x]);
    }
    else {
        tex[0]=128;
        tex[1]=128;
        tex[2]=128;
        tex[3]=255;
    }
}

void BitmapTexture::texelFetch(int x, int y, fvec4 tex) const
{
    if (m_pImage && x>=0 && x<m_iWidth && y>=0 && y<m_iHeight)
    {
        int id=y*m_iWidth+x;
        constexpr float s=1.0f/255.0f;
        tex[0]=static_cast<float>(m_pImage[id][0])*s;
        tex[1]=static_cast<float>(m_pImage[id][1])*s;
        tex[2]=static_cast<float>(m_pImage[id][2])*s;
        tex[3]=static_cast<float>(m_pImage[id][3])*s;
    }
    else
    {
        tex[0]=0.5f;
        tex[1]=0.5f;
        tex[2]=0.5f;
        tex[3]=1.0f;
    }
}

void BitmapTexture::getPixCoords(float u, float v, ivec4 id, fvec4 w) const
{
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
}

void BitmapTexture::texture(float u, float v, ubvec4 tex) const
{
    fvec4 tc;
    texture(u,v,tc);
    tex[0]=static_cast<unsigned char>(tc[0]*255.0f);
    tex[1]=static_cast<unsigned char>(tc[1]*255.0f);
    tex[2]=static_cast<unsigned char>(tc[2]*255.0f);
    tex[3]=static_cast<unsigned char>(tc[3]*255.0f);
}

void BitmapTexture::texture(float u, float v, fvec4 tex) const
{        
    if (!m_pImage)
    {
        tex[0]=0.5f;
        tex[1]=0.5f;
        tex[2]=0.5f;
        tex[3]=1.0f;
    }
    else
    {
        ivec4 id;
        fvec4 w;
        constexpr float s=1.0f/255.0f;
        getPixCoords(u,v,id,w);
        for (int i=0; i<4; ++i)
            tex[i]=static_cast<float>(m_pImage[id[0]][i])*w[0]*s+
                   static_cast<float>(m_pImage[id[1]][i])*w[1]*s+
                   static_cast<float>(m_pImage[id[2]][i])*w[2]*s+
                   static_cast<float>(m_pImage[id[3]][i])*w[3]*s;
    }
}

void BitmapTexture::texture(const SurfacePoint* sp, int chId, ubvec4 tex) const
{
    fvec2 uv;
    if (!sp->getTexCoords(chId,uv))
    {
        tex[0]=128;
        tex[1]=128;
        tex[2]=128;
        tex[3]=255;
    }
    else
    {
        texture(uv[0],uv[1],tex);
    }
}

void BitmapTexture::texture(const SurfacePoint* sp, int chId, fvec4 tex) const
{
    fvec2 uv;
    if (!sp->getTexCoords(chId,uv))
    {
        tex[0]=0.5f;
        tex[1]=0.5f;
        tex[2]=0.5f;
        tex[3]=1.0f;
    }
    else
    {
        texture(uv[0],uv[1],tex);
    }
}
