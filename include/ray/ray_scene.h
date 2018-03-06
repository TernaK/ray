#pragma once
#include <ray/implicit.h>
#include <opencv2/opencv.hpp>

namespace ray {
  struct TraversedImplicit {
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

    std::vector<TraversedImplicit> traverse_scene();

    void traverse_node(std::shared_ptr<Implicit> implicit,
                       std::vector<TraversedImplicit>& traversed_implicits,
                       Transform& p_transform);
  };

  /// RayCaster
  //--------------------------------------------------
  struct RayCaster {
    int max_depth;
    glm::vec3 ambient;
    int supersample;
    float x_near, y_near, z_near;
    glm::vec2 frame_size;
    glm::vec3 cam_position;
    glm::mat3 cam_trans;

    RayCaster(int max_depth, std::shared_ptr<Camera> camera,
              glm::vec3 ambient, int supersample = 1);

    Ray make_ray(glm::vec2 px);

  };

  /// RayRenderer
  //--------------------------------------------------
//  struct RayRenderer {
//    int max_depth = 1;
//
//    RayRenderer() = default;
//
//    cv::Mat render_scene(std::shared_ptr<RayScene> scene);
//
//  };
}
