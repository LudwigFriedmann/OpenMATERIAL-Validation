//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      BDPTRenderer.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Bidirectional pathtracing rederer class

#ifndef BDPTRENDERER_H
#define BDPTRENDERER_H

#include <atomic>
#include <thread>

#include "RenderSensor.h"
#include "RenderScene.h"
#include "RendererParameters.h"

/// @brief @c BDPTPathPart is an aggregate which stores a chunk of
/// information about @a ray path.
struct BDPTPathPart final
{
    /// @brief Outgoing ray @a direction.
    fvec3 m_outRay;
    /// @brief Point of @a intersection between ray and surface.
    SurfacePoint m_surfPoint;
    /// @brief Outgoing ray @a radiance.
    fvec3 m_outRad;
    /// @brief Outgoing radiance @a scaling factors.
    fvec3 m_outFactor;
};

/**
 * @brief @c BDPTRenderer class is the main rendering entity which
 * implements idea of @a bidirectional-pathtracing rendering and
 * assures robust and fast computation.
 */
class BDPTRenderer final
{
    /// @brief Scaling factor along the ray away from surface point.
    constexpr static float m_fEpsAcc = 3.0f;

    /// @brief Rendering @a parameters.
    RendererParameters* m_pParams;
    /// @brief Rendering @a scene.
    RenderScene* m_pScene;
    /// @brief Camera traced path data for each thread.
    BDPTPathPart** m_ppCPBuf;
    /// @brief Light traced path data for each thread.
    BDPTPathPart** m_ppLPBuf;
    /// @brief Memory buffer for @ref BDPTRenderer::m_ppCPBuf and @ref BDPTRenderer::m_ppLPBuf.
    BDPTPathPart* m_pPathMem;

    /// @brief Number of threads for a @a parallel rendering.
    /// Unlike @ref RendererParameters::numberOfCores, this variable is modified to "match" image height.
    int m_iCoresN = 0;
    /// @brief @a Atomic counter for the current rendering row. 
    std::atomic_int m_aiCurRow {};

    /// @brief Computes maximal (in absolute sense) component in the given vector @p v ,
    /// greater than some threshold @p th .
    static float maxAbsInVector(fvec3 v, float th = 1.0f);
    /// @brief Shifts the hitpoint in @p ray away from surface along the
    /// @b outgoing ray using some scaling factor @p epsScale to avoid
    /// self-intersections.
    static void shiftRayAlongDirection(RTCRayHit& ray, float epsScale = 3.0f);
    /// @brief Computes the @a intensity of the radiance vector @p r .
    static float intensity(const fvec3 r);
    /// @brief Computes light attenuation over the distance @p d with respect
    /// to normal attenuation distance @p attD .
    float computeLightAttenuation(float d, float attD);
    /// @brief Computes and fills proper data in @p sp using @a Embree's hit data @p hit .
    void computeSurfacePoint(RTCRayHit& hit, SurfacePoint& sp);
    /** 
     * @brief Shoots @a Embree's ray on the @a Embree's scene to get @a Embree's hit data
     * and writes data to surface point @p sp . Surfaces masked by materials are skipped.
     * 
     * @param esc       @a Embree scene;
     * @param rhit      @a Embree ray-hit information;
     * @param sp        Surface point;
     * 
     * @return @c True , if the valid intersection with @a non-masked material is found within
     * maximal possible number of tracing iterations, @c false otherwise.
     */
    bool sceneIntersect(RTCScene esc, RTCRayHit& rhit, SurfacePoint& sp);

    /** 
     * @brief Computes a single path information.
     * 
     * @param P     Traced path data;
     * @param max_n Maximum number of points in the path (including @a camera or @a light point);
     * @param O     Ray @a origin;
     * @param R     Ray @a direction;
     * @param Rad   For light path - input radiance from the light, for camera path - material emission collected along the path;
     * @param n     Number of points in calculated path (including @a camera or @a light point);
     * @param inv   Defines if the path calculated from the light source ( @c true ) or from the camera ( @c false );
     * @param attD  Attenuation distance for light source;
     * 
     * @return @c True if the path exited the scene, @c false otherwise.
     */
    bool computePath(BDPTPathPart* P, int max_n, fvec3 O, fvec3 R, fvec3 Rad, int &n,
                    bool inv = false, float attD = std::numeric_limits<float>::max());
    /// @brief Tries to compute a @a light path for the given path length @p len and thread @p tid .
    /// If it is possible, returns corresponding light source, otherwise @c nullptr .
    RenderLight* tryComputeLightPath(int& len, int tid);
    /** 
     * @brief Function for computing paths from @a camera and @a light source and connecting them.
     * 
     * @param O         @a Origin of the ray from the sensor;
     * @param R         @b Unit @a direction of the ray from the sensor;
     * @param radiance  Intensity of the light, returned by traced path;
     * @param camRay    The ray returned to the sensor (in case of camera it is a ray from the first
     *                  hit surface in the direction opposite to @p R );
     * @param tid       Index of the thread that calls this function;
    */
    void computePaths(fvec3 O, fvec3 R, fvec3 radiance, RenderRay& camRay, int tid = 0);  

    /// @brief Increases incoming radiance @p rad by @p rad_inc scaled with factor @p kf .
    static void increaseRadiance(fvec3 rad, const fvec3 rad_inc, float kf);
    /// @brief Fills ray-hit @p rh data with direction @p R and origin @p O , which is slightly
    /// tilted along the direction using some scaling factor @p epsShift.
    static void setRay(const fvec3 O, const fvec3 R, RTCRayHit& rh, float epsShift = 1.0f);
    /// @brief Checks whether two points @p A and @p B are @b directly connected on the scene.
    bool isConnected(fvec3 A, fvec3 B);
    /// @brief Allocates capacity for future @a camera and @a light tracing path information as
    /// @p cpn and @p lpn respectively.
    void allocatePathBufs(int cpn, int lpn);
    /// @brief Releases all reserved memory.
    void freePathBuffers();
    /// @brief Renders a single row using the sensor @p sensor on the thread @p tid .
    void renderRow(RenderSensor* sensor, int tid);

public:
    /// @publicsection Basic interface (e.g. @a -ctors).

    /// @brief Creates new empty instance of @c BDPTRenderer .
    BDPTRenderer();
    /// @brief -Dtor.
    ~BDPTRenderer();

public:
    /// @publicsection Getters and setters.

    /// @brief Sets rendering @a parameters to @p p .
    void setParameters(RendererParameters* p) noexcept;
    /// @brief Sets rendering @a scene to @p s .
    void setScene(RenderScene* s) noexcept;

public:
    /// @publicsection Interaction interface.

    /// @brief @b Core @b function. Renders current scene using provided sensor @p sensor .
    void render(RenderSensor* sensor);
};

#endif // !BDPTRENDERER_H
