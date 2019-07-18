#include "Wyvern.h"

#include <string>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include "WyvWindow.h"

using namespace wyv;

#ifdef NDEBUG
bool Wyvern::g_debug = false;
#else
bool Wyvern::g_debug = true;
#endif //NDEBUG
bool Wyvern::g_init = false;
bool Wyvern::g_throwOnError = true;
WyvCode Wyvern::g_verbosity = WYV_ERROR;
std::stack<std::pair<WyvCode, std::string>> Wyvern::g_log = std::stack<std::pair<WyvCode, std::string>>();
std::function<void(WyvCode, std::string)> Wyvern::g_logCallback = nullptr;
VkDebugUtilsMessengerEXT Wyvern::g_debugMessenger = VK_NULL_HANDLE;

VkInstance Wyvern::g_instance = 0;
VkPhysicalDevice Wyvern::g_physicalDevice = VK_NULL_HANDLE;
VkDevice Wyvern::g_device = VK_NULL_HANDLE;
VkQueue Wyvern::g_graphicsQueue = VK_NULL_HANDLE;
std::vector<const char*> Wyvern::g_desiredValidationLayers = { "VK_LAYER_KHRONOS_validation" };

void Wyvern::ErrorCallbackGlfw(int _error, const char *_message)
{
	Error(_message);
}

VKAPI_ATTR VkBool32 VKAPI_CALL Wyvern::ErrorCallbackVulkan(VkDebugUtilsMessageSeverityFlagBitsEXT _messageSeverity, VkDebugUtilsMessageTypeFlagsEXT _messageType, const VkDebugUtilsMessengerCallbackDataEXT *_pCallbackData, void *_pUserData)
{
	if (_messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		Fail("Vulkan validation layer: " + std::string(_pCallbackData->pMessage));
	else if (_messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		Warn("Vulkan validation layer: " + std::string(_pCallbackData->pMessage));
	else if (_messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		Message("Vulkan validation layer: " + std::string(_pCallbackData->pMessage));
	return VK_FALSE;
}

std::vector<const char*> Wyvern::GetValidationLayers()
{
	uint32_t validationLayerCount = 0;
	vkEnumerateInstanceLayerProperties(&validationLayerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(validationLayerCount);
	vkEnumerateInstanceLayerProperties(&validationLayerCount, availableLayers.data());

	std::vector<const char*> validationLayers;
	if (g_debug)
	{
		validationLayers = g_desiredValidationLayers;
		Message("Wyvern debug mode - checking Vulkan validation layers");
	}
	int i = 0;
	while (i < validationLayers.size())
	{
		bool canUseLayer = false;
		for (const VkLayerProperties layer : availableLayers)
		{
			if (!strcmp(validationLayers[i], layer.layerName))
			{
				canUseLayer = true;
				break;
			}
		}
		if (!canUseLayer)
		{
			Warn("Validation layer '" + std::string(validationLayers[i]) + "' unavailable");
			validationLayers.erase(validationLayers.begin() + i);
		}
		else
			i++;
	}
	return validationLayers;
}

std::vector<const char*> Wyvern::GetRequiredInstanceExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char **glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	return std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);
}

std::vector<const char*> Wyvern::GetRequiredDeviceExtensions()
{
	std::vector<const char*> result = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	return result;
}

void Wyvern::EnableVulkanDebugCallback()
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = &ErrorCallbackVulkan;

	auto callbackRegisterFunc = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(g_instance, "vkCreateDebugUtilsMessengerEXT");
	if (callbackRegisterFunc && callbackRegisterFunc(g_instance, &createInfo, nullptr, &g_debugMessenger) != VK_SUCCESS)
		Error("Couldn't create Vulkan error callback");
	Message("Created Vulkan error callback");
}

void Wyvern::Initialize()
{
	Message("Initializing Wyvern");
	if (!g_init)
	{
		g_init = true;

		if (glfwInit())
			Message("GLFW initialized");
		else
		{
			Fail("GLFW not initialized");
			return;
		}

		glfwSetErrorCallback(&ErrorCallbackGlfw);

		if (VulkanIsAvailable())
			Message("Vulkan is installed");
		else
		{
			Fail("Vulkan not installed");
			return;
		}

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Wyvern Engine";
		appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 1);
		appInfo.pEngineName = "Wyvern";
		appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 1);
		appInfo.apiVersion = VK_API_VERSION_1_1;

		std::vector<const char*> validationLayers = GetValidationLayers();

		VkInstanceCreateInfo instanceCreateInfo = {};
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pApplicationInfo = &appInfo;

		std::vector<const char*> instanceExtensions = GetRequiredInstanceExtensions();
		if (g_debug)
			instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		instanceCreateInfo.enabledExtensionCount = instanceExtensions.size();
		instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
		instanceCreateInfo.enabledLayerCount = validationLayers.size();
		instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();

		if (vkCreateInstance(&instanceCreateInfo, nullptr, &g_instance) == VK_SUCCESS)
			Message("Vulkan instance created");
		else
		{
			Fail("Vulkan instance creation failed");
			return;
		}

		if(g_debug)
			EnableVulkanDebugCallback();

		WyvWindow mainWindow("Wyvern Initial Window", 64, 64);
		VkSurfaceKHR mainSurface = mainWindow.getSurface();

		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(g_instance, &deviceCount, nullptr);
		if (!deviceCount)
		{
			Fail("No Vulkan compatible GPU detected");
			return;
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(g_instance, &deviceCount, devices.data());

		std::vector<const char*> deviceExtensions = GetRequiredDeviceExtensions();

		//Missing device checks for surface capabilities, formats and presentation modes
		int i, qFamily = -1;
		for (i = 0; i < devices.size(); i++)
		{
			uint32_t availableExtensionsCount = 0;
			vkEnumerateDeviceExtensionProperties(devices[i], nullptr, &availableExtensionsCount, nullptr);

			std::vector<VkExtensionProperties> availableExtensions(availableExtensionsCount);
			vkEnumerateDeviceExtensionProperties(devices[i], nullptr, &availableExtensionsCount, availableExtensions.data());

			bool deviceSupportsExtensions = true;
			for (const char *extension : deviceExtensions)
			{
				bool found = false;
				for (VkExtensionProperties props : availableExtensions)
				{
					if (!strcmp(props.extensionName, extension))
					{
						found = true;
						break;
					}
				}
				if (!found)
				{
					deviceSupportsExtensions = found;
					break;
				}
			}
				
			if (deviceSupportsExtensions)
			{
				uint32_t queueFamilyCount = 0;
				vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queueFamilyCount, nullptr);
				std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
				vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queueFamilyCount, queueFamilyProperties.data());

				for (int j = 0; j < queueFamilyProperties.size(); j++)
				{
					VkBool32 canPresent = false;
					vkGetPhysicalDeviceSurfaceSupportKHR(devices[i], j, mainSurface, &canPresent);

					if (queueFamilyProperties[j].queueCount > 0 && queueFamilyProperties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT && canPresent)
					{
						qFamily = j;
						break;
					}
				}

				if (qFamily >= 0)
					break;
			}

			if (i == devices.size() - 1)
			{
				i = -1;
				break;
			}
		}

		if (i >= 0)
		{
			g_physicalDevice = devices[i];
			Message("Using GPU " + std::to_string(i + 1) + " of " + std::to_string(deviceCount));
		}
		else
		{
			Fail("No physical devices found support graphics features");
			return;
		}

		VkDeviceQueueCreateInfo queueCreateInfo = {};
		float queuePriority = 1.0f;
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = qFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		VkPhysicalDeviceFeatures deviceFeatures = {};

		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = 1;
		deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
		deviceCreateInfo.enabledExtensionCount = 0;
		deviceCreateInfo.enabledExtensionCount = deviceExtensions.size();
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (vkCreateDevice(g_physicalDevice, &deviceCreateInfo, nullptr, &g_device) == VK_SUCCESS)
			Message("Vulkan logical device created");
		else
		{
			Fail("Vulkan logical device construction failed");
			return;
		}

		vkGetDeviceQueue(g_device, qFamily, 0, &g_graphicsQueue);
	}
	else
		Warn("Tried to initialize Wyvern more than once");
}

void Wyvern::Terminate()
{
	Message("Terminating Wyvern");
	if (g_init)
	{
		g_init = false;

		if (g_debug)
		{
			auto destroyCallbackFunc = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(g_instance, "vkDestroyDebugUtilsMessengerEXT");
			if (destroyCallbackFunc)
				destroyCallbackFunc(g_instance, g_debugMessenger, nullptr);
		}

		vkDestroyDevice(g_device, nullptr);
		vkDestroyInstance(g_instance, nullptr);

		glfwTerminate();
	}
	else
		Warn("Tried to terminate Wyvern without intitializing first");
}

void wyv::Wyvern::Log(WyvCode _code, std::string _message)
{
	if (_code >= g_verbosity)
	{
		if (g_logCallback)
			g_logCallback(_code, _message);
		else
			g_log.push(std::pair<WyvCode, std::string>(_code, _message));
	}
}

void Wyvern::Fail(std::string _message)
{
	Log(WYV_FAILURE, _message);
	Terminate();
	throw std::exception(_message.c_str());
}

void Wyvern::Error(std::string _message)
{
	Log(WYV_ERROR, _message);
	if(g_throwOnError)
		throw std::runtime_error(_message);
}

void Wyvern::Warn(std::string _message)
{
	Log(WYV_WARNING, _message);
}

void Wyvern::Message(std::string _message)
{
	Log(WYV_MESSAGE, _message);
}

WyvCode Wyvern::PopMessage(std::string *_message)
{
	std::pair<WyvCode, std::string> message = g_log.top();
	g_log.pop();
	if (_message)
		*_message = message.second;
	return message.first;
}

void Wyvern::PollEvents()
{
	glfwPollEvents();
}

bool Wyvern::VulkanIsAvailable()
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	return extensionCount;
}
