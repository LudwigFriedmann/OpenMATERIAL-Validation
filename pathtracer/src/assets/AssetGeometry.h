//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      AssetGeometry.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-25
/// @brief     AssetGeometry class

#ifndef ASSETGEOMETRY_H
#define ASSETGEOMETRY_H

#include <map>
#include <vector>

#include "AssetInfo.h"
#include "ImageData.h"
#include "Node.h"
#include "PointLight.h"

/**
 * @brief Class for assets of type geometry.
 *
 * @details This class tries to be as close to the idea of the @a glTF standard as
 * possible. From a @a glTF file the active scene is loaded. A scene consists
 * of one or more nodes. The node structure is hierarchical and corresponds to
 * a tree. Each node can define a local coordinate system and have zero or one
 * meshes of type @c Mesh . A mesh contains the vertices of the triangles and is
 * associated with a material of type @c AssetMaterial .
 */
struct AssetGeometry final : public AssetInfo
{
    /// @brief Filename of the asset (for error messages).
    std::string m_sFilename;
    /// @brief Directory containing the asset (necessary to resolve @a OpenMaterial_reference_link links).
    std::string m_sDirectory;

    /// @brief Vector of the scene nodes.
    std::vector<int> m_viSceneNodes;
    /// @brief Vector of pointers to all nodes.
    std::vector<Node *> m_vpNodes;
    /// @brief Vector of pointers to all primitives.
    std::vector<Mesh *> m_vpPrimitives;
    /// @brief Vector of pointers to all materials.
    std::vector<const Material *> m_vpMaterials;
    /// @brief Pointer to @a missing-material material.
    const Material *m_pMissingMaterial = nullptr;
    /// @brief Vector of pointers to all lights.
    std::vector<PointLight *> m_vpLights;
    /// @brief Vector of loaded image data.
    std::vector<ImageData> m_vImages;   

public:
    /// @publicsection Static loading options.

    /// @brief Variable for defining what material type has priority when loading.
    static std::string m_sMaterialLoadingPriority;
    /// @brief Enables light loading.
    static bool m_bUseLights;

public:
    /// @publicsection Basic interface (e.g. @a -ctors).

    /// @brief Creates a new @c AssetGeometry object out of @a glTF model @p gltfModel stored
    /// in directory @p crsDirectory .
    explicit AssetGeometry(const tinygltf::Model &gltfModel, const std::string &crsDirectory = "");
    /// @brief Creates a new @c AssetGeometry from @a glTF file @p crsFilename .
    explicit AssetGeometry(const std::string &crsFilename);
    /// @brief -Dtor.
    ~AssetGeometry() override;

    /**
     * @brief Computes the @a bounding-box of this asset.
     *
     * @details The minimum axis-aligned bounding box in world coordinates is returned.
     * As nodes are hierarchical and contain transformations, this method has to iterate
     * over every vertex point, transform the vertex point to world coordinates, and
     * thereby compute the bounding box. This method is rather @b expensive to call.
     *
     * @return Minimum bounding box of type @c BoundingBox<float> .
     */
    BoundingBox<float> getBBox() const;

    /// @brief Utility function, which loads the parameters contained in @a glTF file @p crsFilename 
    /// and returns corresponding @c tinygltf::Model object.
    static tinygltf::Model loadGltfModel(const std::string &crsFilename);

    /// @brief Utility function, which checks type of material for @a OpenMaterial_reference_link
    /// and @a pbrMetallicRoughness extensions in the given @a json-file  @p j . Returns either
    /// @a "openMaterial", @a "pbr" respectively or @a "unknown" for any other extension.
    static std::string checkMaterialType(const nlohmann::json &j);

private:
    /// @privatesection Helping interface.

    /// @brief Map of loaded geometry references.
    std::map<std::string, std::vector<Node *>> m_mLoadedFiles;

    /**
     * @brief Loads asset info data from main (or only) @a glTF model @p gltfModel .
     */
    void loadAssetInfoData(const tinygltf::Model &gltfModel);
    /**
     * @brief Loads asset info data from main (or only) @a glTF file @p jsonDoc .
     * @overload loadAssetInfoData(const tinygltf::Model &)
     */
    void loadAssetInfoData(const nlohmann::json &jsonDoc);

    /** 
     * @brief Loads nodes, meshes and materials from the provided @a glTF @p sFilename and all
     * referenced @a glTFs files into @c AssetGeometry object.
     * 
     * @details This function will recursively go through all the referenced @a glTFs, starting at
     * the one passed as @p sFilename . Additional data is loaded from @a glTF model @p gltfModel .
     * 
     * @param sFilename     Filename (path) to @a glTF file;
     * @param gltfModel     @a glTF model;
     * @param pParentNode   Pointer to the node that held the reference link to current @a glTF, @c nullptr by default;
     */
    void loadGltf(std::string sFilename, const tinygltf::Model &gltfModel, Node *pParentNode = nullptr);
    /** 
     * @brief Loads nodes, meshes and materials from the provided @a glTF model @p gltfModel and all
     * referenced @a glTFs files into @c AssetGeometry object out of directory path @p crsDirectory
     * ( @b without the name of file itself).
     * @overload loadGltf(std::string, const tinygltf::Model &, Node *)
     */
    void loadGltf(const tinygltf::Model &gltfModel, const std::string &crsDirectory = "", Node *pParentNode = nullptr);

    /// @brief Loads all data from the provided @a glTF model @p gltfModel stored in directory
    /// @p crsDirectory and returns vector of nodes processed in current @a glTF.
    std::vector<Node *> loadData(const tinygltf::Model &gltfModel, std::string crsDirectory = "");
    /// @brief Hierarchically loads all nodes from the the @a glTF model @p gltfModel and returns
    /// vector of nodes processed in current @a glTF.
    std::vector<Node *> loadNodes(const tinygltf::Model &gltfModel);
    /**
     * @brief Loads a single @a glTF mesh from the @a glTF model @p gltfModel with index @p iMeshIndex .
     * 
     * @param gltfModel         @a glTF model;
     * @param iMeshIndex        Index of the mesh in @a glTF;
     * @param mLoadedMaterials  Map of loaded materials and their indices (used for checking
     * if a certain material was loaded previously);
     * 
     * @return Vector of all associated meshes (i.e. @c Mesh ).
     */
    std::vector<Mesh *> loadMesh(const tinygltf::Model &gltfModel, int iMeshIndex, std::map<int, const Material *> &mLoadedMaterials);
    /// @brief Loads a single @a glTF material from the @a glTF model @p gltfModel with index @p iMaterialIndex
    /// and returns either the pointer to a newly created material or a pointer to the @a missing-material material.
    const Material *loadMaterial(const tinygltf::Model &gltfModel, int iMaterialIndex);
    /// @brief Loads a single @a glTF light ( @a KHR_lights_punctual extension) from the @a glTF model @p gltfModel
    /// with index @p iLightIndex and returns the pointer to a newly created point light @c PointLight .
    PointLight *loadLight(const tinygltf::Model &gltfModel, int iLightIndex);
    /// @brief Loads all @a image data from the provided @a glTF model @p gltfModel .
    void loadImages(const tinygltf::Model &gltfModel);

    /// @brief Fixes node numbers to correspond to their index in the nodes array.
    void fixNodeNumbers() noexcept;
    /// @brief Adds all nodes without parents to scene nodes.
    void addSceneNodes();

    /// @brief Frees allocated memory.
    void free();
};


/**
 * @brief Performs a transformation @p func over the node @p node with a 4D
 * transormation matrix @p T and recursively iterates over all its children nodes.
 * 
 * @details Before @p func is invoked, an external transformation @p T is first applied
 * to the node transformation and then the resulting transformation is passed to @p func .
 * 
 * @tparam Func A binary function which takes a pointer to @c Node as the first positional
 * argument and @c const @c fmat4 as a second one. Return value is @b disregarded ;
 * @param node  Pointer to geometry node of type @c Node ;
 * @param T     4D transformation matrix ;
 * @param func  An invokable object of type @c Func ;
 */
template<typename Func>
void iterateAssetGeometryNodes(Node *node, const fmat4 T, Func func)
{
    fmat4 nT;
    multSquareMatrix(T, node->m_Transformation, nT);
    BDPT_UNUSED(func(node, nT));

    for (auto &&child : node->m_vpChildren) iterateAssetGeometryNodes(child, nT, func);
}

/**
 * @brief Iterates over all children nodes ( @c Node ) of @p scene and performs
 * @ref iterateAssetGeometryNodes(Node *, const fmat4, Func) to each with transformation
 * @p T and invokable object @p func .
 * @overload void iterateAssetGeometryNodes(Node *, const fmat4, Func)
 */
template<typename Func>
void iterateAssetGeometryNodes(AssetGeometry *scene, const fmat4 T, Func func)
{
    for (int i : scene->m_viSceneNodes)
        iterateAssetGeometryNodes(scene->m_vpNodes[static_cast<size_t>(i)], T, func);
}

#endif // !ASSETGEOMETRY_H
