//
// Created by 16182 on 6/27/2021.
//

#include "LoadAssets.h"
#include <stb_image.h>
#include <stdexcept>
#include <fstream>
#include <stl_reader.h>

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

std::tuple<uint32_t, uint32_t, std::vector<char>> LoadAssets::read_image(const char *file_name) {
    int width, height, channels;
    auto pixels = stbi_load(file_name, &width, &height, &channels, STBI_rgb_alpha);
    size_t image_size = width * height * 4;
    if(!pixels){
        throw std::runtime_error("failed to load image");
    }
    std::vector<char> buffer(image_size);
    memcpy(buffer.data(), pixels, image_size);
    return {width, height, std::move(buffer)};
}

std::pair<std::vector<Vertex>, std::vector<uint32_t>> LoadAssets::read_model(const char *file_name) {
    stl_reader::StlMesh<float, uint32_t> mesh(file_name);

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    for(size_t i = 0; i < mesh.num_vrts(); i++){
        float x = mesh.raw_coords()[i*3];
        float y = mesh.raw_coords()[i*3+1];
        float z = mesh.raw_coords()[i*3+2];
        vertices.push_back({{x,y,z,1}, {}});
    }

    for(size_t i = 0; i < mesh.num_tris(); i++){
        indices.push_back(mesh.raw_tris()[i*3]);
        indices.push_back(mesh.raw_tris()[i*3+1]);
        indices.push_back(mesh.raw_tris()[i*3+2]);
    }

    //reconstruct vertex normals by averaging connected triangle normals
    for(size_t tri = 0; tri < mesh.num_tris(); tri++){
        glm::vec4 normal = {
                mesh.tri_normal(tri)[0],
                mesh.tri_normal(tri)[1],
                mesh.tri_normal(tri)[2],
                1
        };
        vertices[mesh.raw_tris()[tri*3]].normal += normal;
        vertices[mesh.raw_tris()[tri*3+1]].normal += normal;
        vertices[mesh.raw_tris()[tri*3+2]].normal += normal;
    }

    for(size_t v = 0; v < mesh.num_vrts(); v++){
        vertices[v].normal /= vertices[v].normal.w;
        vertices[v].normal.w = 0;
    }

    return {std::move(vertices), std::move(indices)};
}

