#include "draw/DrawManager.hpp"
#include <iostream>

int main() {
    DrawManager drawManager;
    try {
        drawManager.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
