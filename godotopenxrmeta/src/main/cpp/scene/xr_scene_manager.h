#pragma once

#include <optional>

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/callable.hpp>

#include "xr_scene_provider_openxr.h"

namespace godot {

/**
 * A real scene model provider, backed by OpenXR. Exposed to GDScript
 */
class XrSceneManager : public RefCounted {
  GDCLASS(XrSceneManager, RefCounted)

public:
  XrSceneManager();
  bool query_room(const Callable &callback);
  void set_simulated(bool simulated);
  void load_xr_scene_object(int index, Ref<XrSceneObject>);
  int get_xr_scene_object_count();

protected:
  static void _bind_methods();

private:
  IXrSceneProvider* scene_provider;
  std::vector<XrSceneObjectInternal> sceneObjects_;
  std::optional<Callable> queryRoomCallback_;
};

}
