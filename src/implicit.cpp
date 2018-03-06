#include <ray/implicit.h>
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
