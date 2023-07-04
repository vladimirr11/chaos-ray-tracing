/// Own includes
#include "AccelerationTree.h"
#include "Timer.h"

/// System headers
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <stack>

struct NodeIndexBBoxPair {
    int32_t nodeIdx;
    BBox nodeBBox;
};

struct PrimBounds {
    enum BoundType { Min, Max };

    float bound = Infinity;
    BoundType type;
};

AccelTree::Node::Node(const int32_t _parentId, const int8_t _splitAxis, const NodeType _nodeType,
                      const NodeParams _nodeParams)
    : parentId(_parentId), splitAxis(_splitAxis), type(_nodeType) {
    if (type == Interior) {
        params.children[0] = _nodeParams.children[0];
        params.children[1] = _nodeParams.children[1];
    } else
        params.nodeTriangles = _nodeParams.nodeTriangles;
};

bool AccelTree::Node::intersect(const Ray& ray, Intersection& isectData) const {
    Intersection closestPrim;
    bool hasIntersect = false;
    if (params.nodeTriangles->size() > 0) {
        const auto nodeTriangles = params.nodeTriangles;
        for (auto it = nodeTriangles->begin(); it != nodeTriangles->end(); ++it) {
            if (it->intersectMT(ray, isectData)) {
                if (isectData.t < closestPrim.t)
                    closestPrim = isectData;
                hasIntersect = true;
            }
        }
    }

    if (hasIntersect)
        isectData = closestPrim;

    return hasIntersect;
}

bool AccelTree::Node::intersectPrim(const Ray& ray, Intersection& isectData) const {
    if (params.nodeTriangles->size() > 0) {
        const auto nodeTriangles = params.nodeTriangles;
        for (auto it = nodeTriangles->begin(); it != nodeTriangles->end(); ++it) {
            if (it->intersectMT(ray, isectData))
                return true;
        }
    }
    return false;
}

AccelTree::AccelTree(const std::vector<Triangle>& triangles, const BBox& sceneBBox) {
    Timer timer;
    std::cout << "Start building acceleration tree...\n";
    timer.start();
    // compute AABB for each triangle in the scene
    std::vector<BBox> trianglesBBoxes;
    trianglesBBoxes.reserve(triangles.size());
    for (size_t i = 0; i < triangles.size(); i++) {
        trianglesBBoxes.emplace_back(getTriangleBBox(triangles[i]));
    }
    const int32_t rootIdx = addNode(Node{-1, -1, Interior, NodeParams{}});
    // recursively build the tree
    buildAccelTree(rootIdx, 0, triangles, trianglesBBoxes, sceneBBox);
    std::cout << "Acceleration tree with " << nodes.size() << " nodes build for [" << std::fixed
              << std::setprecision(2) << Timer::toMilliSec<float>(timer.getElapsedNanoSec())
              << "ms]\n";
}

void AccelTree::buildAccelTree(const int32_t parentIdx, const int32_t treeDepth,
                               const std::vector<Triangle>& triangles,
                               const std::vector<BBox>& trianglesBBoxes, const BBox& nodeBBox) {
    // if conditions met initialize leaf node
    if (treeDepth >= MAX_TREE_DEPTH || triangles.size() <= MAX_TRIANGLES_PER_NODE) {
        nodes[parentIdx].params.nodeTriangles = new std::vector<Triangle>(triangles.size());
        *nodes[parentIdx].params.nodeTriangles = triangles;
        nodes[parentIdx].type = Leaf;
        return;
    }

    // find axis and position for splitting of interior node
    int8_t axis = -1;
    float splitPos = Infinity;
    switch (splitMethod) {
        case SplitMethod::Middle: {  // split axis in middle
            axis = (int8_t)(treeDepth % 3);
            splitPos = (nodeBBox.min[axis] + nodeBBox.max[axis]) * 0.5f;
            nodes[parentIdx].splitAxis = axis;
            nodes[parentIdx].splitPos = splitPos;
            break;
        }
        case SplitMethod::SAH: {                      // the SAH approach used in pbrt
            axis = (uint8_t)findMaxExtent(nodeBBox);  // choose the longest axis for split
            const float isectCost = 60.0f;

            // populate the minimum and maximum extent of each triangle for the choosen axis
            std::vector<PrimBounds> trianglesBounds;
            trianglesBounds.reserve(trianglesBBoxes.size() * 2);
            for (size_t i = 0; i < trianglesBBoxes.size(); ++i) {
                trianglesBounds.push_back(
                    PrimBounds{trianglesBBoxes[i].min[axis], PrimBounds::Min});
                trianglesBounds.push_back(
                    PrimBounds{trianglesBBoxes[i].max[axis], PrimBounds::Max});
            }

            // sort the bounds
            std::sort(
                trianglesBounds.begin(), trianglesBounds.end(),
                [](const PrimBounds& pb0, const PrimBounds& pb1) { return pb0.bound < pb1.bound; });

            const Vector3f nodeDiagonal = nodeBBox.max - nodeBBox.min;
            const float nodeSurfArea =
                2 * (nodeDiagonal.x * nodeDiagonal.y + nodeDiagonal.x * nodeDiagonal.z +
                     nodeDiagonal.y * nodeDiagonal.z);
            const float invNodeSurfArea = 1.f / nodeSurfArea;
            float bestSplitCost = 1e30;
            int32_t bestOffset = -1;
            const float oldCost = isectCost * triangles.size();

            // compute the cost of all possible splits for the chosen axis to find the best
            int32_t lowerBoundPrims = 0, upperBoundPrims = (int32_t)triangles.size();
            for (size_t i = 0; i < triangles.size() * 2; ++i) {
                if (trianglesBounds[i].type == PrimBounds::Max)
                    --upperBoundPrims;
                const float currBound = trianglesBounds[i].bound;
                if (currBound > nodeBBox.min[axis] && currBound < nodeBBox.max[axis]) {
                    int otherAxis0 = (axis + 1) % 3, otherAxis1 = (axis + 2) % 3;
                    float lowerBoundSA =
                        2 * (nodeDiagonal[otherAxis0] * nodeDiagonal[otherAxis1] +
                             (currBound - nodeBBox.min[axis]) *
                                 (nodeDiagonal[otherAxis0] + nodeDiagonal[otherAxis1]));
                    float upperBoundSA =
                        2 * (nodeDiagonal[otherAxis0] * nodeDiagonal[otherAxis1] +
                             (nodeBBox.max[axis] - currBound) *
                                 (nodeDiagonal[otherAxis0] + nodeDiagonal[otherAxis1]));
                    float probLowerBound = lowerBoundSA * invNodeSurfArea;
                    float probUpperBound = upperBoundSA * invNodeSurfArea;
                    float splitCost = isectCost * (probLowerBound * lowerBoundPrims +
                                                   probUpperBound * upperBoundPrims);
                    if (splitCost < bestSplitCost) {
                        bestSplitCost = splitCost;
                        bestOffset = (int32_t)i;
                    }
                }
                if (trianglesBounds[i].type == PrimBounds::Min)
                    ++lowerBoundPrims;
            }
            Assert(lowerBoundPrims == (int32_t)triangles.size() && upperBoundPrims == 0);

            // initialize leaf node if no good split were found
            if (bestSplitCost > 4 * oldCost) {
                nodes[parentIdx].params.nodeTriangles = new std::vector<Triangle>(triangles.size());
                *nodes[parentIdx].params.nodeTriangles = triangles;
                nodes[parentIdx].type = Leaf;
                return;
            }

            // set the found axis and split position for the current interior node
            splitPos = trianglesBounds[bestOffset].bound;
            nodes[parentIdx].splitAxis = axis;
            nodes[parentIdx].splitPos = splitPos;
            break;
        }
        default:
            Assert(false && "Received unsupported split method.");
    }

    // split the current node
    const auto [leftChildBox, rightChildBox] = splitBBox(nodeBBox, axis, splitPos);

    // allocate storage for the triangles and AABB of both children nodes
    std::vector<Triangle> leftChildTriangles, rightChildTriangles;
    std::vector<BBox> leftChildBBoxes, rightChildBBoxes;

    leftChildTriangles.reserve(triangles.size());
    leftChildBBoxes.reserve(triangles.size());
    rightChildTriangles.reserve(triangles.size());
    rightChildBBoxes.reserve(triangles.size());

    // populate the triangles and AABB for the left and right child nodes
    for (size_t i = 0; i < triangles.size(); ++i) {
        const BBox triangleBBox = trianglesBBoxes[i];
        if (boxIntersect(leftChildBox, triangleBBox)) {
            leftChildTriangles.push_back(triangles[i]);
            leftChildBBoxes.push_back(triangleBBox);
        }
        if (boxIntersect(rightChildBox, triangleBBox)) {
            rightChildTriangles.push_back(triangles[i]);
            rightChildBBoxes.push_back(triangleBBox);
        }
    }

    // recursively initialize left and right child nodes
    if (leftChildTriangles.size() > 0) {
        const int32_t leftChildIdx = addNode(Node{parentIdx, -1, Interior, NodeParams{}});
        nodes[parentIdx].params.children[0] = leftChildIdx;
        buildAccelTree(leftChildIdx, treeDepth + 1, leftChildTriangles, leftChildBBoxes,
                       leftChildBox);
    }
    if (rightChildTriangles.size() > 0) {
        const int32_t rightChildIdx = addNode(Node{parentIdx, -1, Interior, NodeParams{}});
        nodes[parentIdx].params.children[1] = rightChildIdx;
        buildAccelTree(rightChildIdx, treeDepth + 1, rightChildTriangles, rightChildBBoxes,
                       rightChildBox);
    }
}

int32_t AccelTree::addNode(const Node& node) {
    const int32_t currTreeSize = (int32_t)nodes.size();
    nodes.push_back(node);
    return currTreeSize;
}

void AccelTree::clearTree() {
    std::for_each(nodes.begin(), nodes.end(), [](Node& node) {
        if (node.type == Leaf) {
            node.params.nodeTriangles->clear();
            delete node.params.nodeTriangles;
        }
    });
}

bool AccelTree::intersect(const Ray& ray, const BBox& sceneBBox, Intersection& isectData) const {
    std::stack<NodeIndexBBoxPair> nodesStack;
    nodesStack.push({0, sceneBBox});
    bool hasIntersect = false;
    Intersection closestPrim;
    while (!nodesStack.empty()) {
        const auto [currNodeIdx, currNodeBBox] = std::move(nodesStack.top());
        nodesStack.pop();
        const Node& currNode = nodes[currNodeIdx];
        if (currNodeBBox.intersect(ray)) {
            if (currNode.type == Interior) {  // stack interior nodes for traversal
                const auto [leftChildBox, rightChildBox] =
                    splitBBox(currNodeBBox, currNode.splitAxis, currNode.splitPos);
                if (currNode.params.children[0] != -1)
                    nodesStack.push({currNode.params.children[0], leftChildBox});
                if (currNode.params.children[1] != -1)
                    nodesStack.push({currNode.params.children[1], rightChildBox});
            } else {  // search for the closest intersection with the leaf's triangles
                bool currNodeIntersect = currNode.intersect(ray, isectData);
                if (currNodeIntersect) {
                    if (isectData.t < closestPrim.t)
                        closestPrim = isectData;
                    hasIntersect = true;
                }
            }
        }
    }

    if (hasIntersect)
        isectData = closestPrim;

    return hasIntersect;
}

bool AccelTree::intersectPrim(const Ray& ray, const BBox& sceneBBox,
                              Intersection& isectData) const {
    std::stack<NodeIndexBBoxPair> nodesStack;
    nodesStack.push({0, sceneBBox});
    while (!nodesStack.empty()) {
        const auto [currNodeIdx, currNodeBBox] = std::move(nodesStack.top());
        nodesStack.pop();
        const Node& currNode = nodes[currNodeIdx];
        if (currNodeBBox.intersect(ray)) {
            if (currNode.type == Interior) {  // stack interior nodes for traversal
                const auto [leftChildBox, rightChildBox] =
                    splitBBox(currNodeBBox, currNode.splitAxis, currNode.splitPos);
                if (currNode.params.children[0] != -1) {
                    nodesStack.push({currNode.params.children[0], leftChildBox});
                }
                if (currNode.params.children[1] != -1) {
                    nodesStack.push({currNode.params.children[1], rightChildBox});
                }
            } else {  // verify for intersection with the leaf's triangles
                return currNode.intersectPrim(ray, isectData);
            }
        }
    }

    return false;
}
