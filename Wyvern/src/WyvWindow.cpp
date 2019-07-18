#include "WyvWindow.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <algorithm>

using namespace wyv;

WyvWindow::WyvWindow(std::string _title, unsigned _width, unsigned _height)
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	m_window = glfwCreateWindow(_width, _height, _title.c_str(), nullptr, nullptr);
	
	if (glfwCreateWindowSurface(Wyvern::GetInstance(), m_window, nullptr, &m_surface) == VK_SUCCESS)
		Wyvern::Message("Window '" + _title + "' created succesfully with surface");
	else
		Wyvern::Fail("Window '" + _title + "' surface creation failed");

	if (Wyvern::GetPhysicalDevice())
	{
		VkPhysicalDevice phyicalDevice = Wyvern::GetPhysicalDevice();

		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phyicalDevice, m_surface, &capabilities);

		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(phyicalDevice, m_surface, &formatCount, nullptr);
		formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(phyicalDevice, m_surface, &formatCount, formats.data());

		uint32_t modeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(phyicalDevice, m_surface, &modeCount, nullptr);
		presentModes.resize(modeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(phyicalDevice, m_surface, &modeCount, presentModes.data());

		VkSurfaceFormatKHR chosenFormat = formats[0];
		for (auto format : formats)
		{
			if (format.format == VK_FORMAT_R8G8B8A8_UNORM && format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
			{
				chosenFormat = format;
				break;
			}
		}

		VkPresentModeKHR chosenMode = VK_PRESENT_MODE_FIFO_KHR;
		for (auto mode : presentModes)
		{
			if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				chosenMode = mode;
				break;
			}
		}

		VkExtent2D chosenExtent = capabilities.currentExtent;
		if (capabilities.currentExtent.width == std::numeric_limits<uint32_t>::max())
		{
			int w = _width, h = _height;
			glfwGetWindowSize(m_window, &w, &h);

			chosenExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, (uint32_t)w));
			chosenExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, (uint32_t)h));
		}

		uint32_t imageCount = std::min(capabilities.minImageCount + 1, std::max(capabilities.maxImageCount, capabilities.minImageCount)); //maxImageCount can be 0

		VkSwapchainCreateInfoKHR swapCreateInfo = {};
		swapCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapCreateInfo.surface = m_surface;
		swapCreateInfo.minImageCount = imageCount;
		swapCreateInfo.imageFormat = chosenFormat.format;
		swapCreateInfo.imageColorSpace = chosenFormat.colorSpace;
		swapCreateInfo.imageExtent = chosenExtent;
		swapCreateInfo.imageArrayLayers = 1;
		swapCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}
	else
		Wyvern::Warn("Window '" + _title + "' created before Wyvern initialisation complete. This message will appear during Wyvern initialisation.");
}

WyvWindow::~WyvWindow()
{
	vkDestroySurfaceKHR(Wyvern::GetInstance(), m_surface, nullptr);
	glfwDestroyWindow(m_window);
}

bool WyvWindow::shouldClose() const
{
	return glfwWindowShouldClose(m_window);
}
