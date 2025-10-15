#include "RenderSystem.h"
#include "glm/fwd.hpp"
#include "glm/gtc/constants.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "Application.h"
#include "GameObject.h"
#include "Log.h"
#include "Renderer.h"
#include "Window.h"
#include <GLFW/glfw3.h>

#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace kopi {

  struct SimplePushConstantData {
    alignas(16) glm::mat2 transform{1.0f};
    alignas(16) glm::vec2 offset;
    alignas(16) glm::vec3 colour;
  };

  Application::Application() { loadGameObjects(); }

  Application::~Application() {}

  void Application::run() {
    RenderSystem renderSystem(m_device, m_renderer.getSwapChainRenderPass());
    while (!m_window.shouldClose()) {
      glfwPollEvents();

      if (auto commandBuffer = m_renderer.beginFrame()) {
        m_renderer.beginSwapChainRenderPass(commandBuffer);
        renderSystem.renderGameObjects(commandBuffer, m_gameObjects);
        m_renderer.endSwapChainRenderPass(commandBuffer);
        m_renderer.endFrame();
      }
    }
    vkDeviceWaitIdle(m_device.device());
  }

  void Application::loadGameObjects() {
    std::vector<Model::Vertex> vertices{
        {.position = {0.0, -0.5}, .colour = {1.0f, 0.0f, 0.0f}},
        {.position = {0.5, 0.5},  .colour = {0.0f, 1.0f, 0.0f}},
        {.position = {-0.5, 0.5}, .colour = {0.0f, 0.0f, 1.0f}}
    };

    auto m_model = std::make_shared<Model>(m_device, vertices);

    auto triangle                      = GameObject::createGameObject();
    triangle.model                     = m_model;
    triangle.color                     = {0.1f, 0.8f, 0.1f};
    triangle.transform2d.translation.x = 0.2f;
    triangle.transform2d.scale         = {1.0f, 1.5f};
    triangle.transform2d.rotation      = 0.25f * glm::two_pi<float>();

    m_gameObjects.push_back(std::move(triangle));
  }

} // namespace kopi