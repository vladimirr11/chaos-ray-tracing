#ifndef ACCELERATIONTREE_H
#define ACCELERATIONTREE_H

#include <vector>
#include "Utils.h"

struct Ray;
struct Intersection;
struct Triangle;

enum NodeType { Interior, Leaf };
enum class SplitMethod { Middle, SAH };

/// @brief Shared storage for interior and leaf nodes
union NodeParams {
    int children[2]{-1, -1};
    std::vector<Triangle>* nodeTriangles;
};

class AccelTree {
private:
    struct alignas(32) Node {
        Node(const int32_t _parentId, const int8_t _splitAxis, const NodeType _nodeType,
             const NodeParams _nodeParams);

        bool intersect(const Ray& ray, Intersection& isectData) const;

        bool intersectPrim(const Ray& ray, Intersection& isectData) const;

        NodeParams params;
        int32_t parentId;
        int32_t splitAxis;
        NodeType type;
        float splitPos = Infinity;
    };

public:
    AccelTree(const std::vector<Triangle>& sceneTriangles, const BBox& sceneBBox);

    ~AccelTree() { clearTree(); }

    bool intersect(const Ray& ray, const BBox& sceneBBox, Intersection& isectData) const;

    bool intersectPrim(const Ray& ray, const BBox& sceneBBox, Intersection& isectData) const;

private:
    void buildAccelTree(const int32_t parentIdx, const int32_t depth,
                        const std::vector<Triangle>& triangles,
                        const std::vector<BBox>& trianglesBBoxes, const BBox& nodeBBox);

    int32_t addNode(const Node& node);

    void clearTree();

private:
    std::vector<Node> nodes;  ///< Flattened nodes of the acceleration tree
    const SplitMethod splitMethod = SplitMethod::SAH;  ///< Split method used to build the tree
};

#endif  // !ACCELERATIONTREE_H
