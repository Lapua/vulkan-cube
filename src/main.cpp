#include "draw/vulkan.hpp"

int main() {
    Vulkan vulkan;
    try {
        vulkan.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
