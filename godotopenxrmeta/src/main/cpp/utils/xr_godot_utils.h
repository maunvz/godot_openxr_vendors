#pragma once

#include <string>

#include <openxr/openxr_extension_helpers.h>

namespace godot {

class XrGodotUtils {
  public:
    static bool isUuidValid(const XrUuidEXT& uuid);
    static std::string hexStr(const uint8_t *data, int len);
    static std::string uuidToString(XrUuidEXT uuid);
};

struct XrUuidExtCmp {
  bool operator()(const XrUuidEXT& a, const XrUuidEXT& b) const {
      return XrGodotUtils::uuidToString(a) < XrGodotUtils::uuidToString(b);
  }
};

} // namespace godot
