#ifndef VULKAN_CUBE_VERTEX_HPP
#define VULKAN_CUBE_VERTEX_HPP

#include <glm/glm.hpp>
#include <array>
#include <vector>
#include <vulkan/vulkan.h>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        // メモリから頂点をロード
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    // 位置と色で2つ
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};

std::vector<Vertex> axisVertices = {
    {{200.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    {{-200.0f, 0.0f, 0.0f}, {0.1f, 0.0f, 0.0f}},
    {{0.0f, 200.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{0.0f, -200.0f, 0.0f}, {0.0f, 0.1f, 0.0f}},
    {{0.0f, 0.0f, 200.0f}, {0.0f, 0.0f, 1.0f}},
    {{0.0f, 0.0f, -200.0f}, {0.0f, 0.0f, 0.1f}}
};

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

#endif //VULKAN_CUBE_VERTEX_HPP
