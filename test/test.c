#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "./testutil.h"
#include "../src/index.c"

int json_atom_eq(json_atom_t *atom, char *start, char *end, json_atom_type_t type) {
  printf("start test: %d\n", atom->start == start);
  printf("atom start: %s, start: %s\n", atom->start, start);
  printf("end test: %d\n", atom->end == end);
  printf("atom end: %s, end: %s\n", atom->end, end);
  printf("type test: %d\n", atom->type == type);
  return (atom->start == start && atom->end == end && atom->type == type);
}

char* pointer_to_end_of_string(char* string) {
  int len = strlen(string);
  return string + len;
}

int test_create_atom() {
  char *primitive_test = "12345678";
  char *string_test = "\"value\"";
  char *invalid_string_test = "\"invalid";
  char *invalid_primitive_test = "12345\"";
  char *start;
  char *end;

  json_atom_t *first_atom = json_atom_initialize();
  json_atom_t *second_atom = json_atom_initialize();
  json_atom_t *third_atom = json_atom_initialize();
  json_atom_t *fourth_atom = json_atom_initialize();

  start = primitive_test;
  end = pointer_to_end_of_string(primitive_test);
  int first_test = json_atom_populate(first_atom, start, end);
  check(json_atom_eq(first_atom, start, end, ATOM_TYPE_PRIMITIVE));

  start = string_test;
  end = pointer_to_end_of_string(string_test);
  int second_test = json_atom_populate(second_atom, start, end);
  check(json_atom_eq(second_atom, start + 1, end - 1, ATOM_TYPE_STRING));

  start = invalid_string_test;
  end = pointer_to_end_of_string(invalid_string_test);
  int third_test = json_atom_populate(third_atom, start, end);
  check(third_test == ERR_INVAL);

  start = invalid_primitive_test;
  end = pointer_to_end_of_string(invalid_primitive_test);
  int fourth_test = json_atom_populate(fourth_atom, start, end);
  check(fourth_test == ERR_INVAL);

  return 0;
}

int main() {
  test(test_create_atom, "atom creation");
  return 0;
}
