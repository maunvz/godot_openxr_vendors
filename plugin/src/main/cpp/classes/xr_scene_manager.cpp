#include "classes/xr_scene_manager.h"

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/classes/worker_thread_pool.hpp>

#include "classes/xr_scene_provider_openxr.h"

using namespace godot;

XrSceneManager::XrSceneManager() {}

void XrSceneManager::query_room() {
  XrSceneProviderOpenXR::get_singleton()->query_room([=](std::vector<XrSceneObjectInternal> results) {
    std::string val = "query_room complete with result count: " + std::to_string(results.size());
    WARN_PRINT(String(val.c_str()));
    sceneObjects_ = results;
    emit_signal("query_room_complete");
  });
}

void XrSceneManager::load_xr_scene_object(int index, Ref<XrSceneObject> object) {
  if (index >= 0 && index < sceneObjects_.size()) {
    object->init(sceneObjects_[index]);
  } else {
    WARN_PRINT("Index out of bounds");
  }
};

int XrSceneManager::get_xr_scene_object_count() {
  return sceneObjects_.size();
};

void XrSceneManager::_bind_methods() {
	ClassDB::bind_method(D_METHOD("query_room"), &XrSceneManager::query_room);
	ClassDB::bind_method(D_METHOD("load_xr_scene_object"), &XrSceneManager::load_xr_scene_object);
	ClassDB::bind_method(D_METHOD("get_xr_scene_object_count"), &XrSceneManager::get_xr_scene_object_count);

  ADD_SIGNAL(MethodInfo("query_room_complete"));
}
