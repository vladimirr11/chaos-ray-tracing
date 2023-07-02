/// Own includes
#include "AccelerationTree.h"
#include "Timer.h"

/// System headers
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <stack>

AccelTree::Node::Node(const int32_t _parentId, const int8_t _splitAxis, const NodeType _nodeType,
                      const NodeParams _nodeParams)
    : parentId(_parentId), splitAxis(_splitAxis), type(_nodeType) {
    if (type == INTERIOR) {
        params.children[0] = _nodeParams.children[0];
        params.children[1] = _nodeParams.children[1];
    } else
        params.nodeTriangles = _nodeParams.nodeTriangles;
};

AccelTree::AccelTree(const std::vector<Triangle>& _triangles, const BBox& _sceneBBox) {
    Timer timer;
    std::cout << "Start building acceleration tree...\n";
    timer.start();
    const int32_t rootIdx = addNode(Node{-1, -1, INTERIOR, NodeParams{}});
    buildAccelTree(rootIdx, 0, _triangles, _sceneBBox);
    std::cout << "Acceleration tree with " << nodes.size() << " nodes build for [" << std::fixed
              << std::setprecision(2) << Timer::toMilliSec<float>(timer.getElapsedNanoSec())
              << "ms]\n";
}

void AccelTree::buildAccelTree(const int32_t parentIdx, const int32_t treeDepth,
                               const std::vector<Triangle>& triangles, const BBox& nodeBBox) {
    if (treeDepth >= MAX_TREE_DEPTH || triangles.size() <= MAX_TRIANGLES_PER_NODE) {
        nodes[parentIdx].params.nodeTriangles = new std::vector<Triangle>(triangles.size());
        *nodes[parentIdx].params.nodeTriangles = triangles;
        nodes[parentIdx].type = LEAF;
        return;
    }

    const int16_t axis = (int16_t)(treeDepth % 3);
    const auto [leftChildBox, rightChildBox] = splitBBox(nodeBBox, axis);
    nodes[parentIdx].splitAxis = axis;

    std::vector<Triangle> leftChildTriangles, rightChildTriangles;
    leftChildTriangles.reserve(triangles.size());
    rightChildTriangles.reserve(triangles.size());
    for (const Triangle& triangle : triangles) {
        const BBox triangleBBox = getTriangleBBox(triangle);
        if (boxIntersect(leftChildBox, triangleBBox)) {
            leftChildTriangles.push_back(triangle);
        }
        if (boxIntersect(rightChildBox, triangleBBox)) {
            rightChildTriangles.push_back(triangle);
        }
    }

    if (leftChildTriangles.size() > 0) {
        const int32_t leftChildIdx = addNode(Node{parentIdx, -1, INTERIOR, NodeParams{}});
        nodes[parentIdx].params.children[0] = leftChildIdx;
        buildAccelTree(leftChildIdx, treeDepth + 1, leftChildTriangles, leftChildBox);
    }
    if (rightChildTriangles.size() > 0) {
        const int32_t rightChildIdx = addNode(Node{parentIdx, -1, INTERIOR, NodeParams{}});
        nodes[parentIdx].params.children[1] = rightChildIdx;
        buildAccelTree(rightChildIdx, treeDepth + 1, rightChildTriangles, rightChildBox);
    }
}

int32_t AccelTree::addNode(const Node& node) {
    const int32_t currTreeSize = (int32_t)nodes.size();
    nodes.push_back(node);
    return currTreeSize;
}

void AccelTree::clearTree() {
    std::for_each(nodes.begin(), nodes.end(), [](Node& node) {
        if (node.type == LEAF) {
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
            if (currNode.type == INTERIOR) {
                const auto [leftChildBox, rightChildBox] =
                    splitBBox(currNodeBBox, currNode.splitAxis);
                if (currNode.params.children[0] != -1) {
                    nodesStack.push({currNode.params.children[0], leftChildBox});
                }
                if (currNode.params.children[1] != -1) {
                    nodesStack.push({currNode.params.children[1], rightChildBox});
                }
            } else {
                const size_t currNodeTrianglesSize = currNode.params.nodeTriangles->size();
                if (currNodeTrianglesSize > 0) {
                    const auto nodeTriangles = currNode.params.nodeTriangles;
                    for (size_t i = 0; i < currNodeTrianglesSize; i++) {
                        if (nodeTriangles->at(i).intersectMT(ray, isectData)) {
                            if (isectData.t < closestPrim.t) {
                                closestPrim = isectData;
                            }
                            hasIntersect = true;
                        }
                    }
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
            if (currNode.type == INTERIOR) {
                const auto [leftChildBox, rightChildBox] =
                    splitBBox(currNodeBBox, currNode.splitAxis);
                if (currNode.params.children[0] != -1) {
                    nodesStack.push({currNode.params.children[0], leftChildBox});
                }
                if (currNode.params.children[1] != -1) {
                    nodesStack.push({currNode.params.children[1], rightChildBox});
                }
            } else {
                const size_t currNodeTrianglesSize = currNode.params.nodeTriangles->size();
                if (currNodeTrianglesSize > 0) {
                    const auto nodeTriangles = currNode.params.nodeTriangles;
                    for (size_t i = 0; i < currNodeTrianglesSize; i++) {
                        if (nodeTriangles->at(i).intersectMT(ray, isectData)) {
                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}
