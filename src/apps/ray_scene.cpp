#include <iostream>
#include <ray/implicit.h>
#include <ray/ray_scene.h>
using namespace std;
using namespace ray;

int main(int argc, char* argv[]) {

  shared_ptr<Implicit> plane = make_shared<Implicit>(ImplicitType::plane);
  shared_ptr<Implicit> sphere = make_shared<Implicit>(ImplicitType::sphere);

  shared_ptr<RayScene> scene = make_shared<RayScene>();
  scene->add(plane);
  scene->add(sphere);

  shared_ptr<Camera> camera = make_shared<Camera>();
  camera->position = glm::vec3(0,5,5);
  
  plane->position.x = 2;
  sphere->position.x = -2;
  
  RayRenderer renderer;
  cv::Mat image = renderer.render_scene(scene, camera);
  cv::imshow("image", image);
  cv::waitKey();
}
