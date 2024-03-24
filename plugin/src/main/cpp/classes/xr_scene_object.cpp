#include "classes/xr_scene_object.h"

#include <godot_cpp/core/class_db.hpp>

#include "classes/xr_scene_provider_openxr.h"
#include "utils/xr_godot_utils.h"

using namespace godot;

void XrSceneObject::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_id"), &XrSceneObject::get_id);
	ClassDB::bind_method(D_METHOD("get_labels"), &XrSceneObject::get_labels);
	ClassDB::bind_method(D_METHOD("update_transform"), &XrSceneObject::update_transform);
	ClassDB::bind_method(D_METHOD("transform_valid"), &XrSceneObject::transform_valid);
	ClassDB::bind_method(D_METHOD("get_transform"), &XrSceneObject::get_transform);
	ClassDB::bind_method(D_METHOD("get_bounding_box_2d"), &XrSceneObject::get_bounding_box_2d);
	ClassDB::bind_method(D_METHOD("get_bounds_2d_count"), &XrSceneObject::get_bounds_2d_count);
	ClassDB::bind_method(D_METHOD("get_bounds_2d_vertex"), &XrSceneObject::get_bounds_2d_vertex);
}

XrSceneObject::XrSceneObject() {}
XrSceneObject::~XrSceneObject() {}

void XrSceneObject::init(XrSceneObjectInternal state) {
	state_ = state;
}

String XrSceneObject::get_id() {
  return String(XrGodotUtils::uuidToString(state_.uuid).c_str());
}

PackedStringArray XrSceneObject::get_labels() {
	return state_.labels;
}

void XrSceneObject::update_transform() {
  // Reset the input structs
  velocity_ = {
		XR_TYPE_SPACE_VELOCITY, // type
		nullptr, // next
		0, // velocityFlags
		{ 0.0, 0.0, 0.0 }, // linearVelocity
		{ 0.0, 0.0, 0.0 } // angularVelocity
	};

	location_ = {
		XR_TYPE_SPACE_LOCATION, // type
		&velocity_, // next
		0, // locationFlags
		{
			{ 0.0, 0.0, 0.0, 0.0 }, // orientation
			{ 0.0, 0.0, 0.0 } // position
		} // pose
	};
	XrSceneProviderOpenXR::get_singleton()->locate_space(state_.space, &location_);
}

bool XrSceneObject::transform_valid() {
  return location_.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT;
}

Transform3D XrSceneObject::get_transform() {
	return XrGodotUtils::poseToTransform(location_.pose);
}

Rect2 XrSceneObject::get_bounding_box_2d() {
	if (state_.boundingBox2D == std::nullopt) {
		return Rect2();
	}

	return Rect2(
		{state_.boundingBox2D->offset.x, state_.boundingBox2D->offset.y},
		{state_.boundingBox2D->extent.width, state_.boundingBox2D->extent.height}
	);
}

int XrSceneObject::get_bounds_2d_count() {
	if (state_.boundary2D != std::nullopt) {
		return state_.boundary2D->size();
	}
	return 0;
}

Vector2 XrSceneObject::get_bounds_2d_vertex(int index) {
	if (state_.boundary2D != std::nullopt) {
		if (index >= 0 && index < state_.boundary2D->size()) {
			return {
				(*state_.boundary2D)[index].x,
				(*state_.boundary2D)[index].y,
			};
		}
	}
	return {
		0.0f, 0.0f
	};
}
