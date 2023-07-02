#ifndef ACCELERATIONTREE_H
#define ACCELERATIONTREE_H

#include <vector>
#include "Utils.h"

struct Ray;
struct Intersection;
struct Triangle;

enum NodeType : uint16_t { INTERIOR, LEAF };

union NodeParams {
    int children[2]{-1, -1};
    std::vector<Triangle>* nodeTriangles;
};

struct NodeIndexBBoxPair {
    int32_t nodeIdx;
    BBox nodeBBox;
};

class AccelTree {
private:
    struct alignas(16) Node {
        Node(const int32_t _parentId, const int8_t _splitAxis, const NodeType _nodeType,
             const NodeParams _nodeParams);

        NodeParams params;
        int32_t parentId;
        int16_t splitAxis;
        NodeType type;
    };

public:
    AccelTree(const std::vector<Triangle>& _sceneTriangles, const BBox& _sceneBBox);

    ~AccelTree() { clearTree(); }

    bool intersect(const Ray& ray, const BBox& sceneBBox, Intersection& isectData) const;

    bool intersectPrim(const Ray& ray, const BBox& sceneBBox, Intersection& isectData) const;

private:
    void buildAccelTree(const int32_t parentIdx, const int32_t depth,
                        const std::vector<Triangle>& triangles, const BBox& nodeBBox);

    int32_t addNode(const Node& node);

    void clearTree();

private:
    std::vector<Node> nodes;
};

#endif  // !ACCELERATIONTREE_H
