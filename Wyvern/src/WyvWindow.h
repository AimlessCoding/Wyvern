#ifndef _H_WYVWINDOW_
#define _H_WYVWINDOW_

#include <string>

#include "Wyvern.h"
#include "WyvObject.h"

class GLFWwindow;
namespace wyv
{
	class WyvWindow;
	typedef std::shared_ptr<WyvWindow> SharedWindow;
	class WyvWindow : public WyvObject
	{
		GLFWwindow *m_window;
		unsigned m_width, m_height;
		VkSurfaceKHR m_surface = VK_NULL_HANDLE;
		VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;

	public:
		WyvWindow(std::string _title, unsigned _width, unsigned _height);
		~WyvWindow();

		bool shouldClose() const;

		GLFWwindow *getGLFWWindow() const { return m_window; }
		VkSurfaceKHR getSurface() const { return m_surface; }

		static SharedWindow CreateShared(std::string _title, unsigned _width, unsigned _height) { return std::make_shared<WyvWindow>(_title, _width, _height); }
	};
}

#endif //_H_WYVWINDOW_