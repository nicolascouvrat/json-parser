#ifndef __DYNAMIC_ARRAY_H__
#define __DYNAMIC_ARRAY_H__

#define INITIAL_CAPACITY 4
#define GROWTH_FACTOR 2
#define SHRINK_FACTOR 4

typedef struct dynamic_array {
  int capacity;
  int size;
  void **items;
} dynamic_array_t;

dynamic_array_t* dynamic_array_initialize();
void dynamic_array_destroy(dynamic_array_t *arr);
int dynamic_array_push(dynamic_array_t *arr, void *val);
void* dynamic_array_pop(dynamic_array_t *arr);
void* dynamic_array_get(dynamic_array_t *arr, int index);

#endif
