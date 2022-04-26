#include "vulkan.h"

#include <cstdlib>
#include <stdexcept>
#include <iostream>

namespace vulkan
{
#pragma region �����������_�_����������
	
	vulkan::vulkan(uint32_t width, uint32_t height, char* nameWindow)
		:_width(width), _height(height), _nameWindow(nameWindow)
	{
	}

	vulkan::~vulkan()
	{
		if (!_isClean) cleanup();
	}

#pragma endregion

#pragma region ������
	
	void vulkan::run()
	{
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

#pragma endregion

#pragma region �������������
	
	void vulkan::initWindow()
	{
		//������������� GLFW
		glfwInit();

		//���������� �������� ��������� OpenGL
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		//���������� ����������� ��������� ������� ����
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		//������������� ����
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
		//��������� ����������� �����������
		setupDebugMessenger();
#endif // ENABLE_VALIDATION_LAYERS
		//����� ����������� ���������� ��� ������ vulkan
		pickPhysicalDevice();
		//�������� ����������� ����������
		createLogicalDevice();
	}
	
	void vulkan::createInstance()
	{
#ifdef ENABLE_VALIDATION_LAYERS

		if(!checkValidationLayerSupport())
			throw std::runtime_error("validation layers requested, but not available!");

#endif // ENABLE_VALIDATION_LAYERS


		//���������� � ���������
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 2);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 2);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		//���������� � ���, ����� ���������� ���������� � ���� ��������� �� ����� ������������ 
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		//�������� ������ ����������� ����������
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


		//�������� ���������� vulkan
		if(vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create instance!");
		}
	}
	std::vector<const char*> vulkan::getRequiredExtensions()
	{
		//��������� ����������� ����������
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;

		//��������� ������ ����������� ���������� vulkan ��� �������������� � ������� ��������
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

#ifdef ENABLE_VALIDATION_LAYERS
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif // ENABLE_VALIDATION_LAYERS


		return extensions;
	}

	void vulkan::pickPhysicalDevice()
	{
		//��������� ���������� ������������ ���������
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		//��������� ������������ ������������ ���������
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

		//����� ���������� ���������
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
		//��������� �������� ������� ��������� (���, ���, �������������� ������ Vulkan )
		//vkGetPhysicalDeviceProperties(device, &deviceProperties);
		//��������� ���������� � ��������� ������������ ������������ (������ �������, 64-������ ����� � ��������� ������ � ��.)
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

		//��������� ���������� �������� �������� ������������ ����������
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		//��������� ���������� ���������� ��������, ������� ������������ ����������
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		//����� ���������, ������� ������������ VK_QUEUE_GRAPHICS_BIT
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

		//�������� ������������ ���������� �������� ��� ������ ���������
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		//�������� ������� � ���������� ����������� ��������
		queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
		//�������� ���������� ��������
		queueCreateInfo.queueCount = 1;

		//������� ���������� ������� (��������� �������� � ��������� �� 0 �� 1)
		float queuePriority = 1.0f;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		//�������� ������������ ������������ ���������� 
		VkPhysicalDeviceFeatures deviceFeatures{};

		//���������� ��������� ��� �������� ����������� ����������
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		//�������� ���������� �� ��������
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;

		//�������� ���������� � ������������ ����������
		createInfo.pEnabledFeatures = &deviceFeatures;

		//�������� ����������, ������������ �����������
		createInfo.enabledExtensionCount = 0;
		
#ifdef ENABLE_VALIDATION_LAYERS

		//�������� ����� ���������
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

#else

		createInfo.enabledLayerCount = 0;

#endif // ENABLE_VALIDATION_LAYERS

		//�������� ����������� ����������
		if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		//��������� ����������� ������� � ���������� ����������� ��������
		vkGetDeviceQueue(_device, indices.graphicsFamily.value(), 0, &_graphicsQueue);
	}

#pragma endregion

#pragma region �������_����
	
	void vulkan::mainLoop()
	{
		while (!glfwWindowShouldClose(_window)) {
			glfwPollEvents();
		}
	}

#pragma endregion

#pragma region �������_������
	
	void vulkan::cleanup()
	{
		vkDestroyDevice(_device, nullptr);

#ifdef ENABLE_VALIDATION_LAYERS
		//����������� ���������� �����������
		DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
#endif // ENABLE_VALIDATION_LAYERS

		//����������� ���������� vulkan
		vkDestroyInstance(_instance, nullptr);

		//�������� ����
		glfwDestroyWindow(_window);

		//���������� ������ GLFW
		glfwTerminate();

		_isClean = true;
	}

#pragma endregion

#pragma region ����_���������

#ifdef ENABLE_VALIDATION_LAYERS

	bool vulkan::checkValidationLayerSupport()
	{
		//��������� ���������� ��������� ���� ���������
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		//��������� ������ ��������� ����� ���������
		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		//��������, �������� �� ����, ����������� ��� ���������
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
		//������� �������� �����������
		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateDebugMessengerCreateInfo(createInfo);

		//�������� �����������
		if (CreateDebugUtilsMessengerEXT(_instance, &createInfo, nullptr, &_debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	void vulkan::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		
		//������� ������� �����������, ��� ������� ����� ���������� callback - �������
		//VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT - ��������������� ���������
		//VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT	  - �������������� ���������, ��������, � �������� �������
		//VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT - ��������� � ���������, ������� �� ����������� �������� ������������, �� ��������� ����� ��������� �� ������
		//VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT   - ��������� � ������������ ���������, ������� ����� �������� � ����
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		
		//����������� ��������� �� ����
		//VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT	  - ������������ ������� �� ������� �� ������������� ��� �������������������
		//VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT  - ������������ ������� �������� ������������ ��� ��������� �� ��������� ������
		//VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT - �������� ������������� ������������� Vulkan
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		
		//������� callback-�������
		createInfo.pfnUserCallback = debugCallback;
	}

	VkResult vulkan::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		//����� ������� ��� �������� ���������� �����������
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
		//����� ������� ��� ����������� ���������� �����������
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

#endif // ENABLE_VALIDATION_LAYERS
#pragma endregion
}