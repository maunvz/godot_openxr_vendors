#pragma once

#include <optional>

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/callable.hpp>

#include "classes/xr_scene_object.h"
#include "extensions/openxr_fb_scene_extension_wrapper.h"

namespace godot {

/**
 * A real scene model provider, backed by OpenXR. Exposed to GDScript
 */
class XrSceneManager : public RefCounted {
  GDCLASS(XrSceneManager, RefCounted)

public:
  XrSceneManager();
  void query_room();
  void load_xr_scene_object(int index, Ref<XrSceneObject>);
  int get_xr_scene_object_count();

protected:
  static void _bind_methods();

private:
  std::vector<XrSceneObjectInternal> sceneObjects_;
};

}
