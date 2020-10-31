cmake_minimum_required(VERSION 3.14)
project(vulkan_cube)

set(CMAKE_CXX_STANDARD 17)

# Qt
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_PREFIX_PATH /Users/lapua/Qt/5.15.1-build)
set(ENV{QT_VULKAN_LIB} /Users/lapua/programs/vulkansdk-macos-1.2.135.0/MoltenVK/macOS/dynamic/libMoltenVK.dylib)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

# Vulkan
set(ENV{VK_ICD_FILENAMES} /Users/lapua/programs/vulkansdk-macos-1.2.135.0/macOS/share/vulkan/icd.d/MoltenVK_icd.json)
set(ENV{VK_LAYER_PATH} /Users/lapua/programs/vulkansdk-macos-1.2.135.0/macOS/share/vulkan/explicit_layer.d)
include_directories(/Users/lapua/programs/vulkansdk-macos-1.2.135.0/macOS/include)
link_directories(/Users/lapua/programs/vulkansdk-macos-1.2.135.0/macOS/lib)
find_package(vulkan REQUIRED)

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

add_executable(vulkan_cube src/main.cpp src/draw/DrawManager.hpp src/draw/Instance.hpp src/draw/DeviceQueue.hpp src/draw/Debugger.hpp src/draw/common/Common.hpp src/draw/Presentation.hpp src/draw/GraphicsPipeline.hpp src/draw/Draw.hpp src/ui/VulkanWindow.hpp src/ui/MainWindow.hpp src/draw/VertexData.hpp)

find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} OpenGL::GL)

target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})

target_link_libraries(${PROJECT_NAME} Qt5::Widgets)
