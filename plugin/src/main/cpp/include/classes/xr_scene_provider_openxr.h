#pragma once

#include <functional>
#include <map>
#include <thread>

#include <godot_cpp/classes/open_xr_extension_wrapper_extension.hpp>
#include <godot_cpp/classes/open_xrapi_extension.hpp>

#include "extensions/openxr_fb_scene_extension_wrapper.h"
#include "utils/xr_godot_utils.h"

namespace godot {

typedef std::function<void(std::vector<XrSceneObjectInternal>)> QueryAnchorCallback_t;

// An IXrSceneProvider powered by OpenXR extensions from Meta
class XrSceneProviderOpenXR : public OpenXRExtensionWrapperExtension {
  GDCLASS(XrSceneProviderOpenXR, OpenXRExtensionWrapperExtension)
public:
	static XrSceneProviderOpenXR *get_singleton();
  XrSceneProviderOpenXR();
  virtual ~XrSceneProviderOpenXR() override;

  // OpenXRExtensionWrapper
  void _on_instance_created(uint64_t instance) override;

  // IXrSceneProvider
  void query_room(QueryAnchorCallback_t callback);
  void locate_space(XrSpace space, XrSpaceLocation* location);

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
