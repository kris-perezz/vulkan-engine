#include "Window.h"
#include <GLFW/glfw3.h>

namespace kopi
{
  Window::Window(std::string name, int width, int height) : m_windowName(name), m_width(width), m_height(height)
  {
    initWindow();
  }
  
  Window::~Window()
  {
    glfwDestroyWindow(m_window);
    glfwTerminate();
  }

  void Window::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(m_width, m_height, m_windowName.c_str(), nullptr, nullptr);
  }
  
  bool Window::shouldClose()
  {
    return glfwWindowShouldClose(m_window);
  }
}