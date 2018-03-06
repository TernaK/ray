#include <ray/implicit.h>
#include <glm/gtx/intersect.hpp>
using namespace ray;
using namespace std;

/// Implicit
//--------------------------------------------------
Implicit::Implicit(ImplicitType type, Material material)
: type(type), material(material) {

}

void Implicit::add_child(std::shared_ptr<Implicit> child) {
  children.push_back(child);
}

ImplicitHit Implicit::ray_hit_test(Ray& ray, Transform& transform) {
  ImplicitHit ihit;
  switch (type) {
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
    ihit.material = material;
  return ihit;
}

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
