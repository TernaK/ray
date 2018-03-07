#pragma once
#include <ray/implicit.h>
#include <opencv2/opencv.hpp>
#include <map>
#include <thread>

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

    RayCaster(std::shared_ptr<Camera> camera, glm::vec2 frame_size);

    Ray make_camera_ray(glm::vec2 px);

    Ray make_ray(glm::vec3 from, glm::vec3 to);

    bool cast_ray(Ray& ray, std::vector<Renderable>& renderables,
                  ImplicitHit& hit);

    bool implicit_hit_test(std::shared_ptr<Implicit> implicit,
                           Ray& ray, Transform& transform,
                           ImplicitHit& ihit);

  };

  /// RayRenderer
  //--------------------------------------------------
  struct RayRenderer {
    int supersample = 1;
    std::shared_ptr<RayCaster> ray_caster;
    glm::vec3 clear_color = glm::vec3(0.3);
    
    RayRenderer() = default;

    cv::Mat render_scene(std::shared_ptr<RayScene> scene,
                         std::shared_ptr<Camera> camera);

    bool in_shadow(Hit& hit, std::shared_ptr<Light> light,
                     std::vector<Renderable>& renderables);

    cv::Vec3f shade_frag(Hit& hit, Material& material,
                         glm::vec3& scene_ambient,
                         std::shared_ptr<Camera> camera,
                         std::shared_ptr<Light> light,
                         std::vector<Renderable>& renderables);

    glm::vec3 get_light_contribution(Hit& hit, Material& material,
                                     std::shared_ptr<Camera> camera,
                                     std::shared_ptr<Light> light,
                                     std::vector<Renderable>& renderables);

  };
}
