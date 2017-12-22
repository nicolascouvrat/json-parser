#ifndef __TESTUTIL_H__
#define __TESTUTIL_H__

static int test_passed = 0;
static int test_failed = 0;

#define fail() return __LINE__
#define done() return 0
#define check(cond) do { if (!(cond)) fail(); } while (0)

static void test(int (*func)(void), char *test_name) {
  int result = func();
  if (result == 0)
    test_passed++;
  else {
    test_failed++;
    printf("TEST FAILED: %s (check line %d)\n", test_name, result);
  }
}

#endif
