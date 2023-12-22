#include "xr_scene_manager.h"

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/classes/worker_thread_pool.hpp>
#include "xr_scene_provider_fake.h"

using namespace godot;

XrSceneManager::XrSceneManager() {
  scene_provider = XrSceneProviderOpenXR::get_singleton();
}

void XrSceneManager::set_simulated(bool simulated) {
  if (simulated) {
    scene_provider = memnew(XrSceneProviderFake);
  } else {
    scene_provider = XrSceneProviderOpenXR::get_singleton();
  }
}

bool XrSceneManager::query_room(const Callable &callback) {
  if (queryRoomCallback_ != std::nullopt) {
    WARN_PRINT("Already querying!");
    return false;
  }
  queryRoomCallback_ = Callable(callback);

  scene_provider->query_room([=](std::vector<XrSceneObjectInternal> results) {
    std::string val = "query_room complete with result count: " + std::to_string(results.size());
    WARN_PRINT(String(val.c_str()));
    sceneObjects_ = results;
    WorkerThreadPool::get_singleton()->add_task(Callable(*queryRoomCallback_));
    queryRoomCallback_.reset();
  });

  return true;
}

void XrSceneManager::load_xr_scene_object(int index, Ref<XrSceneObject> object) {
  if (index >= 0 && index < sceneObjects_.size()) {
    object->init(sceneObjects_[index], scene_provider);
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
	ClassDB::bind_method(D_METHOD("set_simulated"), &XrSceneManager::set_simulated);
}
