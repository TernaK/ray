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
  glm::vec3 plane_o = glm::vec3(transform.model * glm::vec4(offset, 1.0));
  glm::vec3 plane_n = glm::normalize(transform.normal * plane_normal);
  float dist;
  if(glm::intersectRayPlane(ray.pt, ray.dir, plane_o, plane_n, dist)) {
    if(dist < 0) return false;
    hit.pt = ray.pt + dist * ray.dir;
    hit.norm = plane_n;
    hit.dist = dist;
    glm::vec3 plane_hit = glm::vec3(transform.model_inv *
                                    glm::vec4(hit.pt, 1.0));
    //along the normal's axis, that coordinate = 0, hence other two must < 1
    //in the plane's coordinate space
    glm::vec3 diff = glm::abs(plane_hit);
    if( glm::max(diff.x, max(diff.y, diff.z) ) < (1 + RAYEPSILON) )
      did_hit = true;
  }
  return did_hit;
}

bool HitTester::test_box(Ray& ray, Hit& hit, Transform& transform) {
  //use the normals as both normals and plane positions
  //with respect to the box center
  static const vector<glm::vec3> normals = {
    glm::vec3(0,1,0), glm::vec3(0,-1,0),
    glm::vec3(-1,0,0), glm::vec3(1,0,0),
    glm::vec3(0,0,1), glm::vec3(0,0,-1),
  };

  map<float, Hit> hit_pairs;
  for(int i = 0; i < normals.size(); i++) {
    glm::vec3 offset = normals[i];
    Hit plane_hit;
    if(test_plane(ray, plane_hit, transform, normals[i], offset)) {
      hit_pairs[plane_hit.dist] = plane_hit;
    }
  }

  if(!hit_pairs.empty()) {
    hit = hit_pairs.begin()->second;
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
RayCaster::RayCaster(std::shared_ptr<Camera> camera, glm::vec2 frame_size) {
  y_near = camera->z_near * tan(glm::radians(camera->fovy/2.0));
  x_near = camera->get_aspect_ratio() * y_near;
  z_near = camera->z_near;
  this->frame_size = frame_size;
  cam_trans = glm::inverse(glm::mat3(camera->get_view_mat()));
  cam_position = camera->position;
}

Ray RayCaster::make_camera_ray(glm::vec2 px) {
  float ray_dx = ((2.0 * px.x - frame_size.x) / frame_size.x) * x_near;
  float ray_dy = -((2.0 * px.y - frame_size.y) / frame_size.y) * y_near;
  Ray ray;
  ray.pt = cam_position;
  ray.dir = glm::normalize(cam_trans * glm::vec3(ray_dx, ray_dy, -z_near));
  return ray;
}

Ray RayCaster::make_ray(glm::vec3 from, glm::vec3 to) {
  Ray ray;
  ray.pt = std::move(from);
  ray.dir = glm::normalize(to - from);
  return ray;
}

bool RayCaster::implicit_hit_test(std::shared_ptr<Implicit> implicit,
                                         Ray& ray, Transform& transform,
                                         ImplicitHit& ihit) {
  bool did_hit = false;
  Hit hit;
  switch (implicit->type) {
    case ImplicitType::plane:
      did_hit = HitTester::test_plane(ray, hit, transform, glm::vec3(0,1,0));
      break;

    case ImplicitType::sphere:
      did_hit = HitTester::test_sphere(ray, hit, transform);
      break;

    case ImplicitType::box:
      did_hit = HitTester::test_box(ray, hit, transform);
      break;

    default:
      break;
  }
  if(did_hit) {
    ihit.hit = hit;
    ihit.material = implicit->material;
    ihit.yes = true;
  }
  return did_hit;
}

bool RayCaster::cast_ray(Ray& ray, std::vector<Renderable>& renderables, ImplicitHit& hit) {
  //get all hits
  map<float,ImplicitHit> hits;
  float zbuf = std::numeric_limits<float>::max();
  for(auto& renderable: renderables) {
    ImplicitHit ihit;
    bool did_hit = implicit_hit_test(renderable.implicit, ray, renderable.transform, ihit);
    if(did_hit && ihit.hit.dist < zbuf) {
      zbuf = ihit.hit.dist;
      hits[hit.hit.dist] = std::move(ihit);
    }
  }

  //if at least one hit, use the first
  if(!hits.empty()) {
    hit = hits.begin()->second;
    return true;
  }
  
  return false;
}

/// RayRenderer
//--------------------------------------------------
glm::vec3 RayRenderer::get_light_contribution(Hit& hit,
                                              Material& material,
                                              glm::vec3& eye,
                                              std::shared_ptr<Light> light,
                                              std::vector<Renderable>& renderables) {
  bool is_in_shadow = in_shadow(hit, light, renderables);
  glm::vec3 color = glm::vec3(0);
  //if in shadow, then use scene ambient color
  if(!is_in_shadow) {
    glm::vec3 l_vec = glm::normalize(light->position - hit.pt);
    glm::vec3 v_vec = glm::normalize(eye - hit.pt);
    float dist = glm::length(light->position - hit.pt);
    float attenuation = 1.0/(light->attenuation.x +
                             light->attenuation.y * dist +
                             light->attenuation.z * dist * dist);
    float cos_t = 0;
    cos_t = glm::dot(l_vec, hit.norm);
    cos_t = cos_t < 0 ? 0 : cos_t;
    glm::vec3 diffuse = material.strength.y * light->color * cos_t;
    glm::vec3 r = glm::reflect(-l_vec, hit.norm);
    float spec = glm::dot(r, v_vec);
    spec = spec < 0 ? 0 : spec;
    glm::vec3 specular = material.strength.z * light->color * pow(spec, material.shininess);
    color = attenuation * (diffuse + specular) * material.color;
  }
  return color;
}

glm::vec3 RayRenderer::shade_frag(Hit& hit, Material& material,
                                  glm::vec3& scene_ambient,
                                  glm::vec3& eye, shared_ptr<Light> light,
                                  std::vector<Renderable>& renderables,
                                  int depth) {
  //ambient
  glm::vec3 color = material.strength.x * scene_ambient * material.color;

  //diffuse + specular
  color += get_light_contribution(hit, material, eye, light, renderables);

  // reflection
  if(depth < max_depth) {
    glm::vec3 v_vec = glm::normalize(eye - hit.pt);
    Ray reflect_ray;
    reflect_ray.dir = glm::reflect(-v_vec, hit.norm);;
    reflect_ray.pt = hit.pt;
    ImplicitHit ihit;
    if(ray_caster->cast_ray(reflect_ray, renderables, ihit)) {
      float attenuation = 1.0f/(reflect_attenuation.x +
                                reflect_attenuation.y * ihit.hit.dist +
                                reflect_attenuation.z * ihit.hit.dist * ihit.hit.dist);
      glm::vec3 reflect_color = shade_frag(ihit.hit, ihit.material, scene_ambient,
                                           hit.pt, light, renderables, ++depth);
      color += attenuation * material.reflec * reflect_color;
    }
  }
  
  return color;
}

bool RayRenderer::in_shadow(Hit& hit, std::shared_ptr<Light> light,
                              std::vector<Renderable>& renderables) {
  //cast a ray to the light
  Ray ray_to_light = ray_caster->make_ray(hit.pt, light->position);
  bool is_in_light = true;
  for(auto& renderable: renderables) {
    ImplicitHit ihit;
    bool did_hit = ray_caster->implicit_hit_test(renderable.implicit,
                                                 ray_to_light,
                                                 renderable.transform,
                                                 ihit);
    // if there is any hit then shadow
    if(did_hit && (ihit.hit.dist > RAYEPSILON)) {
      //avoid self hit
      is_in_light = false;
      break;
    }
  }
  return !is_in_light;
}

cv::Mat RayRenderer::render_scene(shared_ptr<RayScene> scene, shared_ptr<Camera> camera) {
  vector<Renderable> renderables = scene->traverse_scene();
  
  cv::Size frame_size(camera->frame_size.x, camera->frame_size.y);
  glm::vec3 ambient = scene->ambient;
  cv::Mat frame(frame_size * supersample,
                CV_32FC3, {clear_color.r, clear_color.g, clear_color.b});
  
  ray_caster = make_shared<RayCaster>(camera, glm::vec2(frame.cols, frame.rows));

  // cast ray and shade for each pixel
  auto handle_ray = [&](int row, int col) {
    cv::Vec3f& frag = frame.at<cv::Vec3f>(cv::Point(col,row));
    Ray ray = ray_caster->make_camera_ray(glm::vec2(col,row));
    ImplicitHit hit;
    if(ray_caster->cast_ray(ray, renderables, hit)) {
      glm::vec3 frag_color = shade_frag(hit.hit, hit.material, ambient,
                                        camera->position, scene->light, renderables);
      frag = cv::Vec3f(frag_color.b, frag_color.g, frag_color.r);
    }
  };

  frame.forEach<cv::Vec3f>([&](cv::Vec3f frag, const int* loc){
    handle_ray(loc[0], loc[1]);
  });

  cv::resize(frame, frame, frame_size);
  cv::GaussianBlur(frame, frame, cv::Size(3,3), 0.5);
  return frame;
}
