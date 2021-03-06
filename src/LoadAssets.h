//
// Created by 16182 on 6/27/2021.
//

#ifndef VULKANNOTINCLUDED_LOADASSETS_H
#define VULKANNOTINCLUDED_LOADASSETS_H
#include<vector>
#include<tuple>
#include<cstdint>
#include "Vertex.h"

namespace LoadAssets {
    [[nodiscard]] std::vector<char> read_file(const char *file_name);
    [[nodiscard]] std::tuple<uint32_t, uint32_t, std::vector<char>> read_image(const char *file_name);
    [[nodiscard]] std::pair<std::vector<Vertex>, std::vector<uint32_t>> read_model(const char * file_name);
};


#endif //VULKANNOTINCLUDED_LOADASSETS_H
