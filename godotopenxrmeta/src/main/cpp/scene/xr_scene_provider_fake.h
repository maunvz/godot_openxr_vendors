#include "xr_scene_provider_openxr.h"

namespace godot {

/**
 * A fake scene model provider, to use when OpenXR is not available
 */
class XrSceneProviderFake : public IXrSceneProvider {
public:
  void query_room(QueryRoomCallback_t callback) override;
  void locate_space(XrSpace space, XrSpaceLocation* location) override;
};

} // namespace godot