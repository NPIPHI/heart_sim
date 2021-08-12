//
// Created by 16182 on 8/8/2021.
//

#include "FillPoints.h"
#include <CGAL/Simple_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>
#include<random>
#include<thread>

#include<chrono>
std::vector<glm::vec3> FillPoints::random_fill(std::span<Vertex> vertices, std::span<uint32_t> indices, float density) {
    using K = CGAL::Simple_cartesian<float>;
    using Segment = K::Segment_3;
    using Point = K::Point_3;
    using Triangle = K::Triangle_3;
    using Primitive = CGAL::AABB_triangle_primitive<K, std::vector<Triangle>::iterator>;
    using Tree = CGAL::AABB_tree<CGAL::AABB_traits<K, Primitive>>;

    std::vector<Triangle> triangles;
    auto to_tri = [&](size_t idx){
        auto p1 = vertices[indices[idx*3+0]].position;
        auto p2 = vertices[indices[idx*3+1]].position;
        auto p3 = vertices[indices[idx*3+2]].position;
        return Triangle{{p1.x, p1.y, p1.z},{p2.x, p2.y, p2.z},{p3.x, p3.y, p3.z}};
    };

    for(size_t triangle_index = 0; triangle_index < indices.size()/3; triangle_index++){
        triangles.push_back(to_tri(triangle_index));
    }

    auto t1 = std::chrono::steady_clock::now();
    Tree tree{triangles.begin(),  triangles.end()};
    tree.build();


    std::vector<std::thread> thread_pool;
    size_t thread_pool_size = std::thread::hardware_concurrency();
    std::mutex points_mutex;
    std::vector<glm::vec3> points;

    for(size_t i = 0; i < thread_pool_size; i++){
        float xdif = (tree.bbox().xmax() - tree.bbox().xmin())/thread_pool_size;
        float xmin = tree.bbox().xmin() + xdif * i;
        float xmax = xmin + xdif;
        thread_pool.emplace_back([&, xmin, xmax]{
            std::vector<glm::vec3> local_points;
            std::random_device rd;  //Will be used to obtain a seed for the random number engine
            std::mt19937 gen{}; //Standard mersenne_twister_engine seeded with rd()
            std::uniform_real_distribution<float> dis(-0.6, 0.6);
            for(float x = xmin; x < xmax; x+= density){
                for(float y = tree.bbox().ymin(); y < tree.bbox().ymax(); y += density){
                    for(float z = tree.bbox().zmin(); z < tree.bbox().zmax(); z += density){
                        float rx = x + dis(gen);
                        float ry = y + dis(gen);
                        float rz = z + dis(gen);
                        Segment query{Point{rx,ry,rz}, Point{rx,ry, 10000}};
                        if(tree.number_of_intersected_primitives(query) % 2 == 1){
                            local_points.push_back({rx,ry,rz});
                        };
                    }
                }
            }
            std::lock_guard<std::mutex> l(points_mutex);
            points.insert(points.end(), local_points.begin(), local_points.end());
        });
    }
    for(auto & t : thread_pool){
        t.join();
    }
//    for(float x = tree.bbox().xmin(); x < tree.bbox().xmax(); x+= density){
//        for(float y = tree.bbox().ymin(); y < tree.bbox().ymax(); y += density){
//            for(float z = tree.bbox().zmin(); z < tree.bbox().zmax(); z += density){
//                    float rx = x + dis(gen);
//                    float ry = y + dis(gen);
//                    float rz = z + dis(gen);
//                    Segment query{Point{rx,ry,rz}, Point{rx,ry, 10000}};
//                    if(tree.number_of_intersected_primitives(query) % 2 == 1){
//                        points.push_back({rx,ry,rz});
//                    };
//            }
//        }
//    }


    auto t2 = std::chrono::steady_clock::now();
    std::cout << (t2-t1)/std::chrono::microseconds(1) << ":us gen points\n";
    return points;
}
