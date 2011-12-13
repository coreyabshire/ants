#include "gtest/gtest.h"
#include "state.h"
#include "bot.h"

TEST(State, Sizes) {
  State state(200, 200);
  Loc a(0, 0);
  EXPECT_EQ(8, sizeof a);
  EXPECT_EQ(72, sizeof state.grid[0][0]);
  EXPECT_EQ(200, state.grid.size());
  EXPECT_EQ(40000, state.grid.size() * state.grid[0].size());
  EXPECT_EQ(2880000, state.rows * state.cols * sizeof state.grid[0][0]);
  EXPECT_EQ(992, sizeof state);
}

TEST(State, Distance) {
  State state(200, 200);
  Loc a(0, 0), b(1, 1), c(5, 5), d(10, 10), e(198, 198);
  Loc u(-1, -1), v(-5, -5);
  EXPECT_EQ(0, state.distance(a, a));
  EXPECT_EQ(2, state.distance(a, b));
  EXPECT_EQ(50, state.distance(a, c));
  EXPECT_EQ(50, state.distance(c, a));
  EXPECT_EQ(200, state.distance(a, d));
  EXPECT_EQ(8, state.distance(a, e));
  EXPECT_EQ(32, state.distance(b, c));
  EXPECT_EQ(18, state.distance(b, e));
  EXPECT_EQ(50, state.distance(c, d));
  EXPECT_EQ(0, state.distance(u, u));
  EXPECT_EQ(2, state.distance(a, u));
  EXPECT_EQ(2, state.distance(u, a));
  EXPECT_EQ(50, state.distance(a, v));
  EXPECT_EQ(32, state.distance(u, v));
}

TEST(State, Init) {
  State state(200, 200);
  Loc a(1, 1), b(3, 3), c(9, 8);
}

TEST(State, MoveAntTo) {
  State state(10, 10);
  Loc a(1, 1), b(1, 2);
  EXPECT_FALSE(state.grid[1][1].isUsed);
  EXPECT_FALSE(state.grid[1][2].isUsed);
  state.putAnt(1, 1, 0);
  EXPECT_FALSE(state.grid[1][1].isUsed);
  EXPECT_FALSE(state.grid[1][2].isUsed);
  EXPECT_EQ(0, state.grid[1][1].ant);
  EXPECT_EQ(-1, state.grid[1][2].ant);
  EXPECT_FALSE(state.grid[1][1].isUsed);
  EXPECT_FALSE(state.grid[1][2].isUsed);
  state.grid[a.r][a.c].moveAntTo(state.grid[b.r][b.c]);
  EXPECT_EQ(-1, state.grid[1][1].ant);
  EXPECT_EQ(0, state.grid[1][2].ant);
}

TEST(State, Update) {
  State state(10, 10);
  Loc a(1, 1), b(1, 2);
  state.putAnt(1, 1, 0);
  state.putFood(4, 4);
  state.update();
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
