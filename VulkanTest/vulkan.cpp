#include "vulkan.h"

#include <cstdlib>
#include <stdexcept>
#include <iostream>
#include <vector>

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
	}

	void vulkan::initVulkan()
	{
		createInstance();
	}
	void vulkan::createInstance()
	{
		//��������� ���������� ����������, �������������� �����������
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		//��������� ������� ����������, �������������� �����������
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		std::cout << "available extensions:\n";

		for (const auto& extension : extensions) {
			std::cout << '\t' << extension.extensionName << '\n';
		}

		//���������� � ���������
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		//���������� � ���, ����� ���������� ���������� � ���� ��������� �� ����� ������������ 
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;


		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;

		//��������� ������ ����������� ���������� vulkan ��� �������������� � ������� ��������
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount); 

		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;
		createInfo.enabledLayerCount = 0;

		//�������� ���������� vulkan
		if(vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create instance!");
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
		//����������� ���������� vulkan
		vkDestroyInstance(_instance, nullptr);

		//�������� ����
		glfwDestroyWindow(_window);

		//���������� ������ GLFW
		glfwTerminate();

		_isClean = true;
	}

#pragma endregion
}