#include "xr_godot_utils.h"

#include <iomanip>
#include <sstream>
#include <string>

using namespace godot;

bool XrGodotUtils::isUuidValid(const XrUuidEXT& uuid) {
  for (int i = 0; i < XR_UUID_SIZE_EXT; ++i) {
    if (uuid.data[i] > 0) {
      return true;
    }
  }
  return false;
}

std::string XrGodotUtils::hexStr(const uint8_t *data, int len) {
  std::stringstream ss;
  ss << std::hex;
  for(int i = 0; i < len; i++) {
    ss << std::setw(2) << std::setfill('0') << (int)data[i];
  }
  return ss.str();
}

std::string XrGodotUtils::uuidToString(XrUuidEXT uuid) {
  return hexStr(uuid.data, XR_UUID_SIZE_EXT);
}
