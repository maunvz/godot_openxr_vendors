/**************************************************************************/
/*  openxr_fb_spatial_entity_storage_extension_wrapper.cpp                  */
/**************************************************************************/
/*                       This file is part of:                            */
/*                              GODOT XR                                  */
/*                      https://godotengine.org                           */
/**************************************************************************/
/* Copyright (c) 2022-present Godot XR contributors (see CONTRIBUTORS.md) */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "extensions/openxr_fb_spatial_entity_storage_extension_wrapper.h"

#include "extensions/openxr_fb_spatial_entity_query_extension_wrapper.h"
#include "extensions/openxr_fb_spatial_entity_extension_wrapper.h"
#include "utils/xr_godot_utils.h"

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#define LOG_FAIL_EVENT(EVENT, ERR) if(EVENT) WARN_PRINT(String(ERR) + String(" - ") + get_openxr_api()->get_error_string(EVENT->result)); else WARN_PRINT(String(ERR) + String(" - NULL EVENT"))
#define FAILED_EVENT(EVENT) !EVENT || !XR_SUCCEEDED(EVENT->result)
#define LOG_FAIL(RESULT, ERR) WARN_PRINT(String(ERR) + String(" - ") + get_openxr_api()->get_error_string(RESULT));
#define FAILED(RESULT) !XR_SUCCEEDED(RESULT)

using namespace godot;

static const uint32_t MAX_PERSISTENT_SPACES = 100;
static const XrPosef kOriginPose =	{
	{ 0.0, 0.0, 0.0, 1.0 }, // orientation
	{ 0.0, 0.0, 0.0 } // position
};

OpenXRFbSpatialEntityStorageExtensionWrapper *OpenXRFbSpatialEntityStorageExtensionWrapper::singleton = nullptr;

OpenXRFbSpatialEntityStorageExtensionWrapper *OpenXRFbSpatialEntityStorageExtensionWrapper::get_singleton() {
	if (singleton == nullptr) {
		singleton = memnew(OpenXRFbSpatialEntityStorageExtensionWrapper());
	}
	return singleton;
}

OpenXRFbSpatialEntityStorageExtensionWrapper::OpenXRFbSpatialEntityStorageExtensionWrapper() :
		OpenXRExtensionWrapperExtension() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "An OpenXRSpatialEntityStorageExtensionWrapper singleton already exists.");

	request_extensions[XR_FB_SPATIAL_ENTITY_STORAGE_EXTENSION_NAME] = &fb_spatial_entity_storage_ext;
	singleton = this;
}

OpenXRFbSpatialEntityStorageExtensionWrapper::~OpenXRFbSpatialEntityStorageExtensionWrapper() {
	cleanup();
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("is_spatial_entity_storage_supported"), &OpenXRFbSpatialEntityStorageExtensionWrapper::is_spatial_entity_storage_supported);
	ClassDB::bind_method(D_METHOD("create_persistent_anchor"), &OpenXRFbSpatialEntityStorageExtensionWrapper::create_persistent_anchor);
	ClassDB::bind_method(D_METHOD("delete_persistent_anchor"), &OpenXRFbSpatialEntityStorageExtensionWrapper::delete_persistent_anchor);
	ClassDB::bind_method(D_METHOD("start_tracking_persistent_anchor"), &OpenXRFbSpatialEntityStorageExtensionWrapper::start_tracking_persistent_anchor);
	ClassDB::bind_method(D_METHOD("stop_tracking_persistent_anchor"), &OpenXRFbSpatialEntityStorageExtensionWrapper::stop_tracking_persistent_anchor);

	ADD_SIGNAL(MethodInfo("create_persistent_anchor", PropertyInfo(Variant::INT, "request_id"), PropertyInfo(Variant::STRING, "uuid")));
	ADD_SIGNAL(MethodInfo("create_persistent_anchor_failed", PropertyInfo(Variant::INT, "request_id")));
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::cleanup() {
	fb_spatial_entity_storage_ext = false;
}

Dictionary OpenXRFbSpatialEntityStorageExtensionWrapper::_get_requested_extensions() {
	Dictionary result;
	for (auto ext : request_extensions) {
		uint64_t value = reinterpret_cast<uint64_t>(ext.value);
		result[ext.key] = (Variant)value;
	}
	return result;
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::_on_instance_created(uint64_t instance) {
	if (fb_spatial_entity_storage_ext) {
		bool result = initialize_fb_spatial_entity_storage_extension((XrInstance)instance);
		if (!result) {
			UtilityFunctions::print("Failed to initialize fb_spatial_entity_storage extension");
			fb_spatial_entity_storage_ext = false;
		}
	}
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::_on_instance_destroyed() {
	cleanup();
}

bool OpenXRFbSpatialEntityStorageExtensionWrapper::initialize_fb_spatial_entity_storage_extension(const XrInstance &p_instance) {
	GDEXTENSION_INIT_XR_FUNC_V(xrSaveSpaceFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrEraseSpaceFB);
	GDEXTENSION_INIT_XR_FUNC_V(xrLocateSpace);
	return true;
}

bool OpenXRFbSpatialEntityStorageExtensionWrapper::_on_event_polled(const void *event) {
	if (static_cast<const XrEventDataBuffer *>(event)->type == XR_TYPE_EVENT_DATA_SPACE_SAVE_COMPLETE_FB) {
		on_space_save_complete((const XrEventDataSpaceSaveCompleteFB *)event);
		return true;
	}

	if (static_cast<const XrEventDataBuffer *>(event)->type == XR_TYPE_EVENT_DATA_SPACE_ERASE_COMPLETE_FB) {
		on_space_erase_complete((const XrEventDataSpaceEraseCompleteFB *)event);
		return true;
	}

	return false;
}

bool OpenXRFbSpatialEntityStorageExtensionWrapper::save_space(const XrSpaceSaveInfoFB *p_info, StorageRequestCompleteCallback p_callback, void *p_userdata) {
	XrAsyncRequestIdFB request_id;

	const XrResult result = xrSaveSpaceFB(SESSION, p_info, &request_id);
	if (!XR_SUCCEEDED(result)) {
		WARN_PRINT("xrSaveSpaceFB failed!");
		WARN_PRINT(get_openxr_api()->get_error_string(result));
		p_callback(result, p_info->location, p_userdata);
		return false;
	}

	requests[request_id] = RequestInfo(p_callback, p_userdata);
	return true;
}

bool OpenXRFbSpatialEntityStorageExtensionWrapper::erase_space(const XrSpaceEraseInfoFB *p_info, StorageRequestCompleteCallback p_callback, void *p_userdata) {
	XrAsyncRequestIdFB request_id;

	const XrResult result = xrEraseSpaceFB(SESSION, p_info, &request_id);
	if (!XR_SUCCEEDED(result)) {
		WARN_PRINT("xrEraseSpaceFB failed!");
		WARN_PRINT(get_openxr_api()->get_error_string(result));
		p_callback(result, p_info->location, p_userdata);
		return false;
	}

	requests[request_id] = RequestInfo(p_callback, p_userdata);
	return true;
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::on_space_save_complete(const XrEventDataSpaceSaveCompleteFB *event) {
	if (!requests.has(event->requestId)) {
		WARN_PRINT("Received unexpected XR_TYPE_EVENT_DATA_SPACE_SAVE_COMPLETE_FB");
		return;
	}

	RequestInfo *request = requests.getptr(event->requestId);
	request->callback(event->result, event->location, request->userdata);
	requests.erase(event->requestId);
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::on_space_erase_complete(const XrEventDataSpaceEraseCompleteFB *event) {
	if (!requests.has(event->requestId)) {
		WARN_PRINT("Received unexpected XR_TYPE_EVENT_DATA_SPACE_ERASE_COMPLETE_FB");
		return;
	}

	RequestInfo *request = requests.getptr(event->requestId);
	request->callback(event->result, event->location, request->userdata);
	requests.erase(event->requestId);
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::start_tracking_persistent_anchor(const String& uuid) {
	OpenXRFbSpatialEntityQueryExtensionWrapper::get_singleton()->query_spatial_entities_by_uuid(uuid, [=](Vector<XrSpaceQueryResultFB> results, void*) {
		if (results.size() == 1) {
			anchor_spaces[uuid] = results[0].space;

			// Next enable locatable
			OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->set_component_enabled(
				results[0].space, XR_SPACE_COMPONENT_TYPE_LOCATABLE_FB, true,
				[](XrResult p_result, XrSpaceComponentTypeFB p_component, bool p_enabled, void *p_userdata) {},
				nullptr);

			Ref<XRPositionalTracker> positional_tracker;
			positional_tracker.instantiate();
			positional_tracker->set_tracker_type(XRServer::TRACKER_ANCHOR);
			positional_tracker->set_tracker_name("anchor_" + uuid);
			positional_tracker->set_input("uuid", uuid);
			positional_tracker->set_pose(
				"default",
				XrGodotUtils::poseToTransform(kOriginPose),
				Vector3 {},
				Vector3 {},
				XRPose::XR_TRACKING_CONFIDENCE_HIGH);

			XRServer *xr_server = XRServer::get_singleton();
			xr_server->add_tracker(positional_tracker);
			trackers[uuid] = positional_tracker;
		} else {
			WARN_PRINT("Could not find anchor" + uuid);
		}
	});
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::stop_tracking_persistent_anchor(const String& uuid) {
	if (trackers.has(uuid)) {
		XRServer *xr_server = XRServer::get_singleton();
		xr_server->remove_tracker(trackers[uuid]);
		trackers.erase(uuid);
		anchor_spaces.erase(uuid);
	} else {
		WARN_PRINT("Trying to delete unknown tracker: " + uuid);
	}
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::create_persistent_anchor(const Transform3D &transform, int request_id) {
	// First create the anchor for this session
	OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->create_spatial_anchor(
		transform,
		[request_id, this](XrResult p_result, XrSpace p_space, const XrUuidEXT *p_uuid, void *p_userdata) {
			if (FAILED(p_result)) {
				LOG_FAIL(p_result, "Unable to create XrSpace");
				emit_signal("create_persistent_anchor_failed", request_id);
				return;
			}
			// Next enable storable
			OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->set_component_enabled(
				p_space, XR_SPACE_COMPONENT_TYPE_STORABLE_FB, true,
				[p_uuid, p_space, request_id, this](XrResult p_result, XrSpaceComponentTypeFB p_component, bool p_enabled, void *p_userdata) {
					if (FAILED(p_result)) {
						LOG_FAIL(p_result, "Unable to enable XR_SPACE_COMPONENT_TYPE_STORABLE_FB for XrSpace");
						emit_signal("create_persistent_anchor_failed", request_id);
						return;
					}

					// Finally, save the anchor
					XrSpaceSaveInfoFB saveInfo = {
						XR_TYPE_SPACE_SAVE_INFO_FB,
						nullptr,
						p_space,
						XR_SPACE_STORAGE_LOCATION_LOCAL_FB,
						XR_SPACE_PERSISTENCE_MODE_INDEFINITE_FB
					};
					OpenXRFbSpatialEntityStorageExtensionWrapper::get_singleton()->save_space(
						&saveInfo,
						[p_uuid, request_id, this](XrResult saveResult, XrSpaceStorageLocationFB p_location, void*) {
							if (FAILED(saveResult)) {
								LOG_FAIL(saveResult, "Unable to save XrSpace");
								emit_signal("create_persistent_anchor_failed", request_id);
								return;
							}
							emit_signal("create_persistent_anchor", request_id, String(XrGodotUtils::uuidToString(*p_uuid).c_str()));
					}, nullptr);
				}, nullptr);
		},
		nullptr
	);
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::delete_persistent_anchor(const String& uuid) {
	OpenXRFbSpatialEntityQueryExtensionWrapper::get_singleton()->query_spatial_entities_by_uuid(uuid, [=](Vector<XrSpaceQueryResultFB> results, void*) {
		if (results.size() == 1) {
			if (trackers.has(uuid)) {
				stop_tracking_persistent_anchor(uuid);
			}
			if (OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->is_component_enabled(
				results[0].space, XR_SPACE_COMPONENT_TYPE_STORABLE_FB
			)) {
				XrSpaceEraseInfoFB info = {
					XR_TYPE_SPACE_ERASE_INFO_FB,
					nullptr,
					results[0].space,
					XR_SPACE_STORAGE_LOCATION_LOCAL_FB, // TODO: Don't hardcode this
				};

				erase_space(&info, [](XrResult, XrSpaceStorageLocationFB, void*){}, nullptr);
			} else {
				// Enable storable so we can delete it
				OpenXRFbSpatialEntityExtensionWrapper::get_singleton()->set_component_enabled(
					results[0].space, XR_SPACE_COMPONENT_TYPE_STORABLE_FB, true,
					[results, this](XrResult p_result, XrSpaceComponentTypeFB p_component, bool p_enabled, void *p_userdata) {
						if (FAILED(p_result)) {
							LOG_FAIL(p_result, "Unable to enable XR_SPACE_COMPONENT_TYPE_STORABLE_FB for XrSpace");
							return;
						}

						XrSpaceEraseInfoFB info = {
							XR_TYPE_SPACE_ERASE_INFO_FB,
							nullptr,
							results[0].space,
							XR_SPACE_STORAGE_LOCATION_LOCAL_FB, // TODO: Don't hardcode this
						};

						erase_space(&info, [](XrResult, XrSpaceStorageLocationFB, void*){}, nullptr);
					}, nullptr);
			}
		} else {
			WARN_PRINT("Could not find anchor: " + uuid);
		}
	});
}

void OpenXRFbSpatialEntityStorageExtensionWrapper::_on_process() {
	if (!fb_spatial_entity_storage_ext) {
		return;
	}

	// Locate every known anchor and update the corresponding tracker's position
	for (auto space: anchor_spaces) {
		XrSpaceLocation location = {
			XR_TYPE_SPACE_LOCATION, // type
			nullptr, // next
			0, // locationFlags
			kOriginPose, // pose
		};
		XrResult result = xrLocateSpace(
			space.value,
			(XrSpace) get_openxr_api()->get_play_space(),
			get_openxr_api()->get_next_frame_time(),
			&location);

		trackers[space.key]->set_pose(
			"default",
			XrGodotUtils::poseToTransform(location.pose),
			Vector3 {},
			Vector3 {},
			XRPose::XR_TRACKING_CONFIDENCE_HIGH);
	}
}
