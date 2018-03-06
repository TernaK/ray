#include <ray/ray_scene.h>
using namespace std;
using namespace ray;

/// RayScene
//--------------------------------------------------
void RayScene::add(std::shared_ptr<Implicit> implicit) {
  root->add_child(implicit);
}

std::vector<TraversedImplicit> RayScene::traverse_scene() {
  vector<TraversedImplicit> traversed_implicits;
  Transform transform;
  traverse_node(root, traversed_implicits, transform);
  return traversed_implicits;
}

void RayScene::traverse_node(std::shared_ptr<Implicit> implicit,
                             std::vector<TraversedImplicit>& traversed_implicits,
                             Transform& p_transform) {
  auto transform = implicit->get_transform() << p_transform;
  if(implicit->type != ImplicitType::undefined) {
    TraversedImplicit traversed;
    traversed.implicit = implicit;
    traversed.transform = transform;
    traversed_implicits.push_back(traversed);
  }

  for(auto& child: implicit->children) {
    traverse_node(child, traversed_implicits, transform);
  }
}

/// RayCaster
//--------------------------------------------------
RayCaster::RayCaster(int max_depth, std::shared_ptr<Camera> camera,
                     glm::vec3 ambient, int supersample)
: max_depth(max_depth), ambient(ambient), supersample(supersample) {
  y_near = camera->z_near * tan(glm::radians(camera->fovy/2.0));
  x_near = camera->get_aspect_ratio() * y_near;
  z_near = camera->z_near;
  frame_size = camera->frame_size;
  cam_trans = glm::inverse(glm::mat3(camera->get_view_mat()));
  cam_position = camera->position;
}

Ray RayCaster::make_ray(glm::vec2 px) {
  float ray_dx = ((2.0 * px.x - frame_size.x) / frame_size.x) * x_near;
  float ray_dy = -((2.0 * px.y - frame_size.y) / frame_size.y) * y_near;
  Ray ray;
  ray.pt = cam_position;
  ray.dir = glm::normalize(cam_trans * glm::vec3(ray_dx, ray_dy, -z_near));
  return ray;
}
