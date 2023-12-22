#pragma once

#include <optional>

#include <godot_cpp/classes/ref_counted.hpp>

#include <openxr/openxr_extension_helpers.h>
#include "openxr/fb_spatial_entity.h"
#include "openxr/fb_scene.h"

#include "include/openxr_fb_scene_extension_wrapper.h"

namespace godot {

class IXrSceneProvider;

class XrSceneObject : public RefCounted {
  GDCLASS(XrSceneObject, RefCounted)

public:
  XrSceneObject();
  ~XrSceneObject();

  void init(XrSceneObjectInternal state, IXrSceneProvider* provider);
  void update_transform();

  String get_id();
  String get_label();

  bool transform_valid();
  Transform3D get_transform();

  Rect2 get_bounding_box_2d();

  int get_bounds_2d_count();
  Vector2 get_bounds_2d_vertex(int index);

protected:
  static void _bind_methods();

private:
  IXrSceneProvider* provider_;
  XrSceneObjectInternal state_;

  XrSpaceVelocity velocity_;
  XrSpaceLocation location_;
};

} // namespace godot
