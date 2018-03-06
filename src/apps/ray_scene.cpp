#include <iostream>
#include <ray/implicit.h>
#include <ray/ray_scene.h>
using namespace std;
using namespace ray;

int main(int argc, char* argv[]) {

  shared_ptr<Implicit> plane = make_shared<Implicit>(ImplicitType::plane);

  shared_ptr<RayScene> scene = make_shared<RayScene>();
  scene->add(plane);
  auto traversed = scene->traverse_scene();

  shared_ptr<Camera> camera = make_shared<Camera>();
  camera->position = glm::vec3(0,5,5);

  int supersample = 1;
  RayCaster rc = RayCaster(camera, scene->ambient, supersample);
  Ray ray = rc.make_ray(camera->frame_size/2.0f);
}
