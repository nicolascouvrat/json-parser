#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "../src/jspr.c"
#define TEST_STRING "{\"key1\":\"value1\",\"key2\":12345}"
#define LEN_TEST_STRING 36

int test_run(int number_of_runs) {
  int i;
  for (i = 0; i < number_of_runs; i++) {
    int len = jspr_size(TEST_STRING, LEN_TEST_STRING);
    if (len == -1)
      exit(EXIT_FAILURE);
    jspr_organism_t *organism = jspr_organism_initialize(len, TEST_STRING, LEN_TEST_STRING);
    if (jspr_organism_populate(organism) != 0)
      exit(EXIT_FAILURE);
    jspr_organism_destroy(organism);
  }
}

int main() {
  float startTime = (float)clock()/CLOCKS_PER_SEC;
  int n_runs = 10000000;
  test_run(n_runs);
  float endTime = (float)clock()/CLOCKS_PER_SEC;
  float timeElapsed = endTime - startTime;

  printf("Time taken for %d runs: %.6f\n", n_runs, timeElapsed);
}
