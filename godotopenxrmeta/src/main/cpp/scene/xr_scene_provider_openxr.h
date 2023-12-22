#pragma once

#include <functional>
#include <map>
#include <thread>

#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>

#include <openxr/openxr_extension_helpers.h>

#include "openxr/fb_spatial_entity_container.h"
#include "openxr/fb_scene.h"

#include "scene/xr_scene_object.h"
#include "utils/xr_godot_utils.h"

namespace godot {

typedef std::function<void(std::vector<XrSceneObjectInternal>)> QueryRoomCallback_t;

/**
 * A generic scene model provider, which can use OpenXR or fake / injected data
 */
class IXrSceneProvider {
public:
  // Queries available XrSpaces that represent a room
  virtual void query_room(QueryRoomCallback_t callback) = 0;
  // Populates an XrSpaceLocation for the XrSpace, based on play space
  virtual void locate_space(XrSpace space, XrSpaceLocation* location) = 0;
};

// An IXrSceneProvider powered by OpenXR extensions from Meta
class XrSceneProviderOpenXR : public OpenXRExtensionWrapperExtension, public IXrSceneProvider {
  GDCLASS(XrSceneProviderOpenXR, OpenXRExtensionWrapperExtension)
public:
	static XrSceneProviderOpenXR *get_singleton();
  XrSceneProviderOpenXR();
  virtual ~XrSceneProviderOpenXR() override;

  // OpenXRExtensionWrapper
  void _on_instance_created(uint64_t instance) override;

  // IXrSceneProvider
  void query_room(QueryRoomCallback_t callback) override;
  void locate_space(XrSpace space, XrSpaceLocation* location) override;

protected:
  static void _bind_methods();

private:
	static XrSceneProviderOpenXR *singleton;

  // Helper for on_space_query_complete
  void try_adding_output_for_uuid(XrUuidEXT uuid, const std::map<XrUuidEXT, XrSpaceQueryResultFB, XrUuidExtCmp>& from, std::vector<XrSceneObjectInternal>& objects);

  // Core OpenXR, TBD where to grab this...
  static PFN_xrLocateSpace xrLocateSpace;
};

} // namespace godot
