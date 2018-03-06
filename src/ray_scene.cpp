#include <ray/ray_scene.h>
#include <glm/gtx/intersect.hpp>
using namespace std;
using namespace ray;

/// HitTester
//--------------------------------------------------
bool HitTester::test_plane(Ray& ray, Hit& hit, Transform& transform,
                           glm::vec3 plane_normal,
                           glm::vec3 offset) {
  bool did_hit = false;
  glm::vec3 plane_o = glm::vec3(transform.model[3]) + offset;
  glm::vec3 plane_n = glm::normalize(transform.normal * plane_normal);
  float dist;
  if(glm::intersectRayPlane(ray.pt, ray.dir, plane_o, plane_n, dist)) {
    hit.pt = ray.pt + dist * ray.dir;
    hit.norm = plane_n;
    hit.dist = dist;
    glm::vec3 plane_hit = glm::vec3(transform.model_inv *
                                    glm::vec4(hit.pt - offset, 1.0));
    glm::vec3 diff = glm::abs(plane_hit);
    if( glm::max(diff.x, max(diff.y, diff.z) ) < 1 )
      did_hit = true;
  }
  return did_hit;
}

bool HitTester::test_box(Ray& ray, Hit& hit, Transform& transform) {
  vector<pair<bool, Hit>> hit_pairs(6);
  static const vector<glm::vec3> normals = {
    glm::vec3(0,1,0), glm::vec3(0,-1,0),
    glm::vec3(-1,0,0), glm::vec3(1,0,0),
    glm::vec3(0,0,1), glm::vec3(0,0,-1),
  };
  for(int i = 0; i < 6; i++) {
    Transform plane_transform = transform;
    glm::vec3 offset = glm::vec3(plane_transform.model *
                                 glm::vec4(normals[i], 1.0));
    hit_pairs[i].first = test_plane(ray, hit_pairs[i].second,
                                    plane_transform, normals[i], offset);
  }
  auto end = std::remove_if(hit_pairs.begin(), hit_pairs.end(),
                            [](const pair<bool, Hit>& h) -> bool {
                              return h.first == false;
                            });
  hit_pairs.resize(end - hit_pairs.begin());
  std::sort(hit_pairs.begin(), hit_pairs.end(),
            [](const pair<bool, Hit>& h1, const pair<bool, Hit>& h2) -> bool {
              return h1.second.dist < h2.second.dist;
            });
  if(!hit_pairs.empty()) {
    hit = hit_pairs[0].second;
    return true;
  } else {
    return false;
  }
}

bool HitTester::test_sphere(Ray& ray, Hit& hit, Transform& transform) {
  bool did_hit = false;
  glm::vec3 p = glm::vec3(transform.model_inv * glm::vec4(ray.pt, 1.0));
  glm::vec3 d = glm::vec3(transform.normal_inv * glm::vec4(ray.dir, 1.0));
  float a = dot(d,d);
  float b = 2 * dot(p,d);
  float c = dot(p,p) - 1;
  float det = b*b - 4*a*c;
  if(det > 0) {
    float sqrt_det = sqrt(det);
    float t1 = (-b + sqrt_det) / (2*a);
    float t2 = (-b - sqrt_det) / (2*a);
    float t = min(t1, t2);
    glm::vec3 hit_sphere = p + t * d;
    if(t > 0 && fabs(glm::length(hit_sphere) - 1) < RAYEPSILON) {
      did_hit = true;
      hit.pt = ray.pt + t * ray.dir;
      hit.norm = glm::normalize(transform.normal * hit_sphere);
      hit.dist = glm::length(hit.pt - ray.pt);
    }
  }
  return did_hit;
}

/// RayScene
//--------------------------------------------------
void RayScene::add(std::shared_ptr<Implicit> implicit) {
  root->add_child(implicit);
}

std::vector<Renderable> RayScene::traverse_scene() {
  vector<Renderable> rendrables;
  Transform transform;
  traverse_node(root, rendrables, transform);
  return rendrables;
}

void RayScene::traverse_node(std::shared_ptr<Implicit> implicit,
                             std::vector<Renderable>& rendrables,
                             Transform& p_transform) {
  auto transform = implicit->get_transform() << p_transform;
  if(implicit->type != ImplicitType::undefined) {
    Renderable traversed;
    traversed.implicit = implicit;
    traversed.transform = transform;
    rendrables.push_back(traversed);
  }

  for(auto& child: implicit->children) {
    traverse_node(child, rendrables, transform);
  }
}

/// RayCaster
//--------------------------------------------------
RayCaster::RayCaster(std::shared_ptr<Camera> camera,
                     glm::vec3 ambient, int supersample)
: ambient(ambient), supersample(supersample) {
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

ImplicitHit RayCaster::ray_hit_test(std::shared_ptr<Implicit> implicit,
                                    Ray& ray, Transform& transform) {
  ImplicitHit ihit;
  switch (implicit->type) {
    case ImplicitType::plane:
      ihit.yes = HitTester::test_plane(ray, ihit.hit, transform, glm::vec3(0,1,0));
      break;

    case ImplicitType::sphere:
      ihit.yes = HitTester::test_sphere(ray, ihit.hit, transform);
      break;

    case ImplicitType::box:
      ihit.yes = HitTester::test_box(ray, ihit.hit, transform);
      break;

    default:
      break;
  }
  if(ihit.yes)
    ihit.material = implicit->material;
  return ihit;
}

ImplicitHit RayCaster::cast_ray(Ray& ray, std::vector<Renderable> renderables) {
  //get all hits
  map<float,ImplicitHit> hits;
  float zbuf = std::numeric_limits<float>::max();
  for(auto& renderable: renderables) {
    ImplicitHit hit = ray_hit_test(renderable.implicit, ray, renderable.transform);
    if(hit.yes && hit.hit.dist < zbuf) {
      zbuf = hit.hit.dist;
      hits[hit.hit.dist] = std::move(hit);
    }
  }

  //if at least one hit, use the first
  if(!hits.empty()) {
    auto best_hit = hits.begin();
    return best_hit->second;
  } else {
    ImplicitHit nohit;
    return nohit;
  }
}

