#include "xr_scene_provider_openxr.h"

#include <algorithm>
#include <string>
#include <memory>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>

#include "utils/xr_godot_utils.h"
#include "include/openxr_fb_scene_extension_wrapper.h"
#include "include/openxr_fb_spatial_entity_extension_wrapper.h"
#include "include/openxr_fb_spatial_entity_container_extension_wrapper.h"
#include "include/openxr_fb_spatial_entity_query_extension_wrapper.h"

#define SESSION  (XrSession) get_openxr_api()->get_session()

using namespace godot;

static const uint32_t MAX_PERSISTENT_SPACES = 100;

// Base OpenXR APIs we still need
PFN_xrLocateSpace XrSceneProviderOpenXR::xrLocateSpace = nullptr;

// Singleton
XrSceneProviderOpenXR *XrSceneProviderOpenXR::singleton = nullptr;

XrSceneProviderOpenXR* XrSceneProviderOpenXR::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(XrSceneProviderOpenXR());
	}
  return singleton;
}

XrSceneProviderOpenXR::XrSceneProviderOpenXR() {
  singleton = this;
}

XrSceneProviderOpenXR::~XrSceneProviderOpenXR() {
  singleton = nullptr;
}

void XrSceneProviderOpenXR::_on_instance_created(uint64_t instance) {
  // Base OpenXR
  xrLocateSpace = (PFN_xrLocateSpace) get_openxr_api()->get_instance_proc_addr("xrLocateSpace");
}

void XrSceneProviderOpenXR::try_adding_output_for_uuid(XrUuidEXT uuid, const std::map<XrUuidEXT, XrSpaceQueryResultFB, XrUuidExtCmp>& from, std::vector<XrSceneObjectInternal>& objects) {
    if (XrGodotUtils::isUuidValid(uuid) && from.count(uuid) > 0) {
      auto result = from.at(uuid);

      // Ensure the anchor we are adding is locatable
      if (OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->is_component_supported(result.space, XR_SPACE_COMPONENT_TYPE_LOCATABLE_FB)) {
        if (!OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->is_component_enabled(result.space, XR_SPACE_COMPONENT_TYPE_LOCATABLE_FB)) {
          OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->set_component_enabled(
              result.space,
              XR_SPACE_COMPONENT_TYPE_LOCATABLE_FB,
              true,
              [uuid](const XrEventDataSpaceSetStatusCompleteFB* eventData) {
                if (!eventData || !XR_SUCCEEDED(eventData->result)) {
                  std::string err = "Unable to enable XR_SPACE_COMPONENT_TYPE_LOCATABLE_FB for XrSpace " + XrGodotUtils::uuidToString(uuid);
                  WARN_PRINT(String(err.c_str()));
                }
              });
        } else {
          std::string msg = "XR_SPACE_COMPONENT_TYPE_LOCATABLE_FB already enabled for XrSpace " + XrGodotUtils::uuidToString(uuid);
          WARN_PRINT(String(msg.c_str()));
        }
      } else {
        std::string err = "XR_SPACE_COMPONENT_TYPE_LOCATABLE_FB not supported for XrSpace " + XrGodotUtils::uuidToString(uuid);
        WARN_PRINT(String(err.c_str()));
      }

     // Grab the semantic label if we can
      auto labels = OpenXRFbSceneExtensionWrapper::get_singleton()->get_semantic_labels(result.space);

      XrSceneObjectInternal obj = {
        result.uuid,
        result.space,
        labels,
        std::nullopt,
        std::nullopt,
        std::nullopt,
      };

      // Grab the 2D or 3D shapes
      OpenXRFbSceneExtensionWrapper::get_singleton()->get_shapes(result.space, obj);

      objects.push_back(obj);
    } else {
      std::string error = "Uuid invalid or unavailable: " + XrGodotUtils::uuidToString(uuid);
      WARN_PRINT(String(error.c_str()));
    }
}

void XrSceneProviderOpenXR::query_room(QueryRoomCallback_t callback) {
  XrSpaceQueryInfoFB queryInfo = {
      XR_TYPE_SPACE_QUERY_INFO_FB,
      nullptr,
      XR_SPACE_QUERY_ACTION_LOAD_FB,
      MAX_PERSISTENT_SPACES,
      0,
      nullptr,
      nullptr};
  OpenXRFbSpatialEntityQueryExtensionWrapper::get_singleton()->query_spatial_entities((XrSpaceQueryInfoBaseHeaderFB*) &queryInfo, [=](std::vector<XrSpaceQueryResultFB> results) {
    // There is exactly 1 room space, find that first
    auto roomIt = std::find_if(results.begin(), results.end(), [](const XrSpaceQueryResultFB& result) {
      return OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->is_component_enabled(result.space, XR_SPACE_COMPONENT_TYPE_ROOM_LAYOUT_FB);
    });

    std::vector<XrSceneObjectInternal> objects;

    if (roomIt == results.end()) {
      WARN_PRINT("No room available, bailing!");
      callback(objects);
      return;
    }

    // Store results into a map by UUID for easier lookup later
    std::map<XrUuidEXT, XrSpaceQueryResultFB, XrUuidExtCmp> resultMap;
    for (const auto& result : results) {
      resultMap[result.uuid] = result;
    }

    // Get the room info: Unused for now because the same info is returned by xrGetSpaceContainerFB
    // with semantic labels (keeping as reference in case the exact layout matters layer)
    // XrRoomLayoutFB roomLayout = {XR_TYPE_ROOM_LAYOUT_FB};
    // xrGetSpaceRoomLayoutFB(SESSION, roomIt->space, &roomLayout);
    // std::vector<XrUuidEXT> wallUuids(roomLayout.wallUuidCountOutput);
    // roomLayout.wallUuidCapacityInput = wallUuids.size();
    // roomLayout.wallUuids = wallUuids.data();
    // xrGetSpaceRoomLayoutFB(SESSION, roomIt->space, &roomLayout);
    //
    // try_adding_output_for_uuid(roomLayout.floorUuid, event->requestId, objects);
    // try_adding_output_for_uuid(roomLayout.ceilingUuid, event->requestId, objects);
    // for (int i = 0; i < roomLayout.wallUuidCountOutput; i++) {
    //   try_adding_output_for_uuid(roomLayout.wallUuids[i], event->requestId, objects);
    // }

    // Get other contained anchors
    auto uuids = OpenXRFbSpatialEntityContainerExtensionWrapper::get_singleton()->get_contained_uuids(roomIt->space);

    // Add contained anchors to the output too
    for (auto uuid : uuids) {
      try_adding_output_for_uuid(uuid, resultMap, objects);
    }

    callback(objects);
  });
}

void XrSceneProviderOpenXR::locate_space(const XrSpace space, XrSpaceLocation* location) {
  XrTime display_time = (XrTime) get_openxr_api()->get_next_frame_time();
  XrSpace play_space = (XrSpace) get_openxr_api()->get_play_space();
  xrLocateSpace(space, play_space, display_time, location);
}

void XrSceneProviderOpenXR::_bind_methods() {
  ClassDB::bind_static_method("XrSceneProviderOpenXR", D_METHOD("get_singleton"), &XrSceneProviderOpenXR::get_singleton);
}
