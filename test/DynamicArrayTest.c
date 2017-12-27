#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "./testutil.h"
#include "../src/DynamicArray.c"

int lifecycle() {
  dynamic_array_t* arr = dynamic_array_initialize();
  dynamic_array_destroy(arr);

  done();
}

int push() {
  dynamic_array_t *arr = dynamic_array_initialize();

  char *p_char = "string";
  dynamic_array_push(arr, p_char);
  int i;
  for (i = 0; i < 10; i++) {
    int *p_int = malloc(sizeof(int));
    *p_int = i;
    dynamic_array_push(arr, p_int);
  }

  check(arr->size == 11);
  check(arr->capacity == 16);
  check((char*)dynamic_array_get(arr, 0) == p_char);
  check(*(int*)dynamic_array_get(arr, 3) == 2);

  for (i = 1; i < arr->size; i++) {
    free(arr->items[i]);
  }

  dynamic_array_destroy(arr);
  done();
}

int pop() {
  dynamic_array_t *arr = dynamic_array_initialize();

  int i;
  for (i = 0; i < 5; i++) {
    int *p_int = malloc(sizeof(int));
    *p_int = i;
    dynamic_array_push(arr, p_int);
  }
  int *removed;
  removed = (int*)dynamic_array_pop(arr);
  check(arr->size == 4);
  check(arr->capacity == 8);
  check(*removed == 4);
  free(removed);

  for (i = 0; i < 3; i++) {
    removed = (int*)dynamic_array_pop(arr);
    free(removed);
  }
  check(arr->size == 1);
  check(arr->capacity == 4);

  free(arr->items[0]);
  dynamic_array_destroy(arr);
  done();
}

int main() {
  test_session_start();

  test(lifecycle, "Dynamic array lifecycle");
  test(push, "Push function");
  test(pop, "Pop function");

  test_session_end();
}
