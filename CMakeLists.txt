cmake_minimum_required(VERSION 3.14)
project(vulkan_cube)

set(CMAKE_CXX_STANDARD 17)

# Vulkan
set(ENV{VK_ICD_FILENAMES} /Users/lapua/programs/vulkansdk-macos-1.2.135.0/macOS/share/vulkan/icd.d/MoltenVK_icd.json)
set(ENV{VK_LAYER_PATH} /Users/lapua/programs/vulkansdk-macos-1.2.135.0/macOS/share/vulkan/explicit_layer.d)
set(ENV{glfw3_DIR} /usr/local/Cellar/glfw/HEAD-d7ae90a/lib/cmake/glfw3)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)
add_executable(vulkan_cube src/main.cpp src/draw/DrawManager.hpp src/draw/Instance.hpp src/draw/DeviceQueue.hpp src/draw/Debugger.hpp src/draw/common/Common.hpp src/draw/Presentation.hpp src/draw/GraphicsPipeline.hpp src/draw/Draw.hpp src/draw/common/Vertex.hpp src/ui/UIManager.hpp src/ui/MainWindow.hpp)

find_package(glfw3 3.3 REQUIRED)
target_link_libraries(${PROJECT_NAME} glfw)

find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} OpenGL::GL)

find_package(vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})

# Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_PREFIX_PATH /Users/lapua/Qt/5.15.1/clang_64)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)