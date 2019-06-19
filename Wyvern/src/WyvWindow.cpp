#include "WyvWindow.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

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
