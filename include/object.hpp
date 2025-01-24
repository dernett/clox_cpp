#ifndef clox_object_h
#define clox_object_h

#include <cassert>
#include <cstdint>
#include <memory_resource>
#include <print>
#include <string>

namespace clox {
enum ObjType : uint8_t {
  OBJ_STRING,
};

class ObjString;

class Obj {
  ObjType type;

public:
  explicit Obj(ObjType type) : type(type) {}

  virtual ~Obj() = default;

  [[nodiscard]] ObjType getType() const { return type; }
};

class ObjString final : public Obj {
  std::pmr::string str;

public:
  using allocator_type = std::pmr::polymorphic_allocator<>;

  ObjString() : Obj(OBJ_STRING) {}
  explicit ObjString(const allocator_type &allocator)
      : Obj(OBJ_STRING), str(allocator) {}
  explicit ObjString(std::string_view sv, const allocator_type &allocator = {})
      : Obj(OBJ_STRING), str(sv, allocator) {}

  allocator_type get_allocator() const { return str.get_allocator(); }

  const std::pmr::string &getString() const { return str; }
};
} // namespace clox

#endif
