//
// Created by 16182 on 8/2/2021.
//

#include "NodeGraph.h"
#include<boost/geometry.hpp>
#include<boost/geometry/geometries/point.hpp>
#include<boost/geometry/index/rtree.hpp>
#include<chrono>
#include<iostream>
#include<thread>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

using point = bg::model::point<float, 3, bg::cs::cartesian>;
using value = std::pair<point, uint32_t>;


NodeGraph::NodeGraph(std::span<const Vertex> vertices) {
    edges.resize(vertices.size());
    auto t1 = std::chrono::steady_clock::now();
    bgi::rtree<value, bgi::quadratic<16>> tree;
    for(size_t i = 0; i < vertices.size(); i++){
        auto v = vertices[i].pos;
        point p = {v.x, v.y, v.z};
        tree.insert({p, i});
    }


    auto build = [&](size_t node){
        auto v = vertices[node].pos;
        point p{v.x, v.y, v.z};
        auto iter = tree.qbegin(bgi::nearest(p, neighbor_count + 1));
        //skip the point itself
        iter++;
        auto write = edges[node].begin();
        for(auto i = iter; i != tree.qend(); i++){
            *write = i->second;
            write++;
        }
    };


    auto t2 = std::chrono::steady_clock::now();
    constexpr size_t thread_pool_size = 12;
    std::vector<std::thread> thread_pool;
    size_t step = vertices.size() / thread_pool_size;
    for(size_t i = 0; i < thread_pool_size; i++){
        size_t begin = i * step;
        size_t end = (i + 1) * step;
        thread_pool.emplace_back([&build, begin,end]{
            for(size_t node = begin; node < end; node++){
                build(node);
            }
        });
    }
    for(size_t node = step * thread_pool_size; node < vertices.size(); node++){
        build(node);
    }
    for(auto & t : thread_pool){
        t.join();
    }
    auto t3 = std::chrono::steady_clock::now();
    std::cout \
    << (t2-t1)/std::chrono::microseconds(1) << ":us build\n"
    << (t3-t2)/std::chrono::microseconds(1) << ":us query\n"
    << (t3-t1)/std::chrono::microseconds (1) << ":us total\n";
}

