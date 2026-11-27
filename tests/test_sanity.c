#include "test.h"

static void test_arithmetic(void) {
  EXPECT_EQ(1 + 1, 2);
  EXPECT(sizeof(int) >= 2);
}

int main(void) {
  RUN_TEST(test_arithmetic);
  return TEST_REPORT();
}
