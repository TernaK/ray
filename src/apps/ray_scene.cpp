#include <iostream>
#include <ray/implicit.h>
#include <ray/ray_scene.h>
using namespace std;
using namespace ray;

int main(int argc, char* argv[]) {

  std::vector<shared_ptr<Implicit>> spheres;
  for(int i = -1; i <= 1; i++) {
    for(int j = -1; j <= 1; j++) {
      if(i == 0 && j == 0) continue;
      auto sphere = std::make_shared<Implicit>(ImplicitType::sphere);
      sphere->scale = glm::vec3(1);
      sphere->position = glm::vec3(j*2.5,i*2.5,1);
      sphere->material.color = glm::vec3(fabs(j),fabs(i),0.2);
      spheres.push_back(std::move(sphere));
    }
  }

  auto box = std::make_shared<Implicit>(ImplicitType::box);
  box->material.color = glm::vec3(1,1,0.2);
  box->rotation = glm::vec3(44,44,0);
  box->position.z = 1.2;
  box->scale = glm::vec3(0.5);

  auto plane = std::make_shared<Implicit>(ImplicitType::plane);
  plane->scale = glm::vec3(6);
  plane->material.color = glm::vec3(0.2,0.2,0.85);
  plane->rotation.x = 90;
  plane->position.z = 0;

  shared_ptr<RayScene> scene = make_shared<RayScene>();
  scene->add(box);
  for(auto& s: spheres)
    scene->add(s);
  scene->add(plane);

  shared_ptr<Camera> camera = make_shared<Camera>();
  camera->frame_size = glm::vec2(640,480);
  camera->position = glm::vec3(0,0,10);
  
  RayRenderer renderer;
  renderer.supersample = 2;

  cv::Mat image = renderer.render_scene(scene, camera);

  cv::imshow("image", image);
  cv::waitKey();
}
