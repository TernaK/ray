#pragma once
#include <ray/implicit.h>
#include <opencv2/opencv.hpp>
#include <map>

namespace ray {

  /// ImplicitHit
  //--------------------------------------------------
  struct ImplicitHit {
    bool yes = false;
    Hit hit;
    Material material;
  };

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

  /// Renderable
  //--------------------------------------------------
  struct Renderable {
    std::shared_ptr<Implicit> implicit;
    Transform transform;
  };

  /// RayScene
  //--------------------------------------------------
  struct RayScene {
    std::shared_ptr<Light> light = std::make_shared<Light>(LightType::point);
    std::shared_ptr<Implicit> root = std::make_shared<Implicit>();
    glm::vec3 ambient = VEC3EYE;

    RayScene() = default;

    void add(std::shared_ptr<Implicit> implicit);

    std::vector<Renderable> traverse_scene();

    void traverse_node(std::shared_ptr<Implicit> implicit,
                       std::vector<Renderable>& rendrables,
                       Transform& p_transform);
  };

  /// RayCaster
  //--------------------------------------------------
  struct RayCaster {
    float x_near, y_near, z_near;
    glm::vec2 frame_size;
    glm::vec3 cam_position;
    glm::mat3 cam_trans;

    RayCaster(std::shared_ptr<Camera> camera);
    
    Ray make_ray(glm::vec2 px);

    bool cast_ray(Ray& ray, std::vector<Renderable>& renderables, ImplicitHit& hit);

    ImplicitHit implicit_hit_test(std::shared_ptr<Implicit> implicit,
                                  Ray& ray, Transform& transform);

  };

  /// RayRenderer
  //--------------------------------------------------
  struct RayRenderer {
    int supersample = 1;
    
    RayRenderer() = default;

    cv::Mat render_scene(std::shared_ptr<RayScene> scene, std::shared_ptr<Camera> camera);

  };
}
