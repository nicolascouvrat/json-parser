#define INITIAL_CAPACITY 4
#define GROWTH_FACTOR 2
#define SHRINK_FACTOR 4

typedef struct dynamic_array {
  int capacity;
  int size;
  void **items;
} dynamic_array_t;
