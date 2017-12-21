#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/index.c"

int test_find_first_char_between() {
  char needle = ':';
  char *first_test_string = "ne:edle";
  char *second_test_string = "needle";

  char *start = first_test_string;
  char *end = start + 6;
  printf("testing start: %s, end: %s\n", start, end);
  char *first_result = _find_first_char_between(needle, start, end);
  start = second_test_string;
  end = start + 5;
  printf("testing start: %s, end: %s\n", start, end);
  char *second_result = _find_first_char_between(needle, start, end);
  if (second_result == NULL)
    printf("not found in the second string\n");
  if (first_result != NULL)
    printf("found needle %c: %s\n", needle, first_result);
  return 0;
}

int _test_molecule_matches_string() {
  char *test_molecule_string = "\"key\":\"value\"";
  char *first_test_string = "key";

  char *start = test_molecule_string;
  char *end = start + strlen(test_molecule_string) - 1;
  json_molecule_t *molecule = create_molecule(start, end);

  printf("Matching: %d\n", json_molecule_matches_string(molecule, first_test_string));

}

int main() {
  test_find_first_char_between();
  _test_molecule_matches_string();
}
