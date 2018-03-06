#pragma once
#include <ray/implicit.h>

namespace ray {
  struct TraversedImplicit {
    std::shared_ptr<Implicit> implicit;
    Transform transform;
  };

  struct RayScene {
    Light light = Light(LightType::point);
    std::shared_ptr<Implicit> root = std::make_shared<Implicit>();
    glm::vec3 ambient = VEC3EYE;

    RayScene() = default;

    void add(std::shared_ptr<Implicit> implicit);

    std::vector<TraversedImplicit>
    traverse_scene(std::shared_ptr<RayScene> scene);

    void traverse_node(std::shared_ptr<Implicit> implicit,
                       std::vector<TraversedImplicit>& traversed_implicits,
                       Transform& p_transform);
  };
}
