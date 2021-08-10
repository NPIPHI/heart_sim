//
// Created by 16182 on 6/27/2021.
//

#include "LoadAssets.h"
#include <stb_image.h>
#include <stdexcept>
#include <fstream>
#include <tiny_obj_loader.h>

std::vector<char> LoadAssets::read_file(const char *file_name) {
    std::ifstream file(file_name, std::ios_base::ate | std::ios_base::binary);
    if(!file.is_open()){
        throw std::runtime_error("failed to open file");
    }
    size_t file_size = (size_t)file.tellg();
    file.seekg(0);
    std::vector<char> buffer(file_size);
    file.read(buffer.data(), file_size);

    return buffer;
}

std::tuple<uint32_t, uint32_t, std::vector<char>> LoadAssets::read_image(const char *filename) {
    int width, height, channels;
    auto pixels = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);
    size_t image_size = width * height * 4;
    if(!pixels){
        throw std::runtime_error("failed to load image");
    }
    std::vector<char> buffer(image_size);
    memcpy(buffer.data(), pixels, image_size);
    return {width, height, std::move(buffer)};
}

