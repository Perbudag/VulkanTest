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

    //Список используемых слоёв валидации
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

    //Хранит информацию, необходимую для работы Swap chain
    struct SwapChainSupportDetails {
        //Базовые требования (capabilities) surface,
        //такие как мин/макс число изображений в swap chain, мин/макс ширина и высота изображений
        VkSurfaceCapabilitiesKHR capabilities;
        //Формат surface(формат пикселей, цветовое пространство)
        std::vector<VkSurfaceFormatKHR> formats;
        //Доступные режимы работы
        std::vector<VkPresentModeKHR> presentModes;
    };

    class vulkan
    {
        bool _isClean = false;
        
        //Переменные для GLFW
        GLFWwindow* _window;
        uint32_t _width;
        uint32_t _height;
        char* _nameWindow;

        //Переменные для vulkan
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

        //Список используемых расширений vulkan
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
        
        //Создание и настройка экземпляра vulkan
        void createInstance();
        //Получение списка расширений, которые необходимы для работы программы
        std::vector<const char*> getRequiredExtensions(); 
        
        //Поиск подходящих устройств, для работы vulkan
        void pickPhysicalDevice();
        //Проверка, является ли устройство device подходящим
        bool isDeviceSuitable(VkPhysicalDevice device);
        //Проверка, какие семейства очередей поддерживает устройство device
        //и какое из этих семейств поддерживает необходимые команды, для работы программы
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        //Проверка, поддерживаются ли используемые расширания
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);
        
        //Создание Swap сhain
        void createSwapChain();
        //Собирает всю необходимую информацию для работы Swap сhain (заполняет структуру SwapChainSupportDetails)
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
        //Настройка формата работы surface (глубина цвета)
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        //Настройка режима работы (условия для смены кадров на экране)
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        //Настройка swap extent (разрешение изображений в swap chain)
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
        
        //Создание ImageViews
        void createImageViews();

        //Создание графического конфеера
        void createGraphicsPipeline();
        //Создание шейдерных модулей
        VkShaderModule createShaderModule(const std::vector<char>& code);
        static std::vector<char> readFile(const std::string& filename);

        //Создание прохода рендера (Настройка рендера)
        void createRenderPass();

        //Создание логического устройства
        void createLogicalDevice();

        //Создание обстрактной, для показа отрендеренных изображений (surface)
        void createSurface();

        //Создание фреймбуферов
        void createFramebuffers();

        //Главный цикл, в котором происходит вся отрисовка
        void mainLoop(); 


        //Очистка данных
        void cleanup(); 

#ifdef ENABLE_VALIDATION_LAYERS

        //Проверка, доступны ли слои, необходимые для валидации
        bool checkValidationLayerSupport();

        //callback-функция для отладки программы
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

        //Настраивает отладочный мессенджер
        void setupDebugMessenger();
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

        //функция для создания мессенджера
        static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
        //функция для уничтожения мессенджера
        static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

#endif // ENABLE_VALIDATION_LAYERS

    };

}