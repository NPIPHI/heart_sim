//
// Created by 16182 on 8/2/2021.
//

#ifndef HEART_SIM_NODEGRAPH_H
#define HEART_SIM_NODEGRAPH_H

#include<span>
#include<vector>
#include<array>
#include "Vertex.h"

class NodeGraph {
public:
    static constexpr int neighbor_count = 8;
    NodeGraph(){};
    NodeGraph(std::span<const Vertex> vertices);

    std::vector<std::array<uint32_t, neighbor_count>> edges;
};


#endif //HEART_SIM_NODEGRAPH_H
