#include "gtest/gtest.h"
#include "state.h"
#include "bot.h"

TEST(State, Sizes) {
  State state(200, 200);
  Location a(0, 0);
  EXPECT_EQ(8, sizeof a);
  EXPECT_EQ(120, sizeof state.grid[0][0]);
  EXPECT_EQ(200, state.grid.size());
  EXPECT_EQ(40000, state.grid.size() * state.grid[0].size());
  EXPECT_EQ(4800000, state.rows * state.cols * sizeof state.grid[0][0]);
  EXPECT_EQ(1032, sizeof state);
}

TEST(State, Distance) {
  State state(200, 200);
  Location a(0, 0), b(1, 1), c(5, 5), d(10, 10), e(198, 198);
  Location u(-1, -1), v(-5, -5);
  EXPECT_FLOAT_EQ(0, state.distance(a, a));
  EXPECT_FLOAT_EQ(1.4142135, state.distance(a, b));
  EXPECT_FLOAT_EQ(7.0710678, state.distance(a, c));
  EXPECT_FLOAT_EQ(7.0710678, state.distance(c, a));
  EXPECT_FLOAT_EQ(14.142136, state.distance(a, d));
  EXPECT_FLOAT_EQ(2.8284271, state.distance(a, e));
  EXPECT_FLOAT_EQ(5.6568542, state.distance(b, c));
  EXPECT_FLOAT_EQ(4.2426405, state.distance(b, e));
  EXPECT_FLOAT_EQ(7.0710678, state.distance(c, d));
  EXPECT_FLOAT_EQ(0, state.distance(u, u));
  EXPECT_FLOAT_EQ(1.4142135, state.distance(a, u));
  EXPECT_FLOAT_EQ(1.4142135, state.distance(u, a));
  EXPECT_FLOAT_EQ(7.0710678, state.distance(a, v));
  EXPECT_FLOAT_EQ(5.6568542, state.distance(u, v));
}

TEST(State, Distance2) {
  State state(200, 200);
  Location a(0, 0), b(1, 1), c(5, 5), d(10, 10), e(198, 198);
  Location u(-1, -1), v(-5, -5);
  EXPECT_EQ(0, state.distance2(a, a));
  EXPECT_EQ(2, state.distance2(a, b));
  EXPECT_EQ(50, state.distance2(a, c));
  EXPECT_EQ(50, state.distance2(c, a));
  EXPECT_EQ(200, state.distance2(a, d));
  EXPECT_EQ(8, state.distance2(a, e));
  EXPECT_EQ(32, state.distance2(b, c));
  EXPECT_EQ(18, state.distance2(b, e));
  EXPECT_EQ(50, state.distance2(c, d));
  EXPECT_EQ(0, state.distance2(u, u));
  EXPECT_EQ(2, state.distance2(a, u));
  EXPECT_EQ(2, state.distance2(u, a));
  EXPECT_EQ(50, state.distance2(a, v));
  EXPECT_EQ(32, state.distance2(u, v));
}

TEST(State, Init) {
  State state(200, 200);
  Location a(1, 1), b(3, 3), c(9, 8);
}

TEST(State, Manhattan) {
  State state(10, 10);
  Location a(1, 1), b(3, 3), c(9, 8);
  EXPECT_EQ(0, state.manhattan(a, a));
  EXPECT_EQ(4, state.manhattan(a, b));
  EXPECT_EQ(4, state.manhattan(b, a));
  EXPECT_EQ(5, state.manhattan(a, c));
  EXPECT_EQ(5, state.manhattan(c, a));
}

TEST(State, TryMoves) {
  State state(10, 10);
  Location a(1, 1), b(1, 2);
  vector<int> moves;
  vector<int> ants;
  EXPECT_EQ(-1, state.grid[1][1].ant);
  EXPECT_EQ(-1, state.grid[1][2].ant);
  EXPECT_EQ(-1, state.grid[2][1].ant);
  EXPECT_EQ(-1, state.grid[0][2].ant);
  state.putAnt(1, 1, 0);
  state.putAnt(1, 2, 0);
  EXPECT_EQ(0, state.grid[1][1].ant);
  EXPECT_EQ(0, state.grid[1][2].ant);
  EXPECT_EQ(-1, state.grid[2][1].ant);
  EXPECT_EQ(-1, state.grid[0][2].ant);
  ants.push_back(0); moves.push_back(SOUTH);
  ants.push_back(1); moves.push_back(NORTH);
  EXPECT_TRUE(state.tryMoves(ants, moves));
  state.printAnts(ants);
  EXPECT_EQ(-1, state.grid[1][1].ant);
  EXPECT_EQ(-1, state.grid[1][2].ant);
  EXPECT_EQ(0, state.grid[2][1].ant);
  EXPECT_EQ(0, state.grid[0][2].ant);
  state.undoMoves(ants);
  EXPECT_EQ(0, state.grid[1][1].ant);
  EXPECT_EQ(0, state.grid[1][2].ant);
  EXPECT_EQ(-1, state.grid[2][1].ant);
  EXPECT_EQ(-1, state.grid[0][2].ant);
}

TEST(State, MoveAntTo) {
  State state(10, 10);
  Location a(1, 1), b(1, 2);
  EXPECT_FALSE(state.grid[1][1].isUsed);
  EXPECT_FALSE(state.grid[1][2].isUsed);
  state.putAnt(1, 1, 0);
  EXPECT_FALSE(state.grid[1][1].isUsed);
  EXPECT_FALSE(state.grid[1][2].isUsed);
  EXPECT_EQ(0, state.grid[1][1].ant);
  EXPECT_EQ(-1, state.grid[1][2].ant);
  EXPECT_FALSE(state.grid[1][1].isUsed);
  EXPECT_FALSE(state.grid[1][2].isUsed);
  state.grid[a.row][a.col].moveAntTo(state.grid[b.row][b.col]);
  EXPECT_EQ(-1, state.grid[1][1].ant);
  EXPECT_EQ(0, state.grid[1][2].ant);
}

TEST(State, Update) {
  State state(10, 10);
  Location a(1, 1), b(1, 2);
  state.putAnt(1, 1, 0);
  state.putFood(4, 4);
  state.update();
}

// TEST(State, DistanceSpeed) {
//   State state(200, 200);
//   vector<Location> a;
//   for (int i = 0; i < 1000; i++)
//     a.push_back(state.randomLocation());
//   Timer timer;
//   timer.start();
//   for (int i = 0; i < 1000000; i++) {
//     double distance = state.distance(a[rand()%1000], a[rand()%1000]);
//   }
//   EXPECT_PRED_FORMAT2(::testing::FloatLE, 0.001, timer.getTime());
// }

// TEST(State, Distance2Speed) {
//   State state(200, 200);
//   vector<Location> a;
//   for (int i = 0; i < 1000; i++)
//     a.push_back(state.randomLocation());
//   Timer timer;
//   timer.start();
//   for (int i = 0; i < 1000000; i++) {
//     int distance = state.distance2(a[rand()%1000], a[rand()%1000]);
//   }
//   EXPECT_PRED_FORMAT2(::testing::FloatLE, 0.001, timer.getTime());
// }

// TEST(State, ManhattanSpeed) {
//   State state(200, 200);
//   vector<Location> a;
//   for (int i = 0; i < 1000; i++)
//     a.push_back(state.randomLocation());
//   Timer timer;
//   timer.start();
//   for (int i = 0; i < 1000000; i++) {
//     int distance = state.manhattan(a[rand()%1000], a[rand()%1000]);
//   }
//   EXPECT_PRED_FORMAT2(::testing::FloatLE, 0.001, timer.getTime());
// }

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
