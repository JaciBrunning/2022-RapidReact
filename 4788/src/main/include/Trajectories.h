#include "Trajectory.h"

struct Trajectories {
  ::Trajectory test{{{0,4}, {1,4}, {3,4}, {4,4}}};

  void build() {
    test.build();
  }
};