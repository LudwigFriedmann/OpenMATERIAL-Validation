//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      AssetGeometry.cpp
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-30
/// @brief     AssetGeometry class

#include <cstring> // std::memcpy
#include <cstddef> // std::size_t
#include <set> // std::set

#include "CImg.h"
#include "doctest.h"
#include "tiny_gltf.h"

#include "AssetGeometry.h"
#include "materials/AssetMaterial.h"
#include "materials/PbrMaterial.h"
#include "ReferenceLink.h"

using namespace nlohmann;
using namespace cimg_library;

std::string AssetGeometry::m_sMaterialLoadingPriority = "om";
bool AssetGeometry::m_bUseLights = false;

/// @brief Unit test for AssetGeometry::AssetGeometry
TEST_CASE("Testing AssetGeometry::AssetGeometry")
{
    std::string filename = "../scenes/multiple_objects.gltf";
    #ifdef _WIN32
		std::replace(filename.begin(), filename.end(), '/', '\\');
	#endif

    // Check if loaded geometry has 6 nodes (2 from main gltf, and 4(3+1) from referenced gltfs)
    AssetGeometry assetGeometry(filename);
    CHECK(assetGeometry.m_vpNodes.size() == 6);
}

/// @brief Unit test for AssetGeometry::loadNodes (loading IDs from glTF feature)
TEST_CASE("Testing AssetGeometry::loadNodes")
{
    std::string filename = "../scenes/multiple_objects.gltf";
    #ifdef _WIN32
		std::replace(filename.begin(), filename.end(), '/', '\\');
	#endif

    // Check if node with index 5 has proper id
    AssetGeometry assetGeometry(filename);
    auto node = assetGeometry.m_vpNodes[5];
    CHECK(node->m_sId == "bda2ddb6-2134-4c11-ba8a-a4d1012bb6d1");
}

/// @brief Unit test for AssetGeometry::loadMaterial (duplicate loading feature)
TEST_CASE("Testing AssetGeometry::loadMaterial")
{
    std::string filename = "../scenes/multiple_objects.gltf";
    #ifdef _WIN32
		std::replace(filename.begin(), filename.end(), '/', '\\');
	#endif

    // Two gltfs are referenced from the scene, both use gold material and one of them uses two more materials (iron and aluminium)
    // In total, three materials should be loaded
    AssetGeometry assetGeometry(filename);
    CHECK(assetGeometry.m_vpMaterials.size() == 3);
}

/// @brief Unit test for AssetGeometry::getBBox
TEST_CASE("Testing AssetGeometry::getBBox")
{
	std::string filename = "../objects/cube_gold.gltf";
	#ifdef _WIN32
		std::replace(filename.begin(), filename.end(), '/', '\\');
	#endif
	
	// Check if the bounding box has spatial extension
	AssetGeometry assetGeometry(filename);
	CHECK(assetGeometry.getBBox().isValid());
}

/// @brief Unit test for AssetGeometry::loadMaterial (missing material feature)
TEST_CASE("Testing AssetGeometry::loadMaterial")
{
	std::string filename = "../objects/icospheres_gold_iron_missing.gltf";
	#ifdef _WIN32
		replace(filename.begin(), filename.end(), '/', '\\');
	#endif
	
	// Check if mesh with index=1 has the missing material set as its material
	AssetGeometry assetGeometry(filename);
    auto pMesh = assetGeometry.m_vpPrimitives[1];
    auto pMeshMaterial = pMesh->m_pMaterial;
    auto pMissingMaterial = assetGeometry.m_pMissingMaterial;
	CHECK(pMeshMaterial == pMissingMaterial);
}

/// @brief Unit test for AssetGeometry::loadNodes (copying nodes feature)
TEST_CASE("Testing AssetGeometry::loadNodes")
{
	std::string filename = "../scenes/multiple_instances.gltf";
	#ifdef _WIN32
		std::replace(filename.begin(), filename.end(), '/', '\\');
	#endif
	
	// Nodes with index 0 and 3 are both from the same referenced glTF, therefore node[3] was made as a copy of node[0]
    // Check if they both point to the same vector of primitives (as defined in the copy constructor)
	AssetGeometry assetGeometry(filename);
    auto vpNodes = assetGeometry.m_vpNodes;
    auto pNode1 = vpNodes[0];
    auto pNode2 = vpNodes[3];

	CHECK(pNode1->m_vpPrimitives == pNode2->m_vpPrimitives);
}

/// @brief Unit test for AssetGeometry::loadMesh (multiple primitives feature)
TEST_CASE("Testing AssetGeometry::loadMesh")
{
    std::string filename = "../objects/cubes_pbr.gltf";
	#ifdef _WIN32
		std::replace(filename.begin(), filename.end(), '/', '\\');
	#endif

    AssetGeometry assetGeometry(filename);
    CHECK(assetGeometry.m_vpPrimitives.size() == 2);
}

/// @brief Unit test for AssetGeometry::loadLight
TEST_CASE("Testing AssetGeometry::loadLight")
{
    std::string filename = "../objects/plane_pointlight.gltf";
    #ifdef _WIN32
		std::replace(filename.begin(), filename.end(), '/', '\\');
	#endif

    AssetGeometry assetGeometry(filename);
    AssetGeometry::m_bUseLights ? CHECK(assetGeometry.m_vpLights.front() != nullptr) : CHECK(assetGeometry.m_vpLights.empty());
}

namespace {

    /// @brief Copy indices from @a glTF data format.
    ///
    /// @details Copy @p count elements of data type @p componentType from address @p src to the
    /// memory address given by the pointer @p dest.
    ///
    /// @param [in] dest destination address
    /// @param [in] src source address
    /// @param [in] componentType e.g. 5126 for float (see @a glTF specification for more details)
    /// @param [in] count number of elements
    void copyIndicesFromArray(unsigned int *dest, void const *src, int componentType, size_t count)
    {
        if(componentType == TINYGLTF_COMPONENT_TYPE_BYTE)
        {
            auto p = reinterpret_cast<const signed char *>(src);
            for(size_t i = 0; i < count; i++)
                dest[i] = static_cast<unsigned int>(static_cast<unsigned char>(p[i]));
        }
        else if(componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
        {
            auto p = reinterpret_cast<const unsigned char *>(src);
            for(size_t i = 0; i < count; i++)
                dest[i] = p[i];
        }
        else if(componentType == TINYGLTF_COMPONENT_TYPE_SHORT)
        {
            auto p = reinterpret_cast<const short *>(src);
            for(size_t i = 0; i < count; i++)
                dest[i] = static_cast<unsigned int>(p[i]);
        }
        else if(componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
        {
            auto p = reinterpret_cast<const unsigned short *>(src);
            for(size_t i = 0; i < count; i++)
                dest[i] = p[i];
        }
        else if(componentType == TINYGLTF_COMPONENT_TYPE_INT)
        {
            auto p = reinterpret_cast<const int *>(src);
            for(size_t i = 0; i < count; i++)
                dest[i] = static_cast<unsigned int>(p[i]);
        }
        else if(componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
        {
            auto p = reinterpret_cast<const unsigned int *>(src);
            for(size_t i = 0; i < count; i++)
                dest[i] = p[i];
        }
    }

    /// @brief Check if node @p uNodeNumber is a root node.
    ///
    /// @param [in] gltfModel tinygltf object
    /// @param [in] uNodeNumber node number
    bool checkNodeIsRoot(const tinygltf::Model &gltfModel, size_t uNodeNumber)
    {
        // Check if any node contains uNodeNumber as child...
        const auto &nodes = gltfModel.nodes;
        return std::all_of(nodes.begin(), nodes.end(), [uNodeNumber](const tinygltf::Node &gltfNode) {
            const auto &children = gltfNode.children;
            return std::find(children.begin(), children.end(), static_cast<int>(uNodeNumber)) == children.end();
        });
    }
} // anonymous namespace


AssetGeometry::AssetGeometry(const std::string &crsFilename)
{
    tinygltf::Model gltfModel = loadGltfModel(crsFilename);
    loadAssetInfoData(gltfModel);
    loadGltf(crsFilename, gltfModel);
    addSceneNodes();
}

AssetGeometry::AssetGeometry(const tinygltf::Model &gltfModel, const std::string &crsDirectory) :
    AssetInfo(gltfModel.j)
{
    loadAssetInfoData(gltfModel.j);
    loadGltf(gltfModel, crsDirectory);
    addSceneNodes();
}

AssetGeometry::~AssetGeometry()
{
    free();
}

BoundingBox<float> AssetGeometry::getBBox() const
{
    BoundingBox<float> BB;
    std::for_each(m_vpNodes.begin(), m_vpNodes.end(), [&BB](Node *pNode) {
        if (!pNode->hasParent()) BB.add(pNode->getBBox());
    });
    return BB;
}

tinygltf::Model AssetGeometry::loadGltfModel(const std::string &crsFilename)
{
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;

    auto sExtension = utils::path::fileextension(crsFilename);

    // Check the extension of the glTF file
    if (sExtension == "gltf") {
        if (!loader.LoadASCIIFromFile(&model, &err, &warn, crsFilename))
            throw GltfError("Could not load glTF file " + crsFilename + ": " + err);
    }
    else if (sExtension == "glb") {
        if (!loader.LoadBinaryFromFile(&model, &err, &warn, crsFilename))
            throw GltfError("Could not load glTF file " + crsFilename + ": " + err);
    }
    else {
        throw GltfError("Wrong file extension in file: " + crsFilename + ": " + err);
    }
    
    return model;
}

std::string AssetGeometry::checkMaterialType(const json &j)
{
    if (AssetGeometry::m_sMaterialLoadingPriority == "om") {
        if (AssetBase::hasKey(j, "extensions")) {
            const json jExtensions = j.at("extensions");
            if (AssetBase::hasKey(jExtensions, "OpenMaterial_reference_link")) {
                return "openMaterial";
            }
        }
        if (AssetBase::hasKey(j, "pbrMetallicRoughness")) {
            return "pbr";
        }
    }
    if (AssetGeometry::m_sMaterialLoadingPriority == "pbr") {
        if (AssetBase::hasKey(j, "pbrMetallicRoughness")) {
            return "pbr";
        }
        if (AssetBase::hasKey(j, "extensions")) {
            const json jExtensions = j.at("extensions");
            if (AssetBase::hasKey(jExtensions, "OpenMaterial_reference_link")) {
                return "openMaterial";
            }
        }
    }
    return "unknown";
}


void AssetGeometry::loadAssetInfoData(const tinygltf::Model &gltfModel)
{
    loadAssetInfoData(gltfModel.j);
}

void AssetGeometry::loadAssetInfoData(const nlohmann::json &jsonDoc)
{
    AssetBase::loadData(jsonDoc);
    AssetInfo::loadData(jsonDoc);
}


void AssetGeometry::loadGltf(std::string sFilename, const tinygltf::Model& gltfModel, Node* pParentNode)
{
    m_sFilename = std::move(sFilename);
    const std::string sDirName = utils::path::dirname(m_sFilename);
    auto jsonDoc = gltfModel.j;

    auto vpNewNodes = loadData(gltfModel, sDirName);    

    fixNodeNumbers();

    auto pAssetInfoToAdd = new AssetInfo(jsonDoc);
    auto sAssetInfoCategory = pAssetInfoToAdd->getCategoryString();
    
    if (pParentNode) {
        pParentNode->m_pAssetInfo = pAssetInfoToAdd;
        pParentNode->m_sCategory = sAssetInfoCategory;
    }
    else {
        delete pAssetInfoToAdd;
    }
    
    for (Node* pNode : vpNewNodes)
    {
        std::string sRefLink = pNode->m_sUri;
        
        // Checking if a glTF from a certain filepath was previously loaded
        auto iter = m_mLoadedFiles.find(sRefLink);
        auto bFound = iter != m_mLoadedFiles.end();

        // If this is not the first occurance of a specific filepath in reference link, nodes are copied
        // This removes the need to open the file again
        if (bFound) {
            auto vChildren = iter->second;
            for (auto &&pChildNode : vChildren){
                auto pChildNodeCopy = new Node(*pChildNode);
                m_vpNodes.push_back(pChildNodeCopy);

                auto vDescendants = pChildNodeCopy->getAllDescendants();
                for(auto &&pDescedant : vDescendants){
                    int iNodeNum = static_cast<int>(m_vpNodes.size());
                    pDescedant->m_uNodeNumber=static_cast<size_t>(iNodeNum);
                    m_vpNodes.push_back(pDescedant);
                }

                pChildNodeCopy->m_pParent=pNode;
                pNode->addChild(pChildNodeCopy);
            }
            fixNodeNumbers();
        }

        // If this is the occurance of a specific filepath in reference link, nodes are loaded normally
        if (!bFound)
        {
            bool bIsRoot = !pNode->hasParent();
            if (pParentNode && bIsRoot) {
                pNode->m_pParent = pParentNode;
                pParentNode->addChild(pNode);
            }

            // This part checks if a certain node is a root node in its file and if the file is opened via reference link
            // If it is, than it's written in a map of filepaths and root nodes on those filepaths
            // This is used for copying nodes when a filepaths occurs more than once
            if (bIsRoot && pParentNode && !pParentNode->m_sUri.empty()) {
                auto it = m_mLoadedFiles.find(pParentNode->m_sUri);
                bFound = it != m_mLoadedFiles.end();

                if (!bFound) {
                    std::vector<Node*> vpRootNodes;
                    vpRootNodes.push_back(pNode);
                    m_mLoadedFiles.emplace(pParentNode->m_sUri, vpRootNodes);
                }
                else {
                    it->second.push_back(pNode);
                }
            }

            if (!sRefLink.empty()) {
                (sDirName + utils::path::getFileSeparator()).append(sRefLink).swap(sRefLink);
                Node* pNewParentNode = pNode;
                std::cout << "Loading " + sRefLink + "\n";
                tinygltf::Model childModel = loadGltfModel(sRefLink);
                loadGltf(sRefLink, childModel, pNewParentNode);
            }
            else if (pNode->m_sCategory.empty()) {
                pNode->m_sCategory = sAssetInfoCategory;
            }
        } 
    }
}

void AssetGeometry::loadGltf(const tinygltf::Model& gltfModel, const std::string& crsDirectory, Node* pParentNode)
{
    auto vpNewNodes = loadData(gltfModel, crsDirectory);
    fixNodeNumbers();

    auto assetInfoToAdd = new AssetInfo(gltfModel.j);

    if (pParentNode) {
        pParentNode->m_pAssetInfo = assetInfoToAdd;
        pParentNode->m_sCategory = assetInfoToAdd->getCategoryString();
    }

    std::for_each(vpNewNodes.begin(), vpNewNodes.end(), [&crsDirectory, &pParentNode, assetInfoToAdd, this](Node *pNode) {

        std::string sRefLink = pNode->m_sUri;
        std::cout << "Loading " + sRefLink + "\n";

        if (pParentNode) {
            pNode->m_pParent = pParentNode;
            pParentNode->addChild(pNode);
        }

        if (!sRefLink.empty()) {
            (crsDirectory + utils::path::getFileSeparator()).append(sRefLink).swap(sRefLink);
            Node *pNewParentNode = pNode;

            tinygltf::Model gltfModelRef = loadGltfModel(sRefLink);
            std::string sDirName = utils::path::dirname(sRefLink);
            
            loadGltf(gltfModelRef, sDirName, pNewParentNode);
        }
        else {
            if (pNode->m_sCategory.empty()) {
                pNode->m_sCategory = assetInfoToAdd->getCategoryString();
            }
        }
    });
}


std::vector<Node*> AssetGeometry::loadData(const tinygltf::Model& gltfModel, std::string crsDirectory)
{
    m_sDirectory = std::move(crsDirectory) + utils::path::getFileSeparator();

    // Get the default scene. The property "scene" is optional and tinygltf will
    // set it to -1 if not present. If scene is not present, use the first scene
    // and set uDefaultScene to 0.
    size_t uDefaultScene = 0;
    if (gltfModel.defaultScene >= 0)
        uDefaultScene = static_cast<size_t>(gltfModel.defaultScene);

    // Check if the default scene is present in scenes
    if (gltfModel.scenes.size() <= uDefaultScene)
        throw GltfError(getUuidAsString().append(": default scene not present"));

    // Save the nodes belonging to the scene
    m_viSceneNodes = gltfModel.scenes[uDefaultScene].nodes;

    // Loading images
    loadImages(gltfModel);

    try {
        auto vpProcessedNodes = loadNodes(gltfModel);

        // Check that all referenced scene nodes actually exist
        auto found = std::find_if(m_viSceneNodes.begin(), m_viSceneNodes.end(), std::bind(std::greater_equal<int>{}, _1, m_vpNodes.size()));
        if (found != m_viSceneNodes.end()) throw GltfError(getUuidAsString().append(": unresolved reference to node ").append(std::to_string(*found)));

        return vpProcessedNodes;
    }
    catch (...) {
        // Tidy up and rethrow exception
        free();
        throw;
    }
}

std::vector<Node *> AssetGeometry::loadNodes(const tinygltf::Model& gltfModel)
{
    // Number of all nodes
    size_t uNodes = gltfModel.nodes.size();
    // Create return vector
    std::vector<Node *> vpProcessedNodes;
    // Create loaded meshes map
    std::map<int, std::vector<Mesh *>> mLoadedMeshes;
    // Create loaded materials map
    std::map<int, const Material *> mLoadedMaterials;

    if (uNodes == 0) return vpProcessedNodes;

    // Create uninitialized nodes
    m_vpNodes.reserve(uNodes);
    for (size_t i = 0; i < uNodes; i++)
        m_vpNodes.insert(m_vpNodes.begin(), new Node(i));

    // Initialize the nodes
    for (size_t uNodeNumber = 0UL; uNodeNumber < uNodes; uNodeNumber++)
    {
        const tinygltf::Node& gltfNode = gltfModel.nodes[uNodeNumber];
        Node *pCurrentNode = m_vpNodes[uNodeNumber];
        // Set name of the current node
        pCurrentNode->m_sName = gltfNode.name;
        
        // Set uri of glTF that is referenced in a node
        try {
            tinygltf::Value refLinkObject = gltfNode.extensions.at("OpenMaterial_reference_link");
            std::string sUri = refLinkObject.Get("uri").Get<std::string>();
            pCurrentNode->m_sUri=sUri;
        }
        catch (...) {}

        // Try to set category and id from node
        if (gltfNode.extras.IsObject()) {

            tinygltf::Value extrasObject = gltfNode.extras;
            try {
                std::string sCategoryFromExtras = extrasObject.Get("category").Get<std::string>();
                pCurrentNode->m_sCategory=sCategoryFromExtras;
            }
            catch (...) {
                std::cout << "No category in node[" << uNodeNumber << "] extras" << std::endl;
            }

            try {
                std::string sIdFromExtras = extrasObject.Get("id").Get<std::string>();
                pCurrentNode->setNodeId(sIdFromExtras);
            }
            catch (...) {
                std::cout << "No ID in node[" << uNodeNumber << "] extras" << std::endl;
            }
        }

        int iMeshIndex = gltfNode.mesh;
        
        // Add the mesh to the node; a node has either one or no mesh. The mesh
        // number is saved in gltfNode.mesh if the number is not negative.
        if (iMeshIndex >= 0) {
            // If a mesh was not previously loaded, load it
            if (mLoadedMeshes.find(iMeshIndex) == mLoadedMeshes.end()) {
                auto vpPrimitives = loadMesh(gltfModel, iMeshIndex, mLoadedMaterials);
                std::for_each(vpPrimitives.begin(), vpPrimitives.end(), std::bind(&Mesh::addParentNode, _1, pCurrentNode));
                pCurrentNode->addPrimitives(vpPrimitives);
                mLoadedMeshes.emplace(iMeshIndex, vpPrimitives);
            }
            // If it's already loaded, use the same mesh for different node
            else {
                auto vpPrimitives = mLoadedMeshes.at(iMeshIndex);
                pCurrentNode->addPrimitives(vpPrimitives);
            }
        }
        // Load the lights from the glTF        
        if (!gltfNode.extensions.empty())
        {
            auto extensionsObject = gltfNode.extensions;
            try {
                if (extensionsObject.at("KHR_lights_punctual").IsObject() && m_bUseLights) {
                    auto lightsObject = extensionsObject.at("KHR_lights_punctual");
                    auto lightIndex = lightsObject.Get("light").Get<int>();
                    auto pLight = loadLight(gltfModel, lightIndex);
                    pLight->setNode(pCurrentNode);
                }
            }
            catch(...) {}
        }

        // Read the transformation for the node
        if(!gltfNode.matrix.empty())
        {
            fmat4& M=m_vpNodes[uNodeNumber]->m_Transformation;
            const double* gm=gltfNode.matrix.data();
            for (int i=0; i<16; ++i)
                M[i%4][i/4]=static_cast<float>(gm[i]);
        }
        else
        {
            // Default values that correspond to identity
            fvec3 nT = {0.0f,0.0f,0.0f};
            fvec3 nS = {1.0f,1.0f,1.0f};
            fvec4 nQ = {0.0f,0.0f,0.0f,1.0f};
            if (!gltfNode.scale.empty()) castVector(gltfNode.scale.data(),nS,3);
            if (!gltfNode.rotation.empty()) castVector(gltfNode.rotation.data(),nQ,4);
            if (!gltfNode.translation.empty()) castVector(gltfNode.translation.data(),nT,3);
            transformationFromQTS(m_vpNodes[uNodeNumber]->m_Transformation, nQ, nT, nS);
        }

        // Add children
        for (int child: gltfNode.children)
        {
            auto cid = static_cast<size_t>(child);
            // Check that the referenced node exists
            if (cid >= m_vpNodes.size())
                throw GltfError(getUuidAsString().append(": unresolved reference to node ").append(std::to_string(cid)));

            Node *childNode = m_vpNodes[cid];
            // Add child to current node
            pCurrentNode->addChild(childNode);
            // Set parent of child node
            childNode->m_pParent = pCurrentNode;
        }
        
        vpProcessedNodes.push_back(pCurrentNode);
    }

    // Check that all nodes in scene are root nodes
    for (int iNodeNumber : m_viSceneNodes)
        if(!checkNodeIsRoot(gltfModel, static_cast<size_t>(iNodeNumber)))
            throw GltfError(getUuidAsString().append(": node ").append(std::to_string(iNodeNumber)).append(" not a root node"));

    // Check that node structure is not cyclic
    // The node structure is cyclic if one of node has a descent that is
    // itself; i.e., if one parent, grandparent... has the same node number
    for(const Node *pNode : m_vpNodes)
        if(pNode->hasParent(pNode->m_uNodeNumber))
            throw GltfError(getUuidAsString().append(": node structure is cyclic"));

    return vpProcessedNodes;
}

std::vector<Mesh*> AssetGeometry::loadMesh(const tinygltf::Model &gltfModel, int iMeshIndex, std::map<int, const Material *> &mLoadedMaterials)
{
    // this is the return vector
    std::vector<Mesh*> vPrimitives;
    auto gltfMesh = gltfModel.meshes[static_cast<size_t>(iMeshIndex)];

    for(const auto &meshPrimitive : gltfMesh.primitives) {

        auto pMesh = new Mesh(gltfMesh.name);

        std::vector<float>& vVertexBuffer = pMesh->m_vfVertexBuffer;
        std::vector<float>& vNormalBuffer = pMesh->m_vfNormalBuffer;
        std::vector<unsigned int >& vIndexBuffer = pMesh->m_vuiIndexBuffer;
        std::vector<float>& vTangentBuffer = pMesh->m_vfTangentBuffer;
        std::vector<float>* vTexCoordBuffer = pMesh->m_vfTexCoordBuffers;
        size_t foundTC = std::string::npos;

        const auto& indicesAccessor = gltfModel.accessors[static_cast<size_t>(meshPrimitive.indices)];
        const auto& bufferView = gltfModel.bufferViews[static_cast<size_t>(indicesAccessor.bufferView)];
        const auto& buffer = gltfModel.buffers[static_cast<size_t>(bufferView.buffer)];
        const auto dataAddress = buffer.data.data() + bufferView.byteOffset + indicesAccessor.byteOffset;

        // Only triangles are supported as primitives
        if (meshPrimitive.mode != TINYGLTF_MODE_TRIANGLES)
            throw NotImplementedError(getUuidAsString().append(": primitives other than triangles not supported"));

        /// Checking if a material that a mesh uses was already loaded
        int iMaterialIndex = meshPrimitive.material;
        if (iMaterialIndex >= 0) {

            auto it = mLoadedMaterials.find(iMaterialIndex);
            bool bFound;
            bFound = it != mLoadedMaterials.end();
            // Material is loaded if it's not found
            if (!bFound) {
                auto pMaterial = loadMaterial(gltfModel, iMaterialIndex);
                pMesh->m_pMaterial = pMaterial;
                mLoadedMaterials[iMaterialIndex] = pMaterial;
            }
            // If found, the previously loaded material is used instead
            else {
                pMesh->m_pMaterial = it->second;
            }
        }

        // Buffer for the triangle indices:
        //   - each element in the buffer consists of three integers
        //     representing the three vertices of each triangle
        //     => size = 3*sizeof(unsigned int )
        //   - the number of elements corresponds to the number of
        //     triangles is indicesAccessor.count/3
        //
        // Summary: There are indicesAccessor.count of unsigned int
        vIndexBuffer.resize(indicesAccessor.count);
        copyIndicesFromArray(vIndexBuffer.data(), dataAddress, indicesAccessor.componentType, indicesAccessor.count);

        for (const auto &attribute : meshPrimitive.attributes)
        {
            const auto attribAccessor = gltfModel.accessors[static_cast<size_t>(attribute.second)];
            const auto& meshBufferView = gltfModel.bufferViews[static_cast<size_t>(attribAccessor.bufferView)];
            const auto& meshBuffer = gltfModel.buffers[static_cast<size_t>(meshBufferView.buffer)];
            const auto dataPtr = meshBuffer.data.data() + meshBufferView.byteOffset + attribAccessor.byteOffset;

            if (attribute.first == "POSITION")
            {
                if(attribAccessor.type != TINYGLTF_TYPE_VEC3)
                    throw NotImplementedError(getUuidAsString().append(": accessor data type not supported"));
                if(attribAccessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT)
                    throw NotImplementedError(getUuidAsString().append(": accessor component type not supported"));
                // 3D vector of float
                vVertexBuffer.resize(3*attribAccessor.count);
                std::memcpy(vVertexBuffer.data(), dataPtr, 3 * sizeof(float) * attribAccessor.count);
            }
            else if (attribute.first == "NORMAL")
            {
                if (attribAccessor.type != TINYGLTF_TYPE_VEC3)
                    throw NotImplementedError(getUuidAsString().append(": accessor data type not supported"));

                if (attribAccessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT)
                    throw NotImplementedError(getUuidAsString().append(": accessor component type not supported"));
                // 3D vector of float
                vNormalBuffer.resize(3*attribAccessor.count);
                std::memcpy(vNormalBuffer.data(), dataPtr, 3 * sizeof(float) * attribAccessor.count);
            }
            else if (attribute.first == "TANGENT")
            {
                if (attribAccessor.type != TINYGLTF_TYPE_VEC4)
                    throw NotImplementedError(getUuidAsString().append(": accessor data type not supported"));

                if (attribAccessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT)
                    throw NotImplementedError(getUuidAsString().append(": accessor component type not supported"));

                // 4D vector of float
                vTangentBuffer.resize(4*attribAccessor.count);
                std::memcpy(vTangentBuffer.data(), dataPtr, 4 * sizeof(float) * attribAccessor.count);
            }
            else if ((foundTC = attribute.first.find("TEXCOORD_")) != std::string::npos)
            {
                try
                {
                    auto channel = std::stoull(attribute.first.substr(foundTC + 9, std::to_string(MAX_UV_CHANNELS).length()));
                    if (channel < MAX_UV_CHANNELS)
                    {
                        // 2D vector of float
                        vTexCoordBuffer[channel].resize(2*attribAccessor.count);
                        std::memcpy(vTexCoordBuffer[channel].data(), dataPtr, 2 * sizeof(float) * attribAccessor.count);
                    }
                }
                catch (...)
                {
                    throw ParsingError(getUuidAsString().append(": accessor texture coordinate channel is not supported"));
                }
            }
        }
        vPrimitives.push_back(pMesh);
        pMesh->m_iId = static_cast<int>(m_vpPrimitives.size());
        m_vpPrimitives.push_back(pMesh);
    }
    return vPrimitives;
}

const Material* AssetGeometry::loadMaterial(const tinygltf::Model& gltfModel, int iMaterialIndex)
{
    auto jMaterial = gltfModel.j.at("materials")[static_cast<size_t>(iMaterialIndex)];

    // Checking type of material (either OpenMaterial or PBR)
    auto sMaterialType = checkMaterialType(jMaterial);
    if (sMaterialType == "openMaterial") {
        const ReferenceLink referenceLink = ReferenceLink(jMaterial);
        std::string sFilename = m_sDirectory + referenceLink.getUri();

        // Checking if the material was previously loaded (only for OpenMaterial materials here)
        for (auto &&pMaterial : m_vpMaterials) {
            if (sFilename == pMaterial->getFilename()) return pMaterial;
        }

        auto pMaterial = new AssetMaterial(sFilename);
        m_vpMaterials.push_back(pMaterial);
        return pMaterial;
    }
    else if (sMaterialType == "pbr") {
        auto pMaterial = new PbrMaterial(gltfModel, iMaterialIndex);
        m_vpMaterials.push_back(pMaterial);
        return pMaterial;
    }
    // Creating a "missing material" material (very bright pink material)
    else {
        std::cout << "Material with index " << iMaterialIndex << " does not exist in file " << m_sFilename << std::endl;
        std::cout << "HINT: if index is -1, it most likely means that there is no 'material' property in mesh primitive" << std::endl;
        std::cout << "Adding missing material indicator as material for primitive." << std::endl;
        if (!m_pMissingMaterial) m_pMissingMaterial = new PbrMaterial();
        return m_pMissingMaterial;
    }
}


PointLight *AssetGeometry::loadLight(const tinygltf::Model &gltfModel, int iLightIndex)
{
    auto pLight = new PointLight(gltfModel, iLightIndex);
    m_vpLights.push_back(pLight);
    return pLight;
}

void AssetGeometry::loadImages(const tinygltf::Model &gltfModel)
{
    size_t id = 0;
    m_vImages.reserve(gltfModel.images.size());
    std::transform(gltfModel.images.begin(), gltfModel.images.end(), std::back_inserter(m_vImages), [&id](const tinygltf::Image &i) {
        ImageData I;
        return (I.load(i, static_cast<int>(id++)), I);
    });
}


void AssetGeometry::fixNodeNumbers() noexcept
{
    size_t i = 0;
    for (Node *pNode : m_vpNodes) pNode->m_uNodeNumber = i++;
}

void AssetGeometry::addSceneNodes()
{
    m_viSceneNodes.clear();
    std::for_each(m_vpNodes.begin(), m_vpNodes.end(), [this](Node *pNode) {
        if (!pNode->hasParent()) {
            auto iNodeNum = static_cast<int>(pNode->m_uNodeNumber);
            if (std::find(m_viSceneNodes.begin(), m_viSceneNodes.end(), iNodeNum) == m_viSceneNodes.end())
                m_viSceneNodes.push_back(iNodeNum);
        }
    });
}

void AssetGeometry::free()
{
    // Free memory for the materials
    std::set<const Material*> spAssetMaterials(m_vpMaterials.begin(), m_vpMaterials.end());
    for (auto &&pMaterial : m_vpMaterials)
        if (spAssetMaterials.count(pMaterial)) delete pMaterial;

    // Delete missing material indicator 
    delete m_pMissingMaterial;

    // Free memory for meshes
    deleteContainerWithPointers(m_vpPrimitives);

    // Free memory for nodes
    deleteContainerWithPointers(m_vpNodes);

    // Free memory for lights
    deleteContainerWithPointers(m_vpLights);

    // Free imagers
    m_vImages.clear();
}
