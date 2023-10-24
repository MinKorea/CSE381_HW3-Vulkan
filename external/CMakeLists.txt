cmake_minimum_required(VERSION 3.19.0)

project(external LANGUAGES C CXX)


# vulkan
find_package(Vulkan REQUIRED GLOBAL)

# tinygltf
add_library(tinygltf INTERFACE)
set(TINYGLTF_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tinygltf)
target_sources(tinygltf INTERFACE ${TINYGLTF_DIR}/tiny_gltf.h ${TINYGLTF_DIR}/json.hpp)
target_include_directories(tinygltf INTERFACE ${TINYGLTF_DIR})

# glm
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/glm)


# glfw
option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_INSTALL OFF)
option(BUILD_SHARED_LIBS ON)
add_subdirectory(glfw)

# stb
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE 
${CMAKE_CURRENT_SOURCE_DIR}/stb
)

# spdlog
add_library(spdlog INTERFACE)
set(SPD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/spdlog/include)
target_include_directories(spdlog INTERFACE ${SPD_DIR})

# vma
add_library(vma INTERFACE)
set(VMA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/VulkanMemoryAllocator/include)
target_sources(vma INTERFACE ${VMA_DIR}/vk_mem_alloc.h)
target_include_directories(vma INTERFACE ${VMA_DIR})
target_link_libraries(vma INTERFACE Vulkan::Vulkan)

# gli
add_library(gli INTERFACE)
set(GLI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gli)
target_sources(gli INTERFACE ${GLI_DIR}/gli/gli.hpp)
target_include_directories(gli INTERFACE ${GLI_DIR})

# renderdoc
add_library(renderdoc INTERFACE)
set(RENDERDOC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/renderdoc)
target_sources(renderdoc INTERFACE ${RENDERDOC_DIR}/renderdoc_app.h)
target_include_directories(renderdoc INTERFACE ${RENDERDOC_DIR})


