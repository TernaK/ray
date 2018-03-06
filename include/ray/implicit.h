#pragma once
#include <ray/ray.h>
#include <vector>
#include <memory>

namespace ray {
  /// HitTester
  //--------------------------------------------------
  struct HitTester {
    static bool test_plane(Ray& ray, Hit& hit, Transform& transform,
                           glm::vec3 plane_normal,
                           glm::vec3 offset = VEC3ZERO);

    static bool test_box(Ray& ray, Hit& hit, Transform& transform);

    static bool test_sphere(Ray& ray, Hit& hit, Transform& transform);

    //static Ray transform_ray(Ray& ray, Transform& transform);
  };

  /// Implicit
  //--------------------------------------------------
  enum struct ImplicitType {
    plane, sphere, box
  };

  struct ImplicitHit {
    bool yes = false;
    Hit hit;
    Material material;
  };

  struct Implicit : public Transformable {
    ImplicitType type;
    Material material;
    std::vector<std::shared_ptr<Implicit>> children;

    Implicit(ImplicitType type = ImplicitType::sphere,
             Material material = Material());

    ImplicitHit ray_hit_test(Ray& ray, Transform& transform);
  };
}
