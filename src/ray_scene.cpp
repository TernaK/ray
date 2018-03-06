#include <ray/ray_scene.h>
using namespace std;
using namespace ray;

void RayScene::add(std::shared_ptr<Implicit> implicit) {
  root->add_child(implicit);
}

std::vector<TraversedImplicit>
RayScene::traverse_scene(std::shared_ptr<RayScene> scene) {
  vector<TraversedImplicit> traversed_implicits;
  Transform transform;
  traverse_node(scene->root, traversed_implicits, transform);
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
