//
// Copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
//

/// @file      Node.h
/// @author    Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @copyright (C) 2019-2023, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
/// @date      2019-09-25
/// @brief     Support for nodes

#ifndef NODE_H
#define NODE_H

#include "AssetInfo.h"
#include "Mesh.h"

/**
 * @brief Class @c Node provides geometry tree-node storage with all
 * additional information on dependent meshes and transformations.
 */
struct Node final
{
    /// @brief Name of node.
    std::string m_sName;
    /// @brief @a URI of @a glTF referenced in node.
    std::string m_sUri;
    /// @brief Node category.
    std::string m_sCategory;
    /// @brief Node @a UUID ( @c std::string ).
    std::string m_sId;
    /// @brief Node asset info.
    AssetInfo* m_pAssetInfo = nullptr;
    /// @brief Number of node / node index.
    size_t m_uNodeNumber;
    /// @brief Transformation of the node.
    fmat4 m_Transformation;
    /// @brief Pointer to @a parent node (may be @c nullptr if node has no parent).
    const Node *m_pParent = nullptr;
    /// @brief List of @a children (list might be empty if node has no children).
    std::vector<Node *> m_vpChildren;
    /// @brief List of @a primitives.
    std::vector<Mesh *> m_vpPrimitives;

public:
    /// @publicsection Default methods and basic interface (e.g. @a -ctors).

    /// @brief Create new @c Node with node number @p uNodeNumber and name given by @p crsName .
    explicit Node(size_t uNodeNumber, std::string crsName = "") :
        m_sName(std::move(crsName)), m_sId(Uuid{}.toString()), m_uNodeNumber(uNodeNumber)
    {
        assignIdentityMatrix(m_Transformation);
    }

    /// @brief Copy -ctor.
    Node(const Node& other)
    {
        copyFrom(other);
    }
    /// @brief Copy -ment.
    Node &operator=(const Node& other)
    {
        copyFrom(other);
        return *this;
    }

    BDPT_DEFINE_MOVE_MODE(Node, default)

    /// @brief -Dtor.
    ~Node() noexcept { delete m_pAssetInfo; }

    /// @brief Adds @c Node @p pChild as a child.
    void addChild(Node *pChild)
    {
        m_vpChildren.push_back(pChild);
    }
    /// @brief Adds primitives to node primitives vector.
    void addPrimitives(std::vector<Mesh *> vpPrimitives)
    {
        m_vpPrimitives.insert(m_vpPrimitives.end(), vpPrimitives.begin(), vpPrimitives.end());
    }

    /// @brief Checks whether current node has children.
    bool hasChildren() const noexcept { return !m_vpChildren.empty(); }
    /// @brief Checks whether current node has a parent. 
    bool hasParent() const noexcept { return m_pParent != nullptr; }
    /// @brief Determines whether node has a specific descent.
    /// Returns @c true if the parent or any other descent of the node has the
    /// node number giben by @p uNodeNumber , Otherwise returns @c false .
    bool hasParent(size_t uNodeNumber) const noexcept
    {
        const Node *pNode = this;
        while ((pNode = pNode->m_pParent))
            if (pNode->m_uNodeNumber == uNodeNumber)
                return true;
        return false;
    }

public:
    /// @publicsection Getters and setters.

    /// @brief Sets node @a UUID to @p sId .
    void setNodeId(const std::string &sId)
    {
        Uuid id(sId);
        m_sId = id.toString();
    }

    /// @brief Returns all descendants of a node.
    std::vector<Node *> getAllDescendants()
    {
        std::vector<Node *> vDescendants = m_vpChildren;

        std::for_each(m_vpChildren.begin(), m_vpChildren.end(), [&vDescendants](Node *child) {
            if (child->hasChildren()) {
                auto descendants = child->getAllDescendants();
                vDescendants.insert(vDescendants.end(), descendants.begin(), descendants.end());
            }
        });

        return vDescendants;
    }

    /**
     * @brief Computes @a minimum axis-aligned bounding box of the node.
     *
     * @details This method computes the @a minimum axis-aligned bounding box for
     * current node. Due to the hierarchical structure of nodes and as nodes can have
     * transformations, this method needs to iterate over each vertex belonging
     * to this node and transfer the vertex point to world coordinate. For this
     * reason, this method is rather @b expensive to call.
     */
    BoundingBox<float> getBBox() const
    {
        BoundingBox<float> BB;
        fmat4 M;
        hasParent() ? m_pParent->getGlobalTransformation(M) : assignIdentityMatrix(M);
        getBBoxRecursive(BB, M);
        return BB;
    }

    /// @brief Sets transformation of node to @p TM .
    void setTransformation(fmat4 TM)
    {
        std::memcpy(m_Transformation, TM, sizeof(fmat4));
    }
    /// @brief Writes a @a global transformation of the node to @p gT .
    void getGlobalTransformation(fmat4 gT) const
    {
        fmat4 pT;
        hasParent() ? m_pParent->getGlobalTransformation(pT) : assignIdentityMatrix(pT);
        multSquareMatrix(pT, m_Transformation, gT);
    }

private:
    /// @privatesection Helping interface.

    void getBBoxRecursive(BoundingBox<float> &BB, const fmat4 extM) const
    {
        fmat4 M;
        multSquareMatrix(extM, m_Transformation, M);

        // If this node contains a mesh, compute the bbox of the mesh
        std::for_each(m_vpPrimitives.begin(), m_vpPrimitives.end(), std::bind(&Mesh::getBBox, _1, std::ref(BB), M));

        // Consider children
        std::for_each(m_vpChildren.begin(), m_vpChildren.end(), std::bind(&Node::getBBoxRecursive, _1, std::ref(BB), M));
    }

    void copyFrom(const Node& other)
    {
        // All of these are copied from the other Node
        this->m_sName = other.m_sName;
        this->m_sUri = other.m_sUri;
        this->m_sCategory = other.m_sCategory;
        this->m_pAssetInfo = other.m_pAssetInfo;
        std::memcpy(this->m_Transformation, other.m_Transformation, sizeof(fmat4));
        this->m_uNodeNumber = other.m_uNodeNumber;  
        this->m_vpPrimitives = other.m_vpPrimitives;

        // Add the copied node to primitive nodes
        std::for_each(this->m_vpPrimitives.begin(), this->m_vpPrimitives.end(), std::bind(&Mesh::addParentNode, _1, this));

        // Generate new UUID for node
        this->m_sId = Uuid{}.toString();

        // This is set later, after copying
        this->m_pParent = nullptr;

        std::vector<Node*> copiedChildren;
        copiedChildren.reserve(other.m_vpChildren.size());
        std::transform(other.m_vpChildren.begin(), other.m_vpChildren.end(), std::back_inserter(this->m_vpChildren), [this](const Node *child) {
            auto childCopy = new Node(*child);
            childCopy->m_pParent = this;
            return childCopy;
        });
    }
};

#endif // NODE_H
