#ifndef _H_WYVERN_
#define _H_WYVERN_

#include <stack>
#include <functional>
#include <vector>

#include "vulkan/vulkan.h"

//#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
//#include "glm/glm.hpp"

namespace wyv
{
	enum WyvCode { WYV_MESSAGE, WYV_DEBUG, WYV_WARNING, WYV_ERROR, WYV_FAILURE };
	class Wyvern
	{
		static bool g_init, g_throwOnError, g_debug;
		static WyvCode g_verbosity;
		static std::stack<std::pair<WyvCode, std::string>> g_log;
		static std::function<void(WyvCode, std::string)> g_logCallback;

		static VkInstance g_instance;
		static VkPhysicalDevice g_physicalDevice;
		static VkDevice g_device;
		static VkQueue g_graphicsQueue;

		static std::vector<const char*> g_desiredValidationLayers;
		static VkDebugUtilsMessengerEXT g_debugMessenger;

		Wyvern() {}
		~Wyvern() {}

		static void ErrorCallbackGlfw(int _error, const char *_message);
		static VKAPI_ATTR VkBool32 VKAPI_CALL ErrorCallbackVulkan(VkDebugUtilsMessageSeverityFlagBitsEXT _messageSeverity, VkDebugUtilsMessageTypeFlagsEXT _messageType, const VkDebugUtilsMessengerCallbackDataEXT *_pCallbackData, void *_pUserData);

		static std::vector<const char*> GetValidationLayers();
		static std::vector<const char*> GetRequiredInstanceExtensions();
		static std::vector<const char*> GetRequiredDeviceExtensions();
		static void EnableVulkanDebugCallback();

	public:

		static void Initialize();
		static void Terminate();

		static void Log(WyvCode _code, std::string _message);
		static void Fail(std::string _message);
		static void Error(std::string _message);
		static void Warn(std::string _message);
		static void Message(std::string _message);
		static WyvCode PopMessage(std::string *_message = nullptr);
		static void SetMessageCallback(std::function<void(WyvCode, std::string)> _callback) { g_logCallback = _callback; }
		static void SetLogVerbosity(WyvCode _verbosity) { g_verbosity = _verbosity; }
		static void SetThrowOnError(bool _throw) { g_throwOnError = _throw; }
		static void SetDebugMode(bool _debug) { g_debug = _debug; }

		static void PollEvents();

		static bool VulkanIsAvailable();

		static VkInstance GetInstance() { return g_instance; }
		static VkPhysicalDevice GetPhysicalDevice() { return g_physicalDevice; }
	};
}

#endif //_H_WYVERN