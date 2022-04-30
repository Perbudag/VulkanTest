#include "vulkan.h"

#include <cstdlib>
#include <stdexcept>
#include <iostream>
#include <set>
#include <cstdint>
#include <algorithm>
#include <fstream>

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
		//Создание обстрактной, для показа отрендеренных изображений (surface)
		createSurface();
		//Поиск подходящего устройства для работы vulkan
		pickPhysicalDevice();
		//Создание логического сутройства
		createLogicalDevice();
		//Создание Swap сhain
		createSwapChain();
		//Создание ImageViews
		createImageViews();
		//Создание прохода рендера (Настройка рендера)
		createRenderPass();
		//Создание графического конфеера
		createGraphicsPipeline();
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

		bool extensionsSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return indices.isComplete() && swapChainAdequate;
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

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &presentSupport);

			if (presentSupport) {
				indices.presentFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}

			i++;
		}

		return indices;
	}
	bool vulkan::checkDeviceExtensionSupport(VkPhysicalDevice device)
	{
		//Получение количества поддерживаемых расширений на видеокарте
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		//Получение информации о поддерживаемых расширениях на видеокарте
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
			if (requiredExtensions.empty())
			{
				return true;
			}
		}

		return false;
	}
	
	void vulkan::createSwapChain()
	{
		//Собор всей необходимой информации для создания Swap сhain
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(_physicalDevice);

		//Настройка формата работы surface
		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		//Настройка режима работы
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		//Настройка swap extent
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		//Определяем сколько объектов image должно быть в swap chain
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		//Структура для создания Swap chain
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = _surface;

		//Информация для создания image объектов
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = findQueueFamilies(_physicalDevice);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		//Есть два способа обработки image с доступом из нескольких очередей:
		//VK_SHARING_MODE_EXCLUSIVE: объект принадлежит одному семейству очередей, 
		// и право владения должно быть передано явно перед использованием его в другом семействе очередей. 
		// Такой способ обеспечивает самую высокую производительность.
	    //VK_SHARING_MODE_CONCURRENT: объекты могут использоваться в нескольких семействах очередей без явной 
		// передачи права владения.
		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		//Определяем, какие преобразования будут применены к изображению в swap chain
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		//указываем, нужно ли использовать альфа - канал для смешивания с другими окнами в оконной системе
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		//Указываем, что не нужно рендерить скрытые пикселы (например, если часть нашего окна перекрыта другим окном)
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = VK_NULL_HANDLE;

		//Создание swap сhain
		if (vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapChain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain!");
		}

		//Получение дескрипторов Image
		vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, nullptr);
		_swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, _swapChainImages.data());

		_swapChainImageFormat = surfaceFormat.format;
		_swapChainExtent = extent;
	}
	SwapChainSupportDetails vulkan::querySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;

		//Получение базовых требований (capabilities) surface, 
		//такие как мин/макс число изображений в swap chain, мин/макс ширина и высота изображений
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &details.capabilities);

		//Получение количества поддерживаемых форматов surface
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, nullptr);

		if (formatCount != 0) {
			//Получение информации о поддерживаемых форматах surface
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, details.formats.data());
		}

		//Получение количества доступных режимов работы surface
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			//Получение информации о доступных режимов работы surface
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}
	VkSurfaceFormatKHR vulkan::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{

		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];;
	}
	VkPresentModeKHR vulkan::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		//Все доступные режимы работы в vulkan
		//VK_PRESENT_MODE_IMMEDIATE_KHR: изображения, отправленные вашим приложением, немедленно отправляются на экран, 
		// что может приводить к артефактам.
		//VK_PRESENT_MODE_FIFO_KHR : изображения для вывода на экран берутся из начала очереди в момент обновления экрана.
		// В то время, как программа помещает отрендеренные изображения в конец очереди.Если очередь заполнена, программа 
		// будет ждать.Это похоже на вертикальную синхронизацию, используемую в современных играх.
		//VK_PRESENT_MODE_FIFO_RELAXED_KHR : этот режим отличается от предыдущего только в одном случае, когда происходит 
		// задержка программы и в момент обновления экрана остается пустая очередь.Тогда изображение передается на экран 
		// сразу после его появления без ожидания обновления экрана.Это может привести к видимым артефактам.
		//VK_PRESENT_MODE_MAILBOX_KHR : это еще один вариант второго режима.Вместо того, чтобы блокировать программу при 
		// заполнении очереди, изображения в очереди заменяются новыми.Этот режим подходит для реализации тройной буферизации.
		// С ней вы можете избежать появления артефактов при низком времени ожидания.

		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}
	VkExtent2D vulkan::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		}
		else {
			//Получение размера изображения в пикселях
			int width, height;
			glfwGetFramebufferSize(_window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			//Ограничение размера изображения в пределах  minImageExtent и maxImageExtent
			actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}
	
	void vulkan::createImageViews()
	{
		_swapChainImageViews.resize(_swapChainImages.size());

		for (size_t i = 0; i < _swapChainImages.size(); i++) {
			//Параметры для создания image view
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = _swapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = _swapChainImageFormat;

			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			//Поле subresourceRange описывает, какая часть image будет использоваться.
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(_device, &createInfo, nullptr, &_swapChainImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create image views!");
			}
		}
	}

	void vulkan::createGraphicsPipeline()
	{
		//Загрузко шейдеров из файлов
		auto vertShaderCode = readFile("shaders/vert.spv");
		auto fragShaderCode = readFile("shaders/frag.spv");

		//Создание шейдерных модулей
		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

		//Информация для связывания графического конвеера и вертексного шейдера
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		//Информация для связывания графического конвеера и фрагментного шейдера
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		//Настройки входных данных вершин
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

		//Описывается: какая геометрия образуется из вершин и 
		// разрешен ли рестарт геометрии для таких геометрий, как line strip и triangle strip
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		//VK_PRIMITIVE_TOPOLOGY_POINT_LIST: геометрия отрисовывается в виде отдельных точек, 
		// каждая вершина — отдельная точка
		//VK_PRIMITIVE_TOPOLOGY_LINE_LIST: геометрия отрисовывается в виде набора отрезков, каждая 
		// пара вершин образует отдельный отрезок
		//VK_PRIMITIVE_TOPOLOGY_LINE_STRIP : геометрия отрисовывается в виде непрерывной ломаной, 
		// каждая последующая вершина добавляет к ломаной один отрезок
		//VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST : геометрия отрисовывается как набор треугольников, 
		// причем каждые 3 вершины образуют независимый треугольник
		//VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP : геометрия отрисовывается как набор связанных треугольников, 
		// причем две последние вершины предыдущего треугольника используются в качестве двух первых вершин для 
		// следующего треугольника
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		//Вьюпорт описывает область фреймбуфера, в которую рендерятся выходные данные
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)_swapChainExtent.width;
		viewport.height = (float)_swapChainExtent.height;
		//minDepth и maxDepth определяют диапазон значений глубины для фреймбуфера
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		//Прямоугольник отсечения
		//Все пиксели, не входящие в диапазон этого прямоугольника, будут удалены
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = _swapChainExtent;

		//объединение информации о вьюпорте и сциссоре
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		//Настройка растеризации
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		//Если в поле depthClampEnable установить VK_TRUE, фрагменты,
		// которые находятся за пределами ближней и дальней плоскости, не отсекаются, а пододвигаются к ним.
		rasterizer.depthClampEnable = VK_FALSE;
		//Если для rasterizerDiscardEnable задать VK_TRUE, 
		// стадия растеризации отключается и выходные данные не передаются во фреймбуфер
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		//polygonMode определяет, каким образом генерируются фрагменты. Доступны следующие режимы:
		//VK_POLYGON_MODE_FILL: полигоны полностью заполняются фрагментами
		//VK_POLYGON_MODE_LINE : ребра полигонов преобразуются в отрезки
		//VK_POLYGON_MODE_POINT : вершины полигонов рисуются в виде точек
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		//задается толщина отрезков
		rasterizer.lineWidth = 1.0f;
		//Параметр cullMode определяет тип отсечения (face culling). Вы можете совсем отключить отсечение, 
		// либо включить отсечение лицевых и/или нелицевых граней. Переменная frontFace определяет порядок обхода 
		// вершин (по часовой стрелке или против) для определения лицевых граней.
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

		rasterizer.depthBiasEnable = VK_FALSE;

		//Настройка мультисэмплинга
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		//Настройки для каждого подключенного фреймбуфера
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

		//Глобальные настройки смешивания цветов
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		VkDynamicState dynamicStates[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_LINE_WIDTH
		};

		//Динамические состояния графического конвеера
		//такие состояние можно измения, не создавая конвеер заново
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = dynamicStates;

		//Настройки Layout конвейера
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		if (vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		//Настройка графического конвейера
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		//Указатель на массив структур шейдеров
		pipelineInfo.pStages = shaderStages;

		//Указатели на все структуры, описывающие непрограммируемые стадии конвейера
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr; // Optional

		//layout конвейера, который является дескриптором Vulkan, а не указателем на структуру
		pipelineInfo.layout = _pipelineLayout;

		//Ссылка на проход (render pass) и номер подпрохода (subpass), который используется в создаваемом ковейере
		pipelineInfo.renderPass = _renderPass;
		pipelineInfo.subpass = 0;

		//Создание графического конвейера
		if (vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		//Уничтожение шейдерных модулей
		vkDestroyShaderModule(_device, fragShaderModule, nullptr);
		vkDestroyShaderModule(_device, vertShaderModule, nullptr);
	}
	VkShaderModule vulkan::createShaderModule(const std::vector<char>& code)
	{
		VkShaderModule shaderModule;

		//Информация для создания шейдерного модуля
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		//Информация о шейдере
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
		
		if (vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}
		return shaderModule;
	}
	std::vector<char> vulkan::readFile(const std::string& filename)
	{
		//ate: установить указатель чтения на конец файла
		//binary: читать файл как двоичный(не использовать текстовые преобразования)
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		//Определение размера файла
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	void vulkan::createRenderPass()
	{
		//Настройка буферов(attachments)
		VkAttachmentDescription colorAttachment{};
		//Формат цветового буфера 
		colorAttachment.format = _swapChainImageFormat;
		//Количество используемых сэмплов
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

		//Настройка, что делать с данными буфера цвета и глубины перед рендерингом 
		//VK_ATTACHMENT_LOAD_OP_LOAD: буфер будет содержать те данные, 
		// которые были помещены в него до этого прохода (например, во время предыдущего прохода)
		//VK_ATTACHMENT_LOAD_OP_CLEAR: буфер очищается в начале прохода рендера
		//VK_ATTACHMENT_LOAD_OP_DONT_CARE : содержимое буфера не определено; для нас оно не имеет значения
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		//Настройка, что делать с данными буфера цвета и глубины после рендеринга
		//VK_ATTACHMENT_STORE_OP_STORE: содержимое буфера сохраняется в память для дальнейшего использования
		//VK_ATTACHMENT_STORE_OP_DONT_CARE: после рендеринга буфер больше не используется, и его содержимое не имеет значения
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		//То же самое, что и loadOp и storeOp, но отвечает за даботу данных буфера трафарета
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		//layout, в котором будет image перед началом прохода рендера
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		//layout, в который image будет автоматически переведен после завершения прохода рендера
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		//Ссылка на Attachment, необходимая для настройки подпрохода
		VkAttachmentReference colorAttachmentRef{};
		//Порядковый номер буфера в массиве, на который ссылается подпроход
		colorAttachmentRef.attachment = 0;
		//layout буфера во время подпрохода, ссылающегося на этот буфер
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		//Настройка подпрохода
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		//Настройки, для создания объекта прохода рендера
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		if (vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	

	void vulkan::createLogicalDevice()
	{
		QueueFamilyIndices indices = findQueueFamilies(_physicalDevice);

		//Список настроик очередей для для каждого из семейств
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		//Список семейств очередей
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		
		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			//Задается очередь с поддержкой графических операций
			queueCreateInfo.queueFamilyIndex = queueFamily;
			//Задается количество очередей
			queueCreateInfo.queueCount = 1;
			//Задание приоритета очереди (приоритет задается в диапазоне от 0 до 1)
			queueCreateInfo.pQueuePriorities = &queuePriority;
			
			queueCreateInfos.push_back(queueCreateInfo);
		}

		//Указание используемых возможностей устройства 
		VkPhysicalDeviceFeatures deviceFeatures{};

		//Заполнение структуры для создания логического устройства
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		//Указание информации об очередях
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		
		//Указание информации о возможностях устройства
		createInfo.pEnabledFeatures = &deviceFeatures;

		//Указание расширений, используемых устройством
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();
		
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
		//Получение дескриптора очереди отображения
		vkGetDeviceQueue(_device, indices.presentFamily.value(), 0, &_presentQueue);
	}

	void vulkan::createSurface()
	{
		//Создание surface
		if (glfwCreateWindowSurface(_instance, _window, nullptr, &_surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
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
		//Уничтожение экземпляра графического конвейера 
		vkDestroyPipeline(_device, _graphicsPipeline, nullptr);
		//Уничтожение экземпляра layout конвейера 
		vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
		//Уничтожение экземпляра объекта прохода рендера
		vkDestroyRenderPass(_device, _renderPass, nullptr);

		//Уничтожение экземпляров ImageView
		for (auto imageView : _swapChainImageViews) {
			vkDestroyImageView(_device, imageView, nullptr);
		}

		//Уничтожение экземпляра Swapchain
		vkDestroySwapchainKHR(_device, _swapChain, nullptr);
		//Уничтожение экземпляра логического устройства
		vkDestroyDevice(_device, nullptr);

#ifdef ENABLE_VALIDATION_LAYERS
		//Уничтожение экземпляра мессенджера
		DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
#endif // ENABLE_VALIDATION_LAYERS
		vkDestroySurfaceKHR(_instance, _surface, nullptr);
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
		switch (messageSeverity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: 
			std::cerr << " [VERBOSE] ";
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: 
			std::cerr << " [INFO] ";
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: 
			std::cerr << " [WARNING] ";
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: 
			std::cerr << " [ERROR] ";
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT: 
			std::cerr << " [FLAG] ";
			break;
		}

		std::cerr << "validation layer: " << pCallbackData->pMessage << "\n\n";
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