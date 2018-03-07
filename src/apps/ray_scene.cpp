#include <iostream>
#include <ray/implicit.h>
#include <ray/ray_scene.h>
using namespace std;
using namespace ray;

int main(int argc, char* argv[]) {

  shared_ptr<Implicit> box = make_shared<Implicit>(ImplicitType::box);
  shared_ptr<Implicit> sphere = make_shared<Implicit>(ImplicitType::sphere);

  shared_ptr<RayScene> scene = make_shared<RayScene>();
  scene->add(box);
  scene->add(sphere);

  shared_ptr<Camera> camera = make_shared<Camera>();
  camera->frame_size = glm::vec2(640,480);
  camera->position = glm::vec3(0,5,5);

  box->material.color = glm::vec3(0.8,0.2,0.2);
  box->position = glm::vec3(1,0,1);
  sphere->position = glm::vec3(1,0,0);
  
  RayRenderer renderer;
  renderer.supersample = 2;
  cv::TickMeter tm;
  tm.start();
  cv::Mat image = renderer.render_scene(scene, camera);
  tm.stop();
  cout << "time: " << tm.getTimeSec() << "s" << endl;
  cv::imshow("image", image);
  cv::waitKey();
}
