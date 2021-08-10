//
// Created by 16182 on 8/8/2021.
//

#include "FillPoints.h"
#include <CGAL/Simple_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>
#include<random>

#include<chrono>
std::vector<glm::vec3> FillPoints::random_fill(const stl_reader::StlMesh<float, uint32_t> &mesh) {
    using K = CGAL::Simple_cartesian<float>;
    using Ray = K::Ray_3;
    using Line = K::Line_3;
    using Segment = K::Segment_3;
    using Point = K::Point_3;
    using Triangle = K::Triangle_3;
    using Primitive = CGAL::AABB_triangle_primitive<K, std::vector<Triangle>::iterator>;
    using Tree = CGAL::AABB_tree<CGAL::AABB_traits<K, Primitive>>;

    std::vector<Triangle> triangles;
    auto to_tri = [&](size_t idx){
        auto tris = mesh.raw_tris();
        auto cords = mesh.raw_coords();
        Point p1 = {
                cords[tris[idx*3+0]*3+0],
                cords[tris[idx*3+0]*3+1],
                cords[tris[idx*3+0]*3+2]
        };

        Point p2 = {
                cords[tris[idx*3+1]*3+0],
                cords[tris[idx*3+1]*3+1],
                cords[tris[idx*3+1]*3+2]
        };

        Point p3 = {
                cords[tris[idx*3+2]*3+0],
                cords[tris[idx*3+2]*3+1],
                cords[tris[idx*3+2]*3+2]
        };
        return Triangle{p1, p2, p3};
    };
    for(size_t triangle_index = 0; triangle_index < mesh.num_tris(); triangle_index++){
        triangles.push_back(to_tri(triangle_index));
    }

    auto t1 = std::chrono::steady_clock::now();
    Tree tree{triangles.begin(),  triangles.end()};
    tree.build();
    std::vector<glm::vec3> points;
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<float> dis(-0.6, 0.6);
    for(float x = tree.bbox().xmin(); x < tree.bbox().xmax(); x+= 1){
        for(float y = tree.bbox().ymin(); y < tree.bbox().ymax(); y += 1){
            for(float z = tree.bbox().zmin(); z < tree.bbox().zmax(); z += 1){
                    float rx = x + dis(gen);
                    float ry = y + dis(gen);
                    float rz = z + dis(gen);
                    Segment query{Point{rx,ry,rz}, Point{rx,ry, 10000}};
                    if(tree.number_of_intersected_primitives(query) % 2 == 1){
                        points.push_back({rx,ry,rz});
                    };
            }
        }
    }
    auto t2 = std::chrono::steady_clock::now();
    std::cout << (t2-t1)/std::chrono::microseconds(1) << ":us gen points\n";
    return points;
}
