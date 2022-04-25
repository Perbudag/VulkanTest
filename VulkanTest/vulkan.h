#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

namespace vulkan
{


#ifdef NDEBUG
#else

    //������ ������������ ���� ���������
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

#define ENABLE_VALIDATION_LAYERS
#endif


    class vulkan
    {
        bool _isClean = false;
        
        //���������� ��� GLFW
        GLFWwindow* _window;
        uint32_t _width;
        uint32_t _height;
        char* _nameWindow;

        //���������� ��� vulkan
        VkInstance _instance;

#ifdef ENABLE_VALIDATION_LAYERS
        VkDebugUtilsMessengerEXT _debugMessenger;
#endif // ENABLE_VALIDATION_LAYERS

    public:
        vulkan(uint32_t width, uint32_t height, char* nameWindow);
        ~vulkan();

        void run();

    private:
        void initWindow();
        void initVulkan();
        //�������� � ��������� ���������� vulkan
        void createInstance();
        // ��������� ������ ����������, ������� ���������� ��� ������ ���������
        std::vector<const char*> getRequiredExtensions(); 

        //������� ����, � ������� ���������� ��� ���������
        void mainLoop(); 

        //������� ������
        void cleanup(); 

#ifdef ENABLE_VALIDATION_LAYERS

        //��������, �������� �� ����, ����������� ��� ���������
        bool checkValidationLayerSupport();

        //callback-������� ��� ������� ���������
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

        //����������� ���������� ����������
        void setupDebugMessenger();
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

        //������� ��� �������� �����������
        static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
        //������� ��� ����������� �����������
        static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

#endif // ENABLE_VALIDATION_LAYERS

    };

}