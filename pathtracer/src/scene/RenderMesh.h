//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      RenderMesh.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     Rendering scene mesh object

#ifndef RENDERMESH_H
#define RENDERMESH_H

#include "embree4/rtcore_geometry.h"

#include "SurfacePoint.h"

/**
 * @brief @c RenderInstance is an auxilliary data structure, which
 * wraps-up main information on the geometry entities to be accessed
 * in the scope of @a bidirectional-pathtracing rendering workflow.
 */
struct RenderInstance final
{
    /// @brief Index of the scene @a instance.
    int m_iInstId;
    /// @brief Index of the scene @a mesh.
    int m_iMeshId;
    /// @brief Instance @a transformation.
    fmat4 m_transformation;
    /// @brief Matrix for normal transformation.
    fmat3 m_normMatrix;

    /// @brief Utility call to compute normal matrix, when the instance transformation
    /// is already set.
    void computeNormalMatrix()
    {
        fmat3 TM;
        for (int i=0; i<3; ++i)
            for (int j=0; j<3; ++j)
                TM[i][j] = m_transformation[j][i];
        inverseMatrix3(TM, m_normMatrix);
    }
};

/**
 * @brief @c RenderMesh class represents scene geometry, defined as a set
 * of primitives (i.e. @a triangles ), in the form of vertex data and provides
 * wide functionality to work with it in the scope of @a bidirectional-pathtracing
 * rendering.
 */
struct RenderMesh final
{
    /// @brief Maximal number of @a texture channels.
    constexpr static int m_iMaxTexChannels = 8;

    /// @brief Mesh index.
    int m_iId;
    /// @brief Number of vertices.
    int m_iVN;
    /// @brief Number of faces.
    int m_iFN;
    /// @brief Face buffer.
    uivec3 *m_pF;
    /// @brief Vertex buffer.
    float *m_pfV;
    /// @brief Buffer stride.
    int m_iStride;
#ifdef FC_VALIDATION
    /// @brief Buffer of adjacent faces.
    ivec3 *m_pA;
#endif // !FC_VALIDATION

    /// @brief @a Vertices offset in vertex data.
    int m_iVertexOffs;
    /// @brief @a Normals offset in vertex data.
    int m_iNormalOffs;
    /// @brief @a Tangents offset in vertex data.
    int m_iTangentOffs;
    /// @brief @a Textures offset in vertex data.
    int m_iTexOffs[m_iMaxTexChannels];
    /// @brief Number of used texture channels.
    int m_iTexChN;
    /// @brief Number of filled texture channels.
    int m_iFilledTexChN;
    /// @brief Map from @a external geometry texture index to @a internal index.
    int m_iTexMap[m_iMaxTexChannels];

    /// @brief Indicates that at least one @a vertex was defined by user.
    bool m_bVertexDef;
    /// @brief Indicates that at least one @a normal was defined by user.
    bool m_vNormalDef;
    /// @brief Indicates that at least one @a tangent was defined by user.
    bool m_bTangentDef;
    /// @brief Indicates that at least one @a texture was defined by user.
    bool m_bTexDef[m_iMaxTexChannels];

    /// @brief Mesh @a material index.
    int m_iMatId;
    /// @brief @a Embree scene for the mesh.
    RTCScene m_embreeScene;

public:
    /// @publicsection Basic interface (e.g. @a -ctors).

    /// @brief Creates new empty instance of @c RenderMesh .
    RenderMesh();
    /// @brief -Dtor.
    ~RenderMesh();

    /** 
     * @brief Allocates the space for vertex buffers in the mesh and sets additional parameters.
     * 
     * @param index        Mesh index;
     * @param vertN        Number of vertices;
     * @param facesN       Number of faces;
     * @param texChannelsN Number of texture channels;
     * @param matIndex     Mesh material index;
     */
    void allocate(int index, int vertN, int facesN, int texChannelsN, int matIndex);
    /// @brief Resets mesh to its default ( @b invalid ) state.
    void reset();
    /// @brief Releases all allocated in the mesh capacity.
    void free();

public:
    /// @publicsection Getters and setters.

    /// @brief Writes vertex data @p v to the vertex buffer corresponding to the vertex index @p vid .
    void setVertex(int vid, fvec3 v);
    /// @brief Writes normal data @p n to the normal buffer corresponding to the vertex index @p nid .
    void setNormal(int nid, fvec3 n);
    /// @brief Writes tangent data @p t to the tangent buffer corresponding to the vertex index @p tid .
    void setTangent(int tid, fvec4 t);
    /// @brief Sets the face index triplet @p f to the face buffer corresponding to the face index @p fid .
    void setFace(int fid, uivec3 f);
    /// @brief Sets texture data @p tc to the texture buffer @p tcid under channel @p chid .
    void setTextureCoordinate(int chid, int tcid, fvec2 tc);

    /// @brief Sets vertex buffer from raw vertex data @p v with a respective stride @p vstride .
    void setVertices(const float* v, int vstride);
    /// @brief Sets normal buffer from raw normal data @p n with a respective stride @p nstride .
    void setNormals(const float* n, int nstride);
    /// @brief Sets tangent buffer from raw tangent data @p t with a respective stride @p tstride .
    void setTangents(const float* t, int tstride);
    /// @brief Sets texture buffer from raw texture data @p tc under channel @p chid
    /// with a respective stride @p tcstride .
    void setTextureCoordinates(int chid, const float *tc, int tcstride);
    /// @brief Sets face buffer from raw face data @p f .
    void setFaces(unsigned int* f);

    /// @brief Retrieves texture coordinates @p tc from face @p faceId and channel @p channel
    /// using face barycentric coordinates @p bc .
    bool getTexCoord(const fvec3 bc, int faceId, fvec3 tc, int channel);
    /**
     * @brief Computes surface point on instance of current mesh.
     * 
     * @param sp    Surface point;
     * @param TM    Transformation matrix of the instance;
     * @param normM Normal matrix of the instance;
     */
    void computeSurfacePoint(SurfacePoint &sp, const fmat4 TM, const fmat3 normM);
    /**
     * @brief Simplified version of @ref RenderMesh::computeSurfacePoint(SurfacePoint &, const fmat4, fmat3),
     * computes only position on the instance surface for given face index @p fid , barycentric coordinates
     * @p bc with transformation matrix @p TM .
     * @overload void computeSurfacePoint(SurfacePoint &, const fmat4, fmat3)
     */
    void computeSurfacePoint(int fid, fvec3 bc, const fmat4 TM, fvec3 pos) const;
    
    /// @brief Computes all missing data using normal texture channel index @p normalTexCh .
    bool commitMesh(int normalTexCh = -1);
    /// @brief Checks the validity of the mesh for being committed.
    bool isValid() const noexcept;

    /// @brief Counts the number of @b invalid tangents.
    /// @note Internal utility check.
    int testTangent() const noexcept;

#ifdef FC_VALIDATION
    /// @brief Returns the minimum and maximum density within the mesh
    /// w.r.t. to the transformation @p TM . 
    std::pair<float, float> getMinMaxDensity(fmat4 TM) const;
#endif // !FC_VALIDATION

private:
    void computeNormals();
    void computeTangents(int tid); // Build tangent based on texture coordinates
    void computeTangents(); // Build random tangent
    void randomTangent(const fvec3 nv, fvec3 tv, float scale=1.0f);
    void computeNeighbors();
};

#endif // !RENDERMESH_H
