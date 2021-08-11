cmake_minimum_required(VERSION 3.19)


find_package(glm CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(unofficial-vulkan-memory-allocator CONFIG REQUIRED)
find_package(Boost 1.7 REQUIRED)
find_package(CGAL CONFIG REQUIRED)

add_executable(heart_sim main.cpp App.cpp App.h Buffer.h Image.h Vertex.h GraphicsPass.cpp GraphicsPass.h LoadAssets.cpp LoadAssets.h Utils.cpp Utils.h HeaderImpl.cpp VmaUniqueAllocator.h PresentPass.cpp PresentPass.h ViewState.h NodeGraph.cpp NodeGraph.h ComputePass.cpp ComputePass.h FillPoints.cpp FillPoints.h NodeState.h)

add_custom_target(CompileShaders
        COMMAND ${CMAKE_SOURCE_DIR}/src/shaders/compile.bat)

add_custom_target(CopyShaders
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/src/shaders/ $<TARGET_FILE_DIR:heart_sim>
        DEPENDS CompileShaders
        )

add_custom_target(CopyData
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data/ $<TARGET_FILE_DIR:heart_sim>)

add_dependencies(heart_sim CopyShaders)
add_dependencies(heart_sim CopyData)

target_link_libraries(heart_sim PRIVATE
        glm::glm
        Vulkan::Vulkan
        glfw
        unofficial::vulkan-memory-allocator::vulkan-memory-allocator
        Boost::boost
        CGAL::CGAL)

target_include_directories(heart_sim PRIVATE ../stl_reader)