#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace ray {
  const glm::mat4 MAT4EYE = glm::mat4(1.0);
  const glm::mat3 MAT3EYE = glm::mat3(1.0);
  const glm::vec3 VEC3EYE = glm::vec3(1.0);
  const glm::vec3 VEC3ZERO = glm::vec3(0.0);
  const glm::vec4 VEC4EYE = glm::vec4(1.0);
  constexpr float RAYEPSILON = 1E-4f;

  /// Transform
  //--------------------------------------------------
  struct Transform {
    glm::mat4 model = MAT4EYE;
    glm::mat3 normal = MAT4EYE;
    glm::mat4 model_inv = MAT4EYE;
    glm::mat3 normal_inv = MAT4EYE;
    Transform operator<<(const Transform& t) {
      Transform transform = *this;
      transform.model = t.model * transform.model;
      transform.normal = t.normal * transform.normal;
      transform.model_inv = glm::inverse(transform.model);
      transform.normal_inv = glm::transpose(transform.normal);
      return transform;
    }
  };

  /// Transformable
  //--------------------------------------------------
  struct Transformable {
    glm::vec3 rotation = VEC3ZERO;
    glm::vec3 position = VEC3ZERO; //degrees
    glm::vec3 scale = VEC3EYE;

    Transform get_transform(glm::mat4 p_model = MAT4EYE) {
      Transform transform;
      transform.model = glm::translate(glm::mat4(1.0), position);
      transform.model = glm::rotate(transform.model, glm::radians(rotation.x), glm::vec3(1,0,0));
      transform.model = glm::rotate(transform.model, glm::radians(rotation.y), glm::vec3(0,1,0));
      transform.model = glm::rotate(transform.model, glm::radians(rotation.z), glm::vec3(0,0,1));
      transform.model = glm::scale(transform.model, scale);
      transform.model = p_model * transform.model;
      transform.model_inv = glm::inverse(transform.model);
      transform.normal = glm::transpose(glm::inverse(glm::mat3(transform.model)));
      transform.normal_inv = glm::transpose(transform.normal);
      return transform;
    }
  };

  /// Hit
  //--------------------------------------------------
  struct Hit {
    glm::vec3 point;
    glm::vec3 normal;
    float distance;
  };

  /// Ray
  //--------------------------------------------------
  struct Ray {
    glm::vec3 origin;
    glm::vec3 direc;
    float depth;
  };

  /// Material
  //--------------------------------------------------
  struct Material {
    glm::vec3 color = glm::vec3(0.6, 0.2, 0.7);
    glm::vec3 strength = glm::vec3(0.25,1.0,0.3); //ambient, diffuse, specular
    float shininess = 32;
    float alpha = 1.0;

    Material() = default;
    Material(glm::vec3 color, float shininess = 32, float alpha = 1.0)
    : color(color), shininess(shininess), alpha(alpha) { }
  };
}
