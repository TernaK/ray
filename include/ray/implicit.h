#pragma once
#include <ray/ray.h>
#include <vector>
#include <memory>

namespace ray {
  /// Implicit
  //--------------------------------------------------
  enum struct ImplicitType {
    plane, sphere, box, undefined
  };
  struct Implicit : public Transformable {
    ImplicitType type = ImplicitType::undefined;
    Material material;
    std::vector<std::shared_ptr<Implicit>> children;

    Implicit() = default;
    Implicit(ImplicitType type,
             Material material = Material());

    void add_child(std::shared_ptr<Implicit> child);
  };
}
