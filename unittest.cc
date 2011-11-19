#include "gtest/gtest.h"
#include "state.h"
#include "bot.h"

TEST(State, Manhattan) {
  State state(10, 10);
  Location a(1, 1), b(3, 3), c(9, 8);
  EXPECT_EQ(0, state.manhattan(a, a));
  EXPECT_EQ(4, state.manhattan(a, b));
  EXPECT_EQ(4, state.manhattan(b, a));
  EXPECT_EQ(5, state.manhattan(a, c));
  EXPECT_EQ(5, state.manhattan(c, a));
}

TEST(Bot, DoMoveDirection) {
  Bot bot(10,10);
  Location a(5,5), n(4,5), s(6,5), e(5,4), w(5,6);
  bot.state[a].ant = 0;
  bot.state.myAnts.push_back(a);
  EXPECT_TRUE(bot.doMoveDirection(a, NORTH));
  EXPECT_EQ(-1, bot.state[a].ant);
  EXPECT_EQ(0, bot.state[n].ant);
  EXPECT_EQ(1, bot.orders.count(n));
}

TEST(Route, Assignment) {
  Location a(1,1), b(2,2), o(0,0);
  map<Location,Location> p;
  p[b] = a;
  Route r1(a, b, p), r2, r3(r1);
  EXPECT_EQ(a, r1.start);
  EXPECT_EQ(b, r1.end);
  EXPECT_EQ(2, r1.distance);
  EXPECT_EQ(o, r2.start);
  EXPECT_EQ(o, r2.end);
  EXPECT_EQ(0, r2.distance);
  EXPECT_EQ(a, r3.start);
  EXPECT_EQ(b, r3.end);
  EXPECT_EQ(2, r3.distance);
  r2 = r1;
  EXPECT_EQ(a, r2.start);
  EXPECT_EQ(b, r2.end);
  EXPECT_EQ(2, r2.distance);
}

TEST(Bot, Search4) {
  Bot bot(10, 10);
  Location start(1,1), goal(5,5);
  Route route;
  EXPECT_TRUE(bot.search(start, goal, route));
  EXPECT_EQ(start, route.start);
  EXPECT_EQ(goal, route.end);
  EXPECT_EQ(9, route.distance);
}

TEST(State, Sizes) {
  State state(200,200);
  Location a(0,0);
  EXPECT_EQ(4, sizeof a);
  EXPECT_EQ(48, sizeof state.grid[0][0]);
  EXPECT_EQ(200, state.grid.size());
  EXPECT_EQ(40000, state.grid.size() * state.grid[0].size());
  EXPECT_EQ(1920000, state.rows * state.cols * sizeof state.grid[0][0]);
  EXPECT_EQ(800, sizeof state);
}

TEST(State, Distance) {
  State state(200,200);
  Location a(0,0), b(1,1), c(5,5), d(10,10), e(198,198);
  EXPECT_FLOAT_EQ(0, state.distance(a, a));
  EXPECT_FLOAT_EQ(1.4142135, state.distance(a, b));
  EXPECT_FLOAT_EQ(7.0710678, state.distance(a, c));
  EXPECT_FLOAT_EQ(7.0710678, state.distance(c, a));
  EXPECT_FLOAT_EQ(14.142136, state.distance(a, d));
  EXPECT_FLOAT_EQ(2.8284271, state.distance(a, e));
  EXPECT_FLOAT_EQ(5.6568542, state.distance(b, c));
  EXPECT_FLOAT_EQ(4.2426405, state.distance(b, e));
  EXPECT_FLOAT_EQ(7.0710678, state.distance(c, d));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
