#include "xr_scene_provider_fake.h"

using namespace godot;

namespace {
  // struct XrSceneObjectInternal {
  //   XrUuidEXT uuid;
  //   XrSpace space;
  // };

  static const std::vector<XrSceneObjectInternal> objects = {
    XrSceneObjectInternal { // Fake 0
      {{0xFF,0xEE,0xDD,0xCC,0xBB,0xAA,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0xFF,0xEE,0xDD,0x00}}, // UUID
      (XrSpace) 0, // XrSpace (Index into poses)
      "CEILING", // label
      std::vector<XrVector2f>{{-1.0f, -1.0f}, {1.0f, -1.0f}, {1.0f, 1.0f}, {-1.0f, 1.0f}},
      XrRect2Df{{-1.0f, -1.0f}, {2.0f, 2.0f}},
      std::nullopt,
    },
    XrSceneObjectInternal { // Fake 1
      {{0xFF,0xEE,0xDD,0xCC,0xBB,0xAA,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0xFF,0xEE,0xDD,0x01}}, // UUID
      (XrSpace) 1, // XrSpace (Index into poses)
      "FLOOR", // label
      std::vector<XrVector2f>{{-1.0f, -1.0f}, {1.0f, -1.0f}, {1.0f, 1.0f}, {-1.0f, 1.0f}},
      XrRect2Df{{-1.0f, -1.0f}, {2.0f, 2.0f}},
      std::nullopt,
    },
    XrSceneObjectInternal { // Fake 2
      {{0xFF,0xEE,0xDD,0xCC,0xBB,0xAA,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0xFF,0xEE,0xDD,0x02}}, // UUID
      (XrSpace) 2, // XrSpace (Index into poses)
      "TABLE", // label
      std::nullopt,
      XrRect2Df{{-0.5f, -0.5f}, {1.0f, 1.0f}},
      std::nullopt,
    },
    XrSceneObjectInternal { // Fake 3
      {{0xFF,0xEE,0xDD,0xCC,0xBB,0xAA,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0xFF,0xEE,0xDD,0x03}}, // UUID
      (XrSpace) 3, // XrSpace (Index into poses)
      "WALL_FACE", // label
      std::nullopt,
      XrRect2Df{{-1.0f, -1.0f}, {2.0f, 2.0f}},
      std::nullopt,
    },
    XrSceneObjectInternal { // Fake 4
      {{0xFF,0xEE,0xDD,0xCC,0xBB,0xAA,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0xFF,0xEE,0xDD,0x04}}, // UUID
      (XrSpace) 4, // XrSpace (Index into poses)
      "WALL_FACE", // label
      std::nullopt,
      XrRect2Df{{-1.0f, -1.0f}, {2.0f, 2.0f}},
      std::nullopt,
    },
    XrSceneObjectInternal { // Fake 5
      {{0xFF,0xEE,0xDD,0xCC,0xBB,0xAA,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0xFF,0xEE,0xDD,0x05}}, // UUID
      (XrSpace) 5, // XrSpace (Index into poses)
      "WALL_FACE", // label
      std::nullopt,
      XrRect2Df{{-1.0f, -1.0f}, {2.0f, 2.0f}},
      std::nullopt,
    },
    XrSceneObjectInternal { // Fake 6
      {{0xFF,0xEE,0xDD,0xCC,0xBB,0xAA,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0xFF,0xEE,0xDD,0x06}}, // UUID
      (XrSpace) 6, // XrSpace (Index into poses)
      "WALL_FACE", // label
      std::nullopt,
      XrRect2Df{{-1.0f, -1.0f}, {2.0f, 2.0f}},
      std::nullopt,
    },
  };

  static const std::vector<XrPosef> poses = {
    { // Fake 0 (Ceiling)
      {0.7068252f, 0, 0, 0.7073883f},
      {0.0, 3.0, -2.0},
    },
    { // Fake 1 (Floor)
      {-0.7068252f, 0, 0, 0.7073883f},
      {0.0, 1.0, -2.0},
    },
    { // Fake 2 (Table)
      {-0.7068252f, 0, 0, 0.7073883f},
      {0.0, 1.5, -2.0},
    },
    { // Fake 3 (Wall)
      {0, 1, 0, 0.0002963},
      {0.0, 2.0, -1.0},
    },
    { // Fake 4 (Wall)
      {0, 0, 0, 1},
      {0.0, 2.0, -3.0},
    },
    { // Fake 5 (Wall)
      {0, -0.7068252, 0, 0.7073883},
      {1.0, 2.0, -2.0},
    },
    { // Fake 6 (Wall)
      {0, 0.7068252, 0, 0.7073883},
      {-1.0, 2.0, -2.0},
    },
  };
} // anonymous namespace

void XrSceneProviderFake::query_room(QueryRoomCallback_t callback) {
  // Immediately run the callback. It runs the callable async using Godot's thread pool
  callback(objects);
}

void XrSceneProviderFake::locate_space(XrSpace space, XrSpaceLocation* location) {
  location->locationFlags = XR_SPACE_LOCATION_POSITION_VALID_BIT;
  location->pose = poses[(int64_t) space];
}
