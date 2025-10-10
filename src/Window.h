#pragma once

#include <string>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
namespace kopi {
  class Window {
  public:
    Window(std::string name, int width, int height);
    ~Window();

    Window(const Window &)            = delete;
    Window &operator=(const Window &) = delete;

    void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

    bool shouldClose();

    VkExtent2D getExtent();

  private:
    void initWindow();
    GLFWwindow *m_window;

    const int m_width;
    const int m_height;
    std::string m_windowName;
  };
} // namespace kopi