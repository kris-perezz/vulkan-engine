#include "Application.h"
#include "Window.h"
#include <GLFW/glfw3.h>

namespace kopi {
  Application::Application() {

  }

  Application::~Application() {}

  void Application::run() {
    while(!m_window.shouldClose()) {
      glfwPollEvents();
    }
  }
} // namespace kopi