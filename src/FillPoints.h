//
// Created by 16182 on 8/8/2021.
//

#ifndef HEART_SIM_FILLPOINTS_H
#define HEART_SIM_FILLPOINTS_H

#include<span>
#include <vector>
#include "Vertex.h"
#include <glm/vec3.hpp>
namespace FillPoints {
    std::vector<glm::vec3> random_fill(std::span<Vertex> vertices, std::span<uint32_t> indices, float density);
};


#endif //HEART_SIM_FILLPOINTS_H
