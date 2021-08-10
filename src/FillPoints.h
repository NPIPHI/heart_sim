//
// Created by 16182 on 8/8/2021.
//

#ifndef HEART_SIM_FILLPOINTS_H
#define HEART_SIM_FILLPOINTS_H

#include <stl_reader.h>
#include <vector>
#include <GLM/vec3.hpp>
namespace FillPoints {
    std::vector<glm::vec3> random_fill(stl_reader::StlMesh<float, uint32_t> const& mesh);
};


#endif //HEART_SIM_FILLPOINTS_H
