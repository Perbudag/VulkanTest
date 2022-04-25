#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


namespace vulkan 
{
    class vulkan
    {
        bool _isClean = false;
        
        GLFWwindow* _window;
        uint32_t _width;
        uint32_t _height;
        char* _nameWindow;

        VkInstance _instance;

    public:
        vulkan(uint32_t width, uint32_t height, char* nameWindow);
        ~vulkan();

        void run();

    private:
        void initWindow();
        void initVulkan();
        void createInstance();

        void mainLoop();

        void cleanup();
    };

}