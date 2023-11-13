cmake_minimum_required(VERSION 3.19.0)

project(Wolfie3D)

if (MINGW) 
    include(${CMAKE_CURRENT_SOURCE_DIR}/windows.env.cmake)
    set(CMAKE_C_COMPILER ${MINGW_PATH}/bin/clang.exe)
    set(CMAKE_CXX_COMPILER ${MINGW_PATH}/bin/clang++.exe)
    add_link_options("-fuse-ld=lld")
endif()

execute_process(COMMAND uname OUTPUT_VARIABLE uname)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/shaders)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE
    src/main.cpp
    src/gltf_loader.cpp
    src/gltf_loader.hpp
    src/stb_image_resize.cpp
    src/tiny_gltf.cpp
    src/pbr_baker.cpp
    src/pbr_baker.hpp
    src/controller.cpp
    src/controller.hpp

    src/common/cvar.cpp
    src/common/cvar.hpp
    src/common/error.hpp
    src/common/file_utils.cpp
    src/common/file_utils.hpp
    src/common/glm_common.hpp
    src/common/logging.hpp
    src/common/timer.cpp
    src/common/timer.hpp
    src/common/utils.cpp
    src/common/utils.hpp
    src/common/vk_common.hpp

    src/core/command_buffer.cpp
    src/core/command_buffer.hpp
    src/core/command_pool.cpp
    src/core/command_pool.hpp
    src/core/descriptor_allocator.cpp
    src/core/descriptor_allocator.hpp
    src/core/device.cpp
    src/core/device.hpp
    src/core/device_memory
    src/core/framebuffer.cpp
    src/core/framebuffer.hpp
    src/core/graphics_pipeline.cpp
    src/core/graphics_pipeline.hpp
    src/core/image_resource.cpp
    src/core/image_resource.hpp
    src/core/image_view.cpp
    src/core/image_view.hpp
    src/core/instance.cpp
    src/core/instance.hpp
    src/core/physical_device.cpp
    src/core/physical_device.hpp
    src/core/pipeline_layout.cpp
    src/core/pipeline_layout.hpp
    src/core/render_pass.cpp
    src/core/render_pass.hpp
    src/core/renderer.cpp
    src/core/renderer.hpp
    src/core/sampler.cpp
    src/core/sampler.hpp
    src/core/surface.cpp
    src/core/surface.hpp
    src/core/swapchain.cpp
    src/core/swapchain.hpp
    src/core/sync_objects.cpp
    src/core/sync_objects.hpp
    src/core/vulkan_object.hpp
    src/core/window.cpp
    src/core/window.hpp

    src/core/device_memory/allocator.cpp
    src/core/device_memory/allocator.hpp
    src/core/device_memory/buffer.cpp
    src/core/device_memory/buffer.hpp
    src/core/device_memory/device_memory_object.hpp
    src/core/device_memory/image.cpp
    src/core/device_memory/image.hpp
    src/core/device_memory/vk_mem_alloc.cpp

    src/scene_graph/component.cpp
    src/scene_graph/component.hpp
    src/scene_graph/event.hpp
    src/scene_graph/node.cpp
    src/scene_graph/node.hpp
    src/scene_graph/scene.cpp
    src/scene_graph/scene.hpp
    src/scene_graph/script.cpp
    src/scene_graph/script.hpp
    src/scene_graph/components/aabb.cpp
    src/scene_graph/components/aabb.hpp
    src/scene_graph/components/camera.cpp
    src/scene_graph/components/camera.hpp
    src/scene_graph/components/image.cpp
    src/scene_graph/components/image.hpp
    src/scene_graph/components/material.cpp
    src/scene_graph/components/material.hpp
    src/scene_graph/components/mesh.cpp
    src/scene_graph/components/mesh.hpp
    src/scene_graph/components/pbr_material.cpp
    src/scene_graph/components/pbr_material.hpp
    src/scene_graph/components/perspective_camera.cpp
    src/scene_graph/components/perspective_camera.hpp
    src/scene_graph/components/sampler.cpp
    src/scene_graph/components/sampler.hpp
    src/scene_graph/components/submesh.cpp
    src/scene_graph/components/submesh.hpp
    src/scene_graph/components/texture.hpp
    src/scene_graph/components/texture.cpp
    src/scene_graph/components/transform.cpp
    src/scene_graph/components/transform.hpp
    src/scene_graph/scripts/free_camera.cpp
    src/scene_graph/scripts/free_camera.hpp
    src/scene_graph/scripts/player.cpp
    src/scene_graph/scripts/player.hpp
    src/scene_graph/scripts/light.cpp
    src/scene_graph/scripts/light.hpp
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 20 
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)

if (MINGW)
    target_include_directories(${PROJECT_NAME} PUBLIC ${MINGW_PATH}/include)
    target_link_directories(${PROJECT_NAME} PUBLIC ${MINGW_PATH}/lib)
endif()


target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)


target_link_libraries(${PROJECT_NAME}
    tinygltf
    glm
    glfw
    spdlog
    stb
    Vulkan::Vulkan
    vma
    gli
    renderdoc
)
