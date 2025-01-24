#ifndef clox_memory_h
#define clox_memory_h

#include <memory_resource>

namespace clox {

class VM;

class GCResource final : public std::pmr::memory_resource {
  VM &vm;

  void *do_allocate(std::size_t bytes, std::size_t alignment) override {
    (void)vm;
    void *p = std::pmr::new_delete_resource()->allocate(bytes, alignment);
    return p;
  }

  void do_deallocate(void *p, std::size_t bytes,
                     std::size_t alignment) override {
    std::pmr::new_delete_resource()->deallocate(p, bytes, alignment);
  }

  bool
  do_is_equal(const std::pmr::memory_resource &other) const noexcept override {
    return this == &other;
  }

public:
  explicit GCResource(VM &vm) : vm(vm) {}
};
} // namespace clox
#endif
