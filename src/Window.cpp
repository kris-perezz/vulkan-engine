#include "Window.h"
#include "Log.h"
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

namespace kopi {
  Window::Window(std::string name, int width, int height)
      : m_windowName(name), m_width(width), m_height(height) {
    initWindow();
  }

  Window::~Window() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
  }

  void Window::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(m_width, m_height, m_windowName.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, frameBufferResizeCallback);
  }

  bool Window::shouldClose() { return glfwWindowShouldClose(m_window); }

  VkExtent2D Window::getExtent() {
    return {static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height)};
  }

  bool Window::wasWindowResized() { return m_frameBufferResized; }

  void Window::resetWindowResizeFlag() { m_frameBufferResized = false; }

  void Window::frameBufferResizeCallback(GLFWwindow *window, int width, int height) {
    auto w = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));

    w->m_frameBufferResized = true;
    w->m_width = width;
    w->m_height = height;

  }

  void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
    if (glfwCreateWindowSurface(instance, m_window, nullptr, surface) != VK_SUCCESS) {
      LOG_ERROR("failed to create window surface");
      throw std::runtime_error("failed to create window surface");
    }
  }
} // namespace kopi