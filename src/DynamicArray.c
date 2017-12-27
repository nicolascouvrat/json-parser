#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "./DynamicArray.h"

void display_error_and_exit(int errnum) {
  fprintf(stderr, "DYNAMIC_ARRAY FATAL ERROR: %s (%d)\n", strerror(errnum), errnum);
  exit(EXIT_FAILURE);
}

dynamic_array_t* dynamic_array_initialize() {
  dynamic_array_t *arr = malloc(sizeof(dynamic_array_t));
  if (arr == NULL)
    display_error_and_exit(errno);
  arr->items = malloc(sizeof(void*) * INITIAL_CAPACITY);
  if (arr->items == NULL)
    display_error_and_exit(errno);
  arr->capacity = INITIAL_CAPACITY;
  arr->size = 0;
  return arr;
}

void dynamic_array_destroy(dynamic_array_t *arr) {
  free(arr->items);
  free(arr);
}

int dynamic_array_push(dynamic_array_t *arr, void *val) {
  if (arr->capacity == arr->size) {
    // doubling
    arr->capacity = arr->capacity * GROWTH_FACTOR;
    void **extended = realloc(arr->items, sizeof(void*) * arr->capacity);
    if (extended == NULL) {
      //TODO: or just return -1?
      free(arr->items);
      display_error_and_exit(errno);
    }
    arr->items = extended;
  }
  arr->items[arr->size++] = val;
  return 0;
}

void* dynamic_array_pop(dynamic_array_t *arr) {
  int r;
  if (arr->size-- <= (arr->capacity / SHRINK_FACTOR)) {
    arr->capacity = arr->capacity / GROWTH_FACTOR;
    void **shrinked = realloc(arr->items, sizeof(void*) * arr->capacity);
    if (shrinked == NULL) {
      display_error_and_exit(errno);
    }
    arr->items = shrinked;
  }
  void *deleted = arr->items[arr->size];
  return deleted;
}

void* dynamic_array_get(dynamic_array_t *arr, int index) {
  if (index > arr->size)
    return NULL;
  return arr->items[index];
}
