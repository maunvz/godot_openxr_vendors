#pragma once

#include <optional>

#include <godot_cpp/classes/ref_counted.hpp>

#include "extensions/openxr_fb_scene_extension_wrapper.h"

namespace godot {

class XrSceneObject : public RefCounted {
  GDCLASS(XrSceneObject, RefCounted)

public:
  XrSceneObject();
  ~XrSceneObject();

  void init(XrSceneObjectInternal state);
  void update_transform();

  String get_id();
  PackedStringArray get_labels();

  bool transform_valid();
  Transform3D get_transform();

  Rect2 get_bounding_box_2d();

  int get_bounds_2d_count();
  Vector2 get_bounds_2d_vertex(int index);

protected:
  static void _bind_methods();

private:
  XrSceneObjectInternal state_;

  XrSpaceVelocity velocity_;
  XrSpaceLocation location_;
};

} // namespace godot
