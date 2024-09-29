#include <gtest/gtest.h>

#include <vector>

#define MONO 0
#define LEFT 1
#define RIGHT 2

struct QuadSurfaceLayer {
  bool sideBySide3D; // If true, this is a Stereo 3D Quad Layer (SBS)
};

int _get_composition_layer_count(std::vector<QuadSurfaceLayer> layers) {
  int layerCount = 0;
  for (int i = 0; i < layers.size(); i++) {
    layerCount += 1;
    if (layers[i].sideBySide3D) {
      layerCount += 1;
    }
  }
  return layerCount;
};

int _get_composition_layer(int p_index, std::vector<QuadSurfaceLayer> layers) {
  // First, calculate the layerIndex and which eye
  int layerIndex = 0;
  int p_index_countdown = p_index;
  for (; layerIndex < layers.size() && p_index_countdown > 0; layerIndex++) {
    if (layers[layerIndex].sideBySide3D) {
      p_index_countdown--;
    }
    if (p_index_countdown == 0) {
      p_index_countdown = -1;
      break;
    }
    p_index_countdown--;
  }

  // Then grab the layer and quadLayer to return
  // auto layer = layers[layerIndex];
  // int quadLayer;
  // if (layer.sideBySide3D) {
  //   quadLayer = p_index_countdown == -1 ? LEFT : RIGHT;
  // } else {
  //   quadLayer = MONO;
  // }

  return layerIndex;
};

// Demonstrate some basic assertions.
TEST(HelloTest, BasicAssertions) {
  std::vector<QuadSurfaceLayer> layers = {
    {false},
    {true},
    {false},
    {false},
  };

  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world");
  // Expect equality.
  EXPECT_EQ(7 * 6, 42);

  EXPECT_EQ(_get_composition_layer_count(layers), 5);
  EXPECT_EQ(_get_composition_layer(0, layers), 0);
  EXPECT_EQ(_get_composition_layer(1, layers), 1);
  EXPECT_EQ(_get_composition_layer(2, layers), 1);
  EXPECT_EQ(_get_composition_layer(3, layers), 2);
}
