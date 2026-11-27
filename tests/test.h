#ifndef SOKOBANI_TEST_H
#define SOKOBANI_TEST_H

#include <stdio.h>
#include <stdlib.h>

static int test_failures = 0;
static int test_count = 0;

#define EXPECT(cond)                                                           \
  do {                                                                         \
    test_count++;                                                              \
    if (!(cond)) {                                                             \
      test_failures++;                                                         \
      fprintf(stderr, "%s:%d: expectation failed: %s\n", __FILE__, __LINE__,   \
              #cond);                                                          \
    }                                                                          \
  } while (0)

#define EXPECT_EQ(a, b) EXPECT((a) == (b))

#define RUN_TEST(fn)                                                           \
  do {                                                                         \
    printf("  %s\n", #fn);                                                     \
    fn();                                                                      \
  } while (0)

#define TEST_REPORT()                                                          \
  (printf("%d checks, %d failed\n", test_count, test_failures),                \
   test_failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE)

#endif
