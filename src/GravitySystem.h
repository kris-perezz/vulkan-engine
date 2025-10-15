#include "Application.h"

#include "Model.h"
#include "RenderSystem.h"
// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <stdexcept>

namespace kopi {
  static inline glm::vec3 hsv2rgb(float h, float s, float v) {
    float c = v * s;
    float x = c * (1.0f - fabsf(fmodf(h * 6.0f, 2.0f) - 1.0f));
    float m = v - c;

    float r = 0, g = 0, b = 0;
    if (h < 1.0f / 6.0f) {
      r = c;
      g = x;
      b = 0;
    } else if (h < 2.0f / 6.0f) {
      r = x;
      g = c;
      b = 0;
    } else if (h < 3.0f / 6.0f) {
      r = 0;
      g = c;
      b = x;
    } else if (h < 4.0f / 6.0f) {
      r = 0;
      g = x;
      b = c;
    } else if (h < 5.0f / 6.0f) {
      r = x;
      g = 0;
      b = c;
    } else {
      r = c;
      g = 0;
      b = x;
    }

    return {r + m, g + m, b + m};
  }

  class GravityPhysicsSystem {
  public:
    GravityPhysicsSystem(float strength) : strengthGravity{strength} {}

    const float strengthGravity;

    void update(std::vector<GameObject> &objs, float dt, unsigned int substeps = 1) {
      const float stepDelta = dt / substeps;
      for (int i = 0; i < substeps; i++) {
        stepSimulation(objs, stepDelta);
      }
    }

    glm::vec2 computeForce(GameObject &fromObj, GameObject &toObj) const {
      auto offset           = fromObj.transform2d.translation - toObj.transform2d.translation;
      float distanceSquared = glm::dot(offset, offset);

      if (glm::abs(distanceSquared) < 1e-10f) {
        return {.0f, .0f};
      }

      float force =
          strengthGravity * toObj.rigidBody2d.mass * fromObj.rigidBody2d.mass / distanceSquared;
      return force * offset / glm::sqrt(distanceSquared);
    }

  private:
    void stepSimulation(std::vector<GameObject> &physicsObjs, float dt) {
      for (auto iterA = physicsObjs.begin(); iterA != physicsObjs.end(); ++iterA) {
        auto &objA = *iterA;
        for (auto iterB = iterA; iterB != physicsObjs.end(); ++iterB) {
          if (iterA == iterB) {
            continue;
          }
          auto &objB = *iterB;

          auto force = computeForce(objA, objB);
          objA.rigidBody2d.velocity += dt * -force / objA.rigidBody2d.mass;
          objB.rigidBody2d.velocity += dt * force / objB.rigidBody2d.mass;
        }
      }

      for (auto &obj : physicsObjs) {
        obj.transform2d.translation += dt * obj.rigidBody2d.velocity;
      }
    }
  };

  class Vec2FieldSystem {
  public:
    void update(const GravityPhysicsSystem &physicsSystem,
                std::vector<GameObject> &physicsObjs,
                std::vector<GameObject> &vectorField) {
      for (auto &vf : vectorField) {
        glm::vec2 direction{};
        for (auto &obj : physicsObjs) {
          direction += physicsSystem.computeForce(obj, vf);
        }

        // magnitude-based scale (keep your existing curve)
        float mag              = glm::length(direction);
        float mag01            = glm::clamp(glm::log(mag + 1.0f) / 3.0f, 0.0f, 1.0f);
        vf.transform2d.scale.x = 0.005f + 0.045f * mag01;

        // orientation
        vf.transform2d.rotation = atan2(direction.y, direction.x);

        // Colour driven only by gravity (acceleration magnitude at this point)
        float accelMag = 0.0f;
        for (auto &obj : physicsObjs) {
          glm::vec2 d = obj.transform2d.translation - vf.transform2d.translation;
          float r2    = glm::dot(d, d);
          if (r2 < 1e-10f)
            continue;
          accelMag += physicsSystem.strengthGravity * obj.rigidBody2d.mass / r2; // a = G * M / r^2
        }

        // Map accelMag -> [0,1] using a gentle log curve
        float a01 = glm::clamp(glm::log(accelMag + 1.0f) / 3.0f, 0.0f, 1.0f);

        // Gradient (blue→green→red) by acceleration only
        glm::vec3 c0 = {0.0f, 0.1f, 0.8f};
        glm::vec3 c1 = {0.0f, 1.0f, 0.0f};
        glm::vec3 c2 = {1.0f, 0.0f, 0.0f};
        vf.color =
            (a01 < 0.5f) ? glm::mix(c0, c1, a01 * 2.0f) : glm::mix(c1, c2, (a01 - 0.5f) * 2.0f);
      }
    }
  };

  std::unique_ptr<Model> createSquareModel(EngineDevice &device, glm::vec2 offset) {
    std::vector<Model::Vertex> vertices = {
        {{-0.5f, -0.5f}},
        {{0.5f, 0.5f}},
        {{-0.5f, 0.5f}},
        {{-0.5f, -0.5f}},
        {{0.5f, -0.5f}},
        {{0.5f, 0.5f}}, //
    };
    for (auto &v : vertices) {
      v.position += offset;
    }
    return std::make_unique<Model>(device, vertices);
  }

  std::unique_ptr<Model> createCircleModel(EngineDevice &device, unsigned int numSides) {
    std::vector<Model::Vertex> uniqueVertices{};
    for (int i = 0; i < numSides; i++) {
      float angle = i * glm::two_pi<float>() / numSides;
      uniqueVertices.push_back({
          {glm::cos(angle), glm::sin(angle)}
      });
    }
    uniqueVertices.push_back({});

    std::vector<Model::Vertex> vertices{};
    for (int i = 0; i < numSides; i++) {
      vertices.push_back(uniqueVertices[i]);
      vertices.push_back(uniqueVertices[(i + 1) % numSides]);
      vertices.push_back(uniqueVertices[numSides]);
    }
    return std::make_unique<Model>(device, vertices);
  }

  Application::Application() { loadGameObjects(); }

  Application::~Application() {}

  void Application::run() {

    std::shared_ptr<Model> squareModel = createSquareModel(m_device, {.5f, .0f});
    std::shared_ptr<Model> circleModel = createCircleModel(m_device, 64);

    std::vector<GameObject> physicsObjects{};
    auto red                    = GameObject::createGameObject();
    red.transform2d.scale       = glm::vec2{.05f};
    red.transform2d.translation = {.5f, .5f};
    red.color                   = {1.f, 0.f, 0.f};
    red.rigidBody2d.velocity    = {-.5f, .0f};
    red.model                   = circleModel;
    physicsObjects.push_back(std::move(red));
    auto blue                    = GameObject::createGameObject();
    blue.transform2d.scale       = glm::vec2{.05f};
    blue.transform2d.translation = {-.45f, -.25f};
    blue.color                   = {0.f, 0.f, 1.f};
    blue.rigidBody2d.velocity    = {.5f, .0f};
    blue.model                   = circleModel;
    physicsObjects.push_back(std::move(blue));

    std::vector<GameObject> vectorField{};
    int gridCount = 40;
    for (int i = 0; i < gridCount; i++) {
      for (int j = 0; j < gridCount; j++) {
        auto vf                    = GameObject::createGameObject();
        vf.transform2d.scale       = glm::vec2(0.005f);
        vf.transform2d.translation = {-1.0f + ((i + 0.5f) * 2.0f / gridCount),
                                      -1.0f + (j + 0.5f) * 2.0f / gridCount};
        vf.color                   = glm::vec3(1.0f);
        vf.model                   = squareModel;
        vectorField.push_back(std::move(vf));
      }
    }

    GravityPhysicsSystem gravitySystem{0.81f};
    Vec2FieldSystem vecFieldSystem{};

    RenderSystem m_renderSystem{m_device, m_renderer.getSwapChainRenderPass()};

    while (!m_window.shouldClose()) {
      glfwPollEvents();

      if (auto commandBuffer = m_renderer.beginFrame()) {

        gravitySystem.update(physicsObjects, 1.f / 60, 5);
        vecFieldSystem.update(gravitySystem, physicsObjects, vectorField);

        m_renderer.beginSwapChainRenderPass(commandBuffer);
        m_renderSystem.renderGameObjects(commandBuffer, physicsObjects);
        m_renderSystem.renderGameObjects(commandBuffer, vectorField);
        m_renderer.endSwapChainRenderPass(commandBuffer);
        m_renderer.endFrame();
      }
    }

    vkDeviceWaitIdle(m_device.device());
  }

  void Application::loadGameObjects() {
    std::vector<Model::Vertex> vertices{
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f},  {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    };
    auto m_model = std::make_shared<Model>(m_device, vertices);

    auto triangle                      = GameObject::createGameObject();
    triangle.model                     = m_model;
    triangle.color                     = {.1f, .8f, .1f};
    triangle.transform2d.translation.x = .2f;
    triangle.transform2d.scale         = {2.f, .5f};
    triangle.transform2d.rotation      = .25f * glm::two_pi<float>();

    m_gameObjects.push_back(std::move(triangle));
  }

} // namespace kopi