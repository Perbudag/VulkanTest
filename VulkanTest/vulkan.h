#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <string>
#include <optional>

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


    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() 
                && presentFamily.has_value();
        }
    };

    //������ ����������, ����������� ��� ������ Swap chain
    struct SwapChainSupportDetails {
        //������� ���������� (capabilities) surface,
        //����� ��� ���/���� ����� ����������� � swap chain, ���/���� ������ � ������ �����������
        VkSurfaceCapabilitiesKHR capabilities;
        //������ surface(������ ��������, �������� ������������)
        std::vector<VkSurfaceFormatKHR> formats;
        //��������� ������ ������
        std::vector<VkPresentModeKHR> presentModes;
    };

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

        VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
        VkDevice _device;
        VkQueue _graphicsQueue;

        VkSurfaceKHR _surface;
        VkQueue _presentQueue;
        VkSwapchainKHR _swapChain;
        std::vector<VkImage> _swapChainImages;
        VkFormat _swapChainImageFormat;
        VkExtent2D _swapChainExtent;
        std::vector<VkImageView> _swapChainImageViews;
        VkRenderPass _renderPass;
        VkPipelineLayout _pipelineLayout;
        VkPipeline _graphicsPipeline;
        std::vector<VkFramebuffer> _swapChainFramebuffers;

        //������ ������������ ���������� vulkan
        const std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

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
        //��������� ������ ����������, ������� ���������� ��� ������ ���������
        std::vector<const char*> getRequiredExtensions(); 
        
        //����� ���������� ���������, ��� ������ vulkan
        void pickPhysicalDevice();
        //��������, �������� �� ���������� device ����������
        bool isDeviceSuitable(VkPhysicalDevice device);
        //��������, ����� ��������� �������� ������������ ���������� device
        //� ����� �� ���� �������� ������������ ����������� �������, ��� ������ ���������
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        //��������, �������������� �� ������������ ����������
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);
        
        //�������� Swap �hain
        void createSwapChain();
        //�������� ��� ����������� ���������� ��� ������ Swap �hain (��������� ��������� SwapChainSupportDetails)
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
        //��������� ������� ������ surface (������� �����)
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        //��������� ������ ������ (������� ��� ����� ������ �� ������)
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        //��������� swap extent (���������� ����������� � swap chain)
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
        
        //�������� ImageViews
        void createImageViews();

        //�������� ������������ ��������
        void createGraphicsPipeline();
        //�������� ��������� �������
        VkShaderModule createShaderModule(const std::vector<char>& code);
        static std::vector<char> readFile(const std::string& filename);

        //�������� ������� ������� (��������� �������)
        void createRenderPass();

        //�������� ����������� ����������
        void createLogicalDevice();

        //�������� �����������, ��� ������ ������������� ����������� (surface)
        void createSurface();

        //�������� ������������
        void createFramebuffers();

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