//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      ImageData.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Support for glTF image reading

#include "ImageData.h"

ImageData::ImageData() noexcept :
    m_iId(0), m_pImage(nullptr), m_iWidth(0), m_iHeight(0)
{}

ImageData::ImageData(const ImageData &d) noexcept :
    m_iId(d.m_iId), m_pImage(nullptr), m_iWidth(d.m_iWidth), m_iHeight(d.m_iHeight)
{
    if (m_iWidth*m_iHeight>0)
    {
        m_pImage=new ubvec4[m_iWidth*m_iHeight];
        std::memcpy(m_pImage,d.m_pImage,sizeof(ubvec4)*static_cast<size_t>(m_iWidth*m_iHeight));
    }
}

ImageData::ImageData(ImageData &&d) noexcept :
    m_iId(d.m_iId), m_pImage(d.m_pImage), m_iWidth(d.m_iWidth), m_iHeight(d.m_iHeight)
{
    d.m_pImage = nullptr;
    d.m_iWidth = d.m_iHeight = 0;
}

void ImageData::load(const tinygltf::Image &gimg, int imgId)
{
    free();
    m_iId=imgId;
    m_iWidth=gimg.width;
    m_iHeight=gimg.height;
    if (m_iWidth*m_iHeight<=0) return;
    m_pImage=new ubvec4[m_iWidth*m_iHeight];
    ubvec4 def={0,0,0,255};
    for (int i=0; i<m_iWidth*m_iHeight; ++i) assignVector4(m_pImage[i],def);
    int cn=gimg.component;
    if (!cn) return;
    if (cn>4) cn=4;
    const unsigned char* ptr=gimg.image.data();
    switch(gimg.pixel_type)
    {
    case (TINYGLTF_COMPONENT_TYPE_BYTE):
        iterateData(reinterpret_cast<const char*>(ptr),cn,[](char v)
        {
            return static_cast<unsigned char>(static_cast<int>(v)+128);
        });
        break;
    case (TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE):
        iterateData(ptr,cn,[]( unsigned char v)
        {
            return v;
        });
        break;
    case (TINYGLTF_COMPONENT_TYPE_SHORT):
        iterateData(reinterpret_cast<const short*>(ptr),cn,[](short v)
        {
            return static_cast<unsigned char>((static_cast<int>(v)+32768)/256);
        });
        break;
    case (TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT):
        iterateData(reinterpret_cast<const unsigned short*>(ptr),cn,[](unsigned short v)
        {
            return static_cast<unsigned char>(v/256);
        });
        break;
    case (TINYGLTF_COMPONENT_TYPE_INT):
        iterateData(reinterpret_cast<const int32_t*>(ptr),cn,[](int32_t v)
        {
            return static_cast<unsigned char>(v/16777216+128);
        });
        break;
    case (TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT):
        iterateData(reinterpret_cast<const uint32_t*>(ptr),cn,[](uint32_t v)
        {
            return static_cast<unsigned char>(v/16777216);
        });
        break;
    case (TINYGLTF_COMPONENT_TYPE_FLOAT):
        iterateData(reinterpret_cast<const float*>(ptr),cn,[](float v)
        {
            return static_cast<unsigned char>(clamp(v*256.0f,0.0f,255.0f));
        });
        break;
    case (TINYGLTF_COMPONENT_TYPE_DOUBLE):
        iterateData(reinterpret_cast<const double*>(ptr),cn,[](double v)
        {
            return static_cast<unsigned char>(clamp(v*256.0,0.0,255.0));
        });
        break;
    default:
        return;
    }
}

void ImageData::free()
{
    delete[] m_pImage;
    m_pImage=nullptr;
    m_iWidth=0;
    m_iHeight=0;
    m_iId=0;
}

ImageData::~ImageData()
{
    delete[] m_pImage;
}
