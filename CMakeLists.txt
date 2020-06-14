cmake_minimum_required(VERSION 3.14)
project(vulkan_cube)

set(CMAKE_CXX_STANDARD 17)

set(ENV{VK_ICD_FILENAMES} /Users/lapua/programs/vulkansdk-macos-1.2.135.0/macOS/share/vulkan/icd.d/MoltenVK_icd.json)
set(ENV{VK_LAYER_PATH} /Users/lapua/programs/vulkansdk-macos-1.2.135.0/macOS/share/vulkan/explicit_layer.d)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)
add_executable(vulkan_cube src/main.cpp)

find_package(glfw3 3.3 REQUIRED)
target_link_libraries(${PROJECT_NAME} glfw)

find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} OpenGL::GL)

find_package(vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})
