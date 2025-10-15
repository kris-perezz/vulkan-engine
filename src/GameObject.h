#pragma once

#include "Model.h"
#include <memory>

namespace kopi {
  struct Transform2dComponent {
    glm::vec2 translation{}; // position offset
    glm::vec2 scale{1.f, 1.f};
    float rotation;

    glm::mat2 mat2() {
      const float s = glm::sin(rotation);
      const float c = glm::cos(rotation);
      glm::mat2 rotMatrix{
          {c,  s},
          {-s, c}
      };

      glm::mat2 scaleMat{
          {scale.x, .0f    },
          {.0f,     scale.y}
      };

      return rotMatrix * scaleMat;
    }
  };

  class GameObject {
  public:
    static GameObject createGameObject() {
      static uint32_t currentId = 0;
      return GameObject(currentId++);
    }
    GameObject(const GameObject &)            = delete;
    GameObject &operator=(const GameObject &) = delete;
    GameObject(GameObject &&)                 = default;
    GameObject &operator=(GameObject &&)      = default;

    uint32_t getId() { return m_id; }

    std::shared_ptr<Model> model{};
    glm::vec3 color{};
    Transform2dComponent transform2d{};

  private:
    GameObject(uint32_t id) : m_id{id} {}
    uint32_t m_id;
  };
} // namespace kopi