//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      Mesh.cpp
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-25
/// @brief     Support for meshes

#include "doctest.h"
#include "tiny_gltf.h"

#include "Exceptions.h"
#include "Mesh.h"

/// @brief Unit test for Mesh::Mesh 
TEST_CASE("Testing Mesh::Mesh")
{
	std::string filename = "../objects/cube_gold.gltf";
	#ifdef _WIN32
		std::replace(filename.begin(), filename.end(), '/', '\\');
	#endif

	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err, warn;

	bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
	if (!ret)
		throw GltfError("Could not load glTF file " + filename + ": " + err);

	// Check if correct mesh is loaded
	std::string name = "Cube";
	bool matchingName = false;
	for (const tinygltf::Mesh& gltfMesh : model.meshes)
	{
		Mesh *pMesh = new Mesh(gltfMesh.name);
		std::string meshName = pMesh->getName();
		if (meshName == name)
			matchingName = true;
	}
	CHECK(matchingName);
}

void Mesh::getVerticesOfTriangle(size_t uTriangleIndex, float *&V0, float *&V1, float *&V2)
{
    float *VertexBuffer = m_vfVertexBuffer.data();
    const unsigned int *indexBuffer = m_vuiIndexBuffer.data();
    uTriangleIndex*=3;
    V0=VertexBuffer+indexBuffer[uTriangleIndex+0]*3;
    V1=VertexBuffer+indexBuffer[uTriangleIndex+1]*3;
    V2=VertexBuffer+indexBuffer[uTriangleIndex+2]*3;
}

void Mesh::getBBox(BoundingBox<float>& BB, const fmat4 TM) const
{
	const float* pV = m_vfVertexBuffer.data();
	auto V = reinterpret_cast<const fvec3*>(pV);

	fvec3 tp;
	std::for_each(V, V + m_vfVertexBuffer.size() / 3, [&BB, TM, &tp](const float *v) {
		multVector3(TM, v, tp);
		BB.add(tp);
	});
}
