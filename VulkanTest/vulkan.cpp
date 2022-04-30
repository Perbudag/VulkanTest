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
		//�������� �����������, ��� ������ ������������� ����������� (surface)
		createSurface();
		//����� ����������� ���������� ��� ������ vulkan
		pickPhysicalDevice();
		//�������� ����������� ����������
		createLogicalDevice();
		//�������� Swap �hain
		createSwapChain();
		//�������� ImageViews
		createImageViews();
		//�������� ������� ������� (��������� �������)
		createRenderPass();
		//�������� ������������ ��������
		createGraphicsPipeline();
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
		//��������� ���������� �������������� ���������� �� ����������
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		//��������� ���������� � �������������� ����������� �� ����������
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
		//����� ���� ����������� ���������� ��� �������� Swap �hain
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(_physicalDevice);

		//��������� ������� ������ surface
		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		//��������� ������ ������
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		//��������� swap extent
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		//���������� ������� �������� image ������ ���� � swap chain
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		//��������� ��� �������� Swap chain
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = _surface;

		//���������� ��� �������� image ��������
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = findQueueFamilies(_physicalDevice);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		//���� ��� ������� ��������� image � �������� �� ���������� ��������:
		//VK_SHARING_MODE_EXCLUSIVE: ������ ����������� ������ ��������� ��������, 
		// � ����� �������� ������ ���� �������� ���� ����� �������������� ��� � ������ ��������� ��������. 
		// ����� ������ ������������ ����� ������� ������������������.
	    //VK_SHARING_MODE_CONCURRENT: ������� ����� �������������� � ���������� ���������� �������� ��� ����� 
		// �������� ����� ��������.
		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		//����������, ����� �������������� ����� ��������� � ����������� � swap chain
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		//���������, ����� �� ������������ ����� - ����� ��� ���������� � ������� ������ � ������� �������
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		//���������, ��� �� ����� ��������� ������� ������� (��������, ���� ����� ������ ���� ��������� ������ �����)
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = VK_NULL_HANDLE;

		//�������� swap �hain
		if (vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapChain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain!");
		}

		//��������� ������������ Image
		vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, nullptr);
		_swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, _swapChainImages.data());

		_swapChainImageFormat = surfaceFormat.format;
		_swapChainExtent = extent;
	}
	SwapChainSupportDetails vulkan::querySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;

		//��������� ������� ���������� (capabilities) surface, 
		//����� ��� ���/���� ����� ����������� � swap chain, ���/���� ������ � ������ �����������
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &details.capabilities);

		//��������� ���������� �������������� �������� surface
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, nullptr);

		if (formatCount != 0) {
			//��������� ���������� � �������������� �������� surface
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, details.formats.data());
		}

		//��������� ���������� ��������� ������� ������ surface
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			//��������� ���������� � ��������� ������� ������ surface
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
		//��� ��������� ������ ������ � vulkan
		//VK_PRESENT_MODE_IMMEDIATE_KHR: �����������, ������������ ����� �����������, ���������� ������������ �� �����, 
		// ��� ����� ��������� � ����������.
		//VK_PRESENT_MODE_FIFO_KHR : ����������� ��� ������ �� ����� ������� �� ������ ������� � ������ ���������� ������.
		// � �� �����, ��� ��������� �������� ������������� ����������� � ����� �������.���� ������� ���������, ��������� 
		// ����� �����.��� ������ �� ������������ �������������, ������������ � ����������� �����.
		//VK_PRESENT_MODE_FIFO_RELAXED_KHR : ���� ����� ���������� �� ����������� ������ � ����� ������, ����� ���������� 
		// �������� ��������� � � ������ ���������� ������ �������� ������ �������.����� ����������� ���������� �� ����� 
		// ����� ����� ��� ��������� ��� �������� ���������� ������.��� ����� �������� � ������� ����������.
		//VK_PRESENT_MODE_MAILBOX_KHR : ��� ��� ���� ������� ������� ������.������ ����, ����� ����������� ��������� ��� 
		// ���������� �������, ����������� � ������� ���������� ������.���� ����� �������� ��� ���������� ������� �����������.
		// � ��� �� ������ �������� ��������� ���������� ��� ������ ������� ��������.

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
			//��������� ������� ����������� � ��������
			int width, height;
			glfwGetFramebufferSize(_window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			//����������� ������� ����������� � ��������  minImageExtent � maxImageExtent
			actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}
	
	void vulkan::createImageViews()
	{
		_swapChainImageViews.resize(_swapChainImages.size());

		for (size_t i = 0; i < _swapChainImages.size(); i++) {
			//��������� ��� �������� image view
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = _swapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = _swapChainImageFormat;

			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			//���� subresourceRange ���������, ����� ����� image ����� ��������������.
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
		//�������� �������� �� ������
		auto vertShaderCode = readFile("shaders/vert.spv");
		auto fragShaderCode = readFile("shaders/frag.spv");

		//�������� ��������� �������
		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

		//���������� ��� ���������� ������������ �������� � ����������� �������
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		//���������� ��� ���������� ������������ �������� � ������������ �������
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		//��������� ������� ������ ������
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

		//�����������: ����� ��������� ���������� �� ������ � 
		// �������� �� ������� ��������� ��� ����� ���������, ��� line strip � triangle strip
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		//VK_PRIMITIVE_TOPOLOGY_POINT_LIST: ��������� �������������� � ���� ��������� �����, 
		// ������ ������� � ��������� �����
		//VK_PRIMITIVE_TOPOLOGY_LINE_LIST: ��������� �������������� � ���� ������ ��������, ������ 
		// ���� ������ �������� ��������� �������
		//VK_PRIMITIVE_TOPOLOGY_LINE_STRIP : ��������� �������������� � ���� ����������� �������, 
		// ������ ����������� ������� ��������� � ������� ���� �������
		//VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST : ��������� �������������� ��� ����� �������������, 
		// ������ ������ 3 ������� �������� ����������� �����������
		//VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP : ��������� �������������� ��� ����� ��������� �������������, 
		// ������ ��� ��������� ������� ����������� ������������ ������������ � �������� ���� ������ ������ ��� 
		// ���������� ������������
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		//������� ��������� ������� �����������, � ������� ���������� �������� ������
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)_swapChainExtent.width;
		viewport.height = (float)_swapChainExtent.height;
		//minDepth � maxDepth ���������� �������� �������� ������� ��� �����������
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		//������������� ���������
		//��� �������, �� �������� � �������� ����� ��������������, ����� �������
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = _swapChainExtent;

		//����������� ���������� � �������� � ��������
		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		//��������� ������������
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		//���� � ���� depthClampEnable ���������� VK_TRUE, ���������,
		// ������� ��������� �� ��������� ������� � ������� ���������, �� ����������, � ������������� � ���.
		rasterizer.depthClampEnable = VK_FALSE;
		//���� ��� rasterizerDiscardEnable ������ VK_TRUE, 
		// ������ ������������ ����������� � �������� ������ �� ���������� �� ����������
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		//polygonMode ����������, ����� ������� ������������ ���������. �������� ��������� ������:
		//VK_POLYGON_MODE_FILL: �������� ��������� ����������� �����������
		//VK_POLYGON_MODE_LINE : ����� ��������� ������������� � �������
		//VK_POLYGON_MODE_POINT : ������� ��������� �������� � ���� �����
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		//�������� ������� ��������
		rasterizer.lineWidth = 1.0f;
		//�������� cullMode ���������� ��� ��������� (face culling). �� ������ ������ ��������� ���������, 
		// ���� �������� ��������� ������� �/��� ��������� ������. ���������� frontFace ���������� ������� ������ 
		// ������ (�� ������� ������� ��� ������) ��� ����������� ������� ������.
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

		rasterizer.depthBiasEnable = VK_FALSE;

		//��������� ���������������
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		//��������� ��� ������� ������������� �����������
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

		//���������� ��������� ���������� ������
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

		//������������ ��������� ������������ ��������
		//����� ��������� ����� �������, �� �������� ������� ������
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = dynamicStates;

		//��������� Layout ���������
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		if (vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		//��������� ������������ ���������
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		//��������� �� ������ �������� ��������
		pipelineInfo.pStages = shaderStages;

		//��������� �� ��� ���������, ����������� ����������������� ������ ���������
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr; // Optional

		//layout ���������, ������� �������� ������������ Vulkan, � �� ���������� �� ���������
		pipelineInfo.layout = _pipelineLayout;

		//������ �� ������ (render pass) � ����� ���������� (subpass), ������� ������������ � ����������� ��������
		pipelineInfo.renderPass = _renderPass;
		pipelineInfo.subpass = 0;

		//�������� ������������ ���������
		if (vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		//����������� ��������� �������
		vkDestroyShaderModule(_device, fragShaderModule, nullptr);
		vkDestroyShaderModule(_device, vertShaderModule, nullptr);
	}
	VkShaderModule vulkan::createShaderModule(const std::vector<char>& code)
	{
		VkShaderModule shaderModule;

		//���������� ��� �������� ���������� ������
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		//���������� � �������
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
		
		if (vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}
		return shaderModule;
	}
	std::vector<char> vulkan::readFile(const std::string& filename)
	{
		//ate: ���������� ��������� ������ �� ����� �����
		//binary: ������ ���� ��� ��������(�� ������������ ��������� ��������������)
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		//����������� ������� �����
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	void vulkan::createRenderPass()
	{
		//��������� �������(attachments)
		VkAttachmentDescription colorAttachment{};
		//������ ��������� ������ 
		colorAttachment.format = _swapChainImageFormat;
		//���������� ������������ �������
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

		//���������, ��� ������ � ������� ������ ����� � ������� ����� ����������� 
		//VK_ATTACHMENT_LOAD_OP_LOAD: ����� ����� ��������� �� ������, 
		// ������� ���� �������� � ���� �� ����� ������� (��������, �� ����� ����������� �������)
		//VK_ATTACHMENT_LOAD_OP_CLEAR: ����� ��������� � ������ ������� �������
		//VK_ATTACHMENT_LOAD_OP_DONT_CARE : ���������� ������ �� ����������; ��� ��� ��� �� ����� ��������
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		//���������, ��� ������ � ������� ������ ����� � ������� ����� ����������
		//VK_ATTACHMENT_STORE_OP_STORE: ���������� ������ ����������� � ������ ��� ����������� �������������
		//VK_ATTACHMENT_STORE_OP_DONT_CARE: ����� ���������� ����� ������ �� ������������, � ��� ���������� �� ����� ��������
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

		//�� �� �����, ��� � loadOp � storeOp, �� �������� �� ������ ������ ������ ���������
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		//layout, � ������� ����� image ����� ������� ������� �������
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		//layout, � ������� image ����� ������������� ��������� ����� ���������� ������� �������
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		//������ �� Attachment, ����������� ��� ��������� ����������
		VkAttachmentReference colorAttachmentRef{};
		//���������� ����� ������ � �������, �� ������� ��������� ���������
		colorAttachmentRef.attachment = 0;
		//layout ������ �� ����� ����������, ������������ �� ���� �����
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		//��������� ����������
		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		//���������, ��� �������� ������� ������� �������
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

		//������ �������� �������� ��� ��� ������� �� ��������
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		//������ �������� ��������
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		
		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			//�������� ������� � ���������� ����������� ��������
			queueCreateInfo.queueFamilyIndex = queueFamily;
			//�������� ���������� ��������
			queueCreateInfo.queueCount = 1;
			//������� ���������� ������� (��������� �������� � ��������� �� 0 �� 1)
			queueCreateInfo.pQueuePriorities = &queuePriority;
			
			queueCreateInfos.push_back(queueCreateInfo);
		}

		//�������� ������������ ������������ ���������� 
		VkPhysicalDeviceFeatures deviceFeatures{};

		//���������� ��������� ��� �������� ����������� ����������
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		//�������� ���������� �� ��������
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		
		//�������� ���������� � ������������ ����������
		createInfo.pEnabledFeatures = &deviceFeatures;

		//�������� ����������, ������������ �����������
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();
		
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
		//��������� ����������� ������� �����������
		vkGetDeviceQueue(_device, indices.presentFamily.value(), 0, &_presentQueue);
	}

	void vulkan::createSurface()
	{
		//�������� surface
		if (glfwCreateWindowSurface(_instance, _window, nullptr, &_surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
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
		//����������� ���������� ������������ ��������� 
		vkDestroyPipeline(_device, _graphicsPipeline, nullptr);
		//����������� ���������� layout ��������� 
		vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
		//����������� ���������� ������� ������� �������
		vkDestroyRenderPass(_device, _renderPass, nullptr);

		//����������� ����������� ImageView
		for (auto imageView : _swapChainImageViews) {
			vkDestroyImageView(_device, imageView, nullptr);
		}

		//����������� ���������� Swapchain
		vkDestroySwapchainKHR(_device, _swapChain, nullptr);
		//����������� ���������� ����������� ����������
		vkDestroyDevice(_device, nullptr);

#ifdef ENABLE_VALIDATION_LAYERS
		//����������� ���������� �����������
		DestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
#endif // ENABLE_VALIDATION_LAYERS
		vkDestroySurfaceKHR(_instance, _surface, nullptr);
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