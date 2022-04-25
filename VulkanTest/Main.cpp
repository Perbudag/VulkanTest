#include <iostream>
#include <stdexcept>

#include "vulkan.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

int main() {
    vulkan::vulkan app(800, 600, (char*)"Vulkan");

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}