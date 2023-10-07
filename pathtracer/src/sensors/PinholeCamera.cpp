//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      PinholeCamera.cpp
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Pinhole camera implementation

#include <fstream>
#include <iostream>

#include "json.hpp"
#include "doctest.h"

#include "PinholeCamera.h"

namespace {

    template<typename T, typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
    bool rangeCheck(T index, T begin, T end) noexcept { return begin <= index && index < end; }
} // !anonymous namespace


/// @brief Unit test for PinholeCamera::loadProperties
TEST_CASE("Testing PinholeCamera::loadProperties")
{
    std::string filename = "../cameras/cameraProps.json";
    #ifdef _WIN32
        std::replace(filename.begin(), filename.end(), '/', '\\');
    #endif  

    fvec3 vCameraPosition   = {0.f, 0.f, -5.f};
    fvec3 vCameraForward    = {0.f, 0.f, 1.f};
    fvec3 vCameraUp         = {0.f, 1.f, 0.f};

    fvec3 vCameraLeft;
    cross3(vCameraForward, vCameraUp, vCameraLeft);
    PinholeCamera pcam({vCameraPosition, vCameraLeft, vCameraUp, vCameraForward}, 100, 100);

    pcam.loadProperties(filename.data());
    auto m_fFocus = pcam.getFocus();

    CHECK(m_fFocus == 450.0f);
}

#define MIN_OBJECT_HEIGHT 0.001f
#define MIN_ASPECT 0.1f

PinholeCamera::PinholeCamera(int w, int h) :
    RenderSensor()
{
    m_iWidth = std::max(1, w);
    m_iHeight = std::max(1, h);
    m_pImage = new fvec4[m_iWidth*m_iHeight];
}

PinholeCamera::PinholeCamera(const ViewPoint &vp, int w, int h) :
    PinholeCamera(w, h)
{
    setViewPoint(vp);
}

void PinholeCamera::init()
{
    std::memset(m_pImage, 0, sizeof(fvec4)*static_cast<size_t>(m_iWidth*m_iHeight));
}

void PinholeCamera::stop()
{
    float scale;
    std::for_each(m_pImage, m_pImage + m_iWidth*m_iHeight, [&scale](float *p) {
        scale = p[3] < 0.5f ? 1.0f : 1.0f / p[3];
        std::transform(p, p + 3, p, Scalef{scale});
        p[3] = 1.0f;
    });
}

RenderRay PinholeCamera::getRay(int x, int y, RandomSampler &sampler) const
{
    int id = y*m_iWidth + x;

    float px = static_cast<float>(x) + 0.5f - m_fHalfW;
    float py = static_cast<float>(y) + 0.5f - m_fHalfH;

    float dx = sampler.rand(-0.5f, 0.5f);
    float dy = sampler.rand(-0.5f, 0.5f);
    fvec3 cray = {px + dx, py + dy, m_fFocus};
    normalize3(cray);

    fvec3 wray;
    multVector3(m_stViewPoint.m_rotation, cray, wray);
    return {m_stViewPoint.m_position, wray, id};
}

void PinholeCamera::hit(fvec3 radiance, RenderRay&, RenderRay *outRay)
{
    if (!outRay) return;
    int id = outRay->getIndex();
    
    if (rangeCheck(id, 0, m_iWidth * m_iHeight)) {
        fvec3 c;
        assignVector3(c, m_pImage[id]);
        std::transform(m_pImage[id], m_pImage[id] + 3, radiance, m_pImage[id], std::plus<float>{});
        m_pImage[id][3]+=1.0f;
    }
}

void PinholeCamera::getImpression(int x, int y, fvec4 v) const
{
    (rangeCheck(x, 0, m_iWidth) && rangeCheck(y, 0, m_iHeight)) ? assignVector4(v, m_pImage[y*m_iWidth + x]) : assignVector(v, 0.0f, 0.0f, 0.0f, 1.0f);
}

void PinholeCamera::loadProperties(const char *filename) noexcept
{
    nlohmann::json j;

    std::ifstream jsonFile(filename);
    if (jsonFile.is_open()) {
        jsonFile >> j;
        jsonFile.close();
    }
    else {
        std::cout << "Could not open the file to read camera properties." << std::endl;
        return;
    }

    float aspect = 0.0f;
    float yFov = 0.0f;

    if (j.find("pinhole") != j.end()){
        auto pinhole = j.at("pinhole");

        if (pinhole.find("aspect") != pinhole.end()) {
            aspect = pinhole.at("aspect").get<float>();
        }
        if (pinhole.find("focus") != pinhole.end()) {
            m_fFocus = pinhole.at("focus").get<float>();
        }
        if (pinhole.find("y-fov") != pinhole.end()) {
            yFov = pinhole.at("y-fov").get<float>();
        }
    }

    m_fHalfH = m_fFocus * std::tan(0.5f*yFov*M_PIf/180.0f);
    m_fHalfW = aspect * m_fHalfH;
}

void PinholeCamera::setRealHeight(float height, float aspect)
{
    m_fHalfH = std::max(MIN_OBJECT_HEIGHT, height);
    m_fHalfW = std::max(MIN_ASPECT, aspect) * m_fHalfH;
}

void PinholeCamera::adjustToResolution() noexcept
{
    m_fHalfH = 0.5f * static_cast<float>(m_iHeight);
    m_fHalfW = 0.5f * static_cast<float>(m_iWidth);
}

std::pair<float, float> PinholeCamera::getRealSizes() const noexcept
{
    return {m_fHalfW, m_fHalfH};
}

void PinholeCamera::setYFoV(float yFov) noexcept
{
    if (0.0f < yFov && yFov < 180.0f) {
        m_fFocus = m_fHalfH / std::tan(0.5f*yFov*M_PIf/180.0f);
    }
    else std::cerr << "Field-of-view should be greater than zero "
                      "and smaller than the straight angle." << std::endl;
}

float PinholeCamera::getFocus() const noexcept
{
    return m_fFocus;
}

PinholeCamera::~PinholeCamera()
{
    delete[] m_pImage;
}
