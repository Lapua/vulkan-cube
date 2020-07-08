#include <iostream>
#include "draw/DrawManager.hpp"
//#include "example.hpp"

int main() {
    try {
//        exampleMain();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    // TODO
    // drawFrameまわり・destroy
    // uniformBufferのrecreateSwapChain
    DrawManager drawManager;
    try {
        drawManager.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
