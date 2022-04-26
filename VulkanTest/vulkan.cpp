#include "vulkan.h"

#include <cstdlib>
#include <stdexcept>
#include <iostream>

namespace vulkan
{
#pragma region Конструктор_и_деструктор
	
	vulkan::vulkan(uint32_t width, uint32_t height, char* nameWindow)
		:_width(width), _height(height), _nameWindow(nameWindow)
	{
	}

	vulkan::~vulkan()
	{
		if (!_isClean) cleanup();
	}

#pragma endregion

#pragma region Запуск
	
	void vulkan::run()
	{
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

#pragma endregion

#pragma region Инициализация
	
	void vulkan::initWindow()
	{
		//Инициализация GLFW
		glfwInit();

		//Отключение создания контекста OpenGL
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		//Отключение возможности изменения размера окна
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		//Инициализация окна
		_window = glfwCreateWindow(_width, _height, _nameWindow, nullptr, nullptr);
		
		if (_window == NULL)
		{
			throw std::runtime_error("failed to create window!");
		}
	}

	void vulkan::initVulkan()
	{
		createInstance();

#ifdef ENABLE_VALIDATION_LAYERS
		//Настройка отладочного мессенджера
		setupDebugMessenger();
#endif // ENABLE_VALIDATION_LAYERS
		//Поиск подходящего устройства для работы vulkan
		pickPhysicalDevice();
		//Создание логического сутройства
		createLogicalDevice();
	}
	
	void vulkan::createInstance()
	{
#ifdef ENABLE_VALIDATION_LAYERS

		if(!checkValidationLayerSupport())
			throw std::runtime_error("validation layers requested, but not available!");

#endif // ENABLE_VALIDATION_LAYERS


		//Информация о программе
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 2);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 2);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		//Информация о том, какие глобальных расширения и слои валидации мы хотим использовать 
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		//Получаем список необходимых расширений
		auto extensions = getRequiredExtensions();

		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());;
		createInfo.ppEnabledExtensionNames = extensions.data();
		

#ifdef ENABLE_VALIDATION_LAYERS
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;

		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
#else
		createInfo.enabledLayerCount = 0;

		createInfo.pNext = nullptr;
#endif // ENABLE_VALIDATION_LAYERS


		//Создание экземпляра vulkan
		if(vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create instance!");
		}
	}
	std::vector<const char*> vulkan::getRequiredExtensions()
	{
		//Получение необходимых расширений
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;

		//Получение списка необходимых расширений vulkan для взаимодействия с оконной системой
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef ENABLE_VALIDATION_LAYERS
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif // ENABLE_VALIDATION_LAYERS


		return extensions;
	}

	void vulkan::pickPhysicalDevice()
	{
		//Получение количества подключённых видеокарт
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		//Получение дескрипторов подключённых видеокарт
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

		//Поиск подходящих видеокарт
		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				_physicalDevice = device;
				break;
			}
		}

		if (_physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}
	bool vulkan::isDeviceSuitable(VkPhysicalDevice device)
	{
		//VkPhysicalDeviceProperties deviceProperties;
		//VkPhysicalDeviceFeatures deviceFeatures;
		//Получение основных свойств усройства (имя, тип, поддерживаемая версия Vulkan )
		//vkGetPhysicalDeviceProperties(device, &deviceProperties);
		//Получение информации о поддержке опциональных возможностей (сжатие текстур, 64-битные числа с плавающей точкой и тд.)
		//vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
		//
		//return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
		//	deviceFeatures.geometryShader;


		QueueFamilyIndices indices = findQueueFamilies(device);

		return indices.isComplete();
	}
	QueueFamilyIndices vulkan::findQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		//Получение количества семейств очередей поддерживает устройство
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		//Получение информации семействах очередей, которые поддерживает устройство
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		//Поиск семейства, которое поддерживает VK_QUEUE_GRAPHICS_BIT
		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}

			i++;
		}

		return indices;
	}

	void vulkan::createLogicalDevice()
	{
		QueueFamilyIndices indices = findQueueFamilies(_physicalDevice);

		//Указание необходимого количества очередей для одного семейства
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		//Задается очередь с поддержкой графических операций
		queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
		//Задается количество очередей
		queueCreateInfo.queueCount = 1;

		//Задание приоритета очереди (приоритет задается в диапазоне от 0 до 1)
		float queuePriority = 1.0f;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		//Указание используемых возможностей устройства 
		VkPhysicalDeviceFeatures deviceFeatures{};

		//Заполнение структуры для создания логического устройства
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		//Указание информации об очередях
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;

		//Указание информации о возможностях устройства
		createInfo.pEnabledFeatures = &deviceFeatures;

		//Указание расширений, используемых устройством
		createInfo.enabledExtensionCount = 0;
		
#ifdef ENABLE_VALIDATION_LAYERS

		//Указание слоев валидации
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

#else

		createInfo.enabledLayerCount = 0;

#endif // ENABLE_VALIDATION_LAYERS

		//Создание логического устройства
		if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		//Получение дескриптора очереди с поддержкой графических операций
		vkGetDeviceQueue(_device, indices.graphicsFamily.value(), 0, &_graphicsQueue);
	}

#pragma endregion

#pragma region Главный_цикл
	
	void vulkan::mainLoop()
	{
		while (!glfwWindowShouldClose(_window)) {
			glfwPollEvents();
		}
	}

#pragma endregion

#pragma region Очистка_данных
	
	void vulkan::cleanup()
	{
		vkDestroyDevice(_device, nullptr);

#ifdef ENABLE_VALIDATION_LAYERS
		//Уничтожение экземпляра мессенджера
		DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
#endif // ENABLE_VALIDATION_LAYERS

		//Уничтожение экземпляра vulkan
		vkDestroyInstance(_instance, nullptr);

		//Закрытие окна
		glfwDestroyWindow(_window);

		//Завершение работу GLFW
		glfwTerminate();

		_isClean = true;
	}

#pragma endregion

#pragma region Слои_валидации

#ifdef ENABLE_VALIDATION_LAYERS

	bool vulkan::checkValidationLayerSupport()
	{
		//Получение поличества доступных слоёв валидации
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		//Получение списка доступных слоев валидации
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		//Проверка, доступны ли слои, необходимые для валидации
		for (const char* layerName : validationLayers)
		{
			bool layerFound = true;

			for (const VkLayerProperties& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}
			if (!layerFound)
			{
				return false;
			}
		}
		
		return true;
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL vulkan::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	void vulkan::setupDebugMessenger()
	{
		//Задание настроек мессенджера
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		//Создание мессенджера
		if (CreateDebugUtilsMessengerEXT(_instance, &createInfo, nullptr, &_debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	void vulkan::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		
		//Задание степени серьезности, для которых будет вызываться callback - функция
		//VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT - диагностическое сообщение
		//VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT	  - информационное сообщение, например, о создании ресурса
		//VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT - сообщение о поведении, которое не обязательно является некорректным, но вероятнее всего указывает на ошибку
		//VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT   - сообщение о некорректном поведении, которое может привести к сбою
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		
		//Фильтровать сообщения по типу
		//VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT	  - произошедшее событие не связано со спецификацией или производительностью
		//VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  - произошедшее событие нарушает спецификацию или указывает на возможную ошибку
		//VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT - возможно неоптимальное использование Vulkan
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		
		//Задание callback-функции
		createInfo.pfnUserCallback = debugCallback;
	}

	VkResult vulkan::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		//Поиск функции для создания экземпляра мессенджера
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}
	void vulkan::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		//Поиск функции для уничтожения экземпляра мессенджера
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

#endif // ENABLE_VALIDATION_LAYERS
#pragma endregion
}