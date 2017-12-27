#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "./testutil.h"
#include "../src/index.c"

char* pointer_to_end_of_string(char* string) {
  int len = strlen(string);
  return string + len;
}

// int json_atom_populate_wrap(json_atom_t *atom, char* string) {
//   char* start = string;
//   return json_atom_populate(atom, start, pointer_to_end_of_string(start));
// }
//
// int json_molecule_populate_wrap(json_molecule_t *molecule, char* string) {
//   char *start = string;
//   return json_molecule_populate(molecule, start, pointer_to_end_of_string(start));
// }

int json_atom_eq_p(json_atom_t *atom, char *start, char *end, json_atom_type_t type) {
  char *_start = start, *_end = end;
  if (type == ATOM_TYPE_STRING) {
    _start += 1;
    _end -= 1;
  }
  return (atom->start == _start && atom->end == _end && atom->type == type);
}

int json_atom_eq(json_atom_t *atom, char *snippet, json_atom_type_t type) {
  char *start = snippet;
  char *end = pointer_to_end_of_string(snippet);
  if (type == ATOM_TYPE_STRING) {
    start = start + 1;
    end = end - 1;
  }
  return (atom->start == start && atom->end == end && atom->type == type);
}


/**
 * Check that you can call a destroyer immediately after an initializer
 * (making setters optional)
 */

int t_life_cycle() {
  json_organism_t *organism = json_organism_initialize("");
  json_molecule_t *molecule = json_molecule_initialize();
  json_atom_t *key = json_atom_initialize();
  json_atom_t *value = json_atom_initialize();

  json_molecule_set(molecule, key, value);
  json_organism_add_molecule(organism, molecule);

  json_organism_destroy(organism);
  done();
}

int t_backtrack() {
  char *first_test = "endofkey\":";
  char *second_test = "endofvalue:";
  char *third_test = "threespaces   :";

  json_atom_t *atom = json_atom_initialize();
  
  backtrack(atom, pointer_to_end_of_string(first_test) - 1);
  check(atom->end == pointer_to_end_of_string(first_test) - 2);
  check(atom->type == ATOM_TYPE_STRING);

  backtrack(atom, pointer_to_end_of_string(second_test) - 1);
  check(atom->end == pointer_to_end_of_string(second_test) - 1);
  check(atom->type == ATOM_TYPE_PRIMITIVE);

  backtrack(atom, pointer_to_end_of_string(third_test) - 1);
  check(atom->end == pointer_to_end_of_string(third_test) - 4);
  check(atom->type == ATOM_TYPE_PRIMITIVE);

  json_atom_destroy(atom);

  done();
}

int main() {
  test_session_start();

  test(t_life_cycle, "life cycle of structures");
  test(
    t_backtrack,
    "backtrack function that, given the position of a closing token "
    "will return end of previous atom"
  );

  test_session_end();
  done();
}
