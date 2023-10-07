//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      Mesh.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-25
/// @brief     Support for meshes

#ifndef MESH_H
#define MESH_H

#include <vector>

#include "BoundingBox.h"

class Node;
class Material;

#define MAX_UV_CHANNELS 8

/**
 * @brief Mesh class which stores a set of polygon primitives and the underlying material.
 *
 * @details This class supports mesh storage as well as intersection calculation. Provides a
 * wide geometry functional to work with.
 */
struct Mesh final
{
    /// @brief Mesh number in geometry mesh array.
    int m_iId = -1;
    /// @brief Name of the mesh (string may be empty).
    std::string m_sName;

    /// @brief Pointer to nodes to which the mesh belongs.
    std::vector<Node*> m_vpNodes;    
    /// @brief Buffer containing the vertex indices.
    std::vector<unsigned int> m_vuiIndexBuffer;
    /// @brief Buffer containing the vertices.
    std::vector<float> m_vfVertexBuffer;
    /// @brief Buffer containing the normals (if empty use flat shading).
    std::vector<float> m_vfNormalBuffer;
    /// @brief Buffer containing the tangents .
    std::vector<float> m_vfTangentBuffer;
    /// @brief Material associated to the mesh.
    const Material *m_pMaterial = nullptr;
    /// @brief Buffers containing texture coordinates.
    std::vector<float> m_vfTexCoordBuffers[MAX_UV_CHANNELS];

public:
    /// @publicsection Default methods and basic interface (e.g. @a -ctors).

    /// @brief Create a new mesh with name given by @p crsName .
    explicit Mesh(std::string crsName = "") : m_sName(std::move(crsName)) {}
    /// @brief Gets name of mesh.
    std::string getName() const noexcept {return m_sName;}
    /// @brief Adds a node @p pNode to which the mesh belongs (multiple nodes can use the same mesh).
    void addParentNode(Node *pNode) { m_vpNodes.push_back(pNode); }
    /// @brief Returns the number of triangles stored in the mesh.
    unsigned getNumberOfTriangles() const { return static_cast<unsigned>(m_vuiIndexBuffer.size() / 3); }
    /// @brief Writes three vertices @p V0 , @p V1 and @p V2 of the triangle with index @p uTriangleIndex .
    void getVerticesOfTriangle(size_t uTriangleIndex, float* &V0, float* &V1, float* &V2);
    /// @brief Writes bounding box of the mesh to @p BB with respect to transformation @p TM .
    void getBBox(BoundingBox<float>& BB, const fmat4 TM) const;
};

#endif // !MESH_H
