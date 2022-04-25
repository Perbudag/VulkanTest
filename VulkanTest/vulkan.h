#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

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
        // Получение списка расширений, которые необходимы для работы программы
        std::vector<const char*> getRequiredExtensions(); 

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