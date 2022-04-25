#include "vulkan.h"

#include <cstdlib>
#include <stdexcept>
#include <iostream>
#include <vector>

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
	}

	void vulkan::initVulkan()
	{
		createInstance();
	}
	void vulkan::createInstance()
	{
		//Получение количества расширений, поддерживаемых видеокартой
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		//Получение массива расширений, поддерживаемых видеокартой
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		std::cout << "available extensions:\n";

		for (const auto& extension : extensions) {
			std::cout << '\t' << extension.extensionName << '\n';
		}

		//Информация о программе
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		//Информация о том, какие глобальных расширения и слои валидации мы хотим использовать 
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;


		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;

		//Получение списка необходимых расширений vulkan для взаимодействия с оконной системой
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount); 

		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;
		createInfo.enabledLayerCount = 0;

		//Создание экземпляра vulkan
		if(vkCreateInstance(&createInfo, nullptr, &_instance) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create instance!");
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
		//Уничтожение экземпляра vulkan
		vkDestroyInstance(_instance, nullptr);

		//Закрытие окна
		glfwDestroyWindow(_window);

		//Завершение работу GLFW
		glfwTerminate();

		_isClean = true;
	}

#pragma endregion
}