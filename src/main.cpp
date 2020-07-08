#include <iostream>
#include "draw/DrawManager.hpp"

int main() {
    DrawManager drawManager;
    try {
        drawManager.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
