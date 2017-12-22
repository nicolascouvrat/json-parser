#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define __DEBUG__

#include "./testutil.h"
#include "../src/index.c"

char* pointer_to_end_of_string(char* string) {
  int len = strlen(string);
  return string + len;
}

int json_atom_populate_wrap(json_atom_t *atom, char* string) {
  char* start = string;
  return json_atom_populate(atom, start, pointer_to_end_of_string(start));
}

int json_molecule_populate_wrap(json_molecule_t *molecule, char* string) {
  char *start = string;
  return json_molecule_populate(molecule, start, pointer_to_end_of_string(start));
}

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

int test_life_cycle() {
  json_organism_t *organism = json_organism_initialize(0, "");
  json_organism_destroy(organism);

  json_molecule_t *molecule = json_molecule_initialize();
  json_molecule_destroy(molecule);

  json_atom_t *atom = json_atom_initialize();
  json_atom_destroy(atom);

  return 0;
}

int test_atom_populate() {
  char *primitive_test = "12345678";
  char *string_test = "\"value\"";
  char *invalid_string_test = "\"invalid";
  char *invalid_primitive_test = "12345\"";

  json_atom_t *atom = json_atom_initialize();

  int first_test = json_atom_populate_wrap(atom, primitive_test);
  check(json_atom_eq(atom, primitive_test, ATOM_TYPE_PRIMITIVE));

  int second_test = json_atom_populate_wrap(atom, string_test);
  check(json_atom_eq(atom, string_test, ATOM_TYPE_STRING));

  int third_test = json_atom_populate_wrap(atom, invalid_string_test);
  check(third_test == ERR_INVAL);

  int fourth_test = json_atom_populate_wrap(atom, invalid_primitive_test);
  check(fourth_test == ERR_INVAL);

  json_atom_destroy(atom);

  return 0;
}

int test_molecule_populate() {
  char *string_value_test = "\"key\":\"value\"";
  char *primitive_value_test = "\"key\":12345";
  char *invalid_json_test = "key:12345";
  char *invalid_atom_test = "\"key\":\"invalid";

  json_molecule_t *first_molecule = json_molecule_initialize();
  json_molecule_t *second_molecule = json_molecule_initialize();
  json_molecule_t *third_molecule = json_molecule_initialize();
  json_molecule_t *fourth_molecule = json_molecule_initialize();

  int first_test = json_molecule_populate_wrap(first_molecule, string_value_test);
  check(
    json_atom_eq_p(
      first_molecule->key,
      string_value_test,
      string_value_test + 5,
      ATOM_TYPE_STRING
    ) && json_atom_eq_p(
      first_molecule->value,
      string_value_test + 6,
      pointer_to_end_of_string(string_value_test),
      ATOM_TYPE_STRING
    )
  );
  int second_test = json_molecule_populate_wrap(second_molecule, primitive_value_test);
  check(
    json_atom_eq_p(
      second_molecule->key,
      primitive_value_test,
      primitive_value_test + 5,
      ATOM_TYPE_STRING
    ) && json_atom_eq_p(
      second_molecule->value,
      primitive_value_test + 6,
      pointer_to_end_of_string(primitive_value_test),
      ATOM_TYPE_PRIMITIVE
    )
  );

  int third_test = json_molecule_populate_wrap(third_molecule, invalid_json_test);
  check(third_test == ERR_STRICT_JSON);

  int fourth_test = json_molecule_populate_wrap(fourth_molecule, invalid_atom_test);
  check(fourth_test == ERR_INVAL);

  json_molecule_destroy(first_molecule);
  json_molecule_destroy(second_molecule);
  json_molecule_destroy(third_molecule);
  json_molecule_destroy(fourth_molecule);

  return 0;
}

int test_organism_populate() {
  char *ref_string_test = "{\"key1\":\"value1\",\"key2\":1234}";
  char *ref_string_invalid_test = "{\"key1\":\"value1,\"key2:1234}";

  json_organism_t *first_organism = json_organism_initialize(2, ref_string_test);
  json_organism_t *second_organism = json_organism_initialize(2, ref_string_invalid_test);

  int first_test = json_organism_populate(first_organism);
  check(
    json_atom_eq_p(
      first_organism->molecules[0]->key,
      ref_string_test + 1,
      ref_string_test + 7,
      ATOM_TYPE_STRING
    ) && json_atom_eq_p(
      first_organism->molecules[0]->value,
      ref_string_test + 8,
      ref_string_test + 16,
      ATOM_TYPE_STRING
    ) && json_atom_eq_p(
      first_organism->molecules[1]->key,
      ref_string_test + 17,
      ref_string_test + 23,
      ATOM_TYPE_STRING
    ) && json_atom_eq_p(
      first_organism->molecules[1]->value,
      ref_string_test + 24,
      pointer_to_end_of_string(ref_string_test) - 1,
      ATOM_TYPE_PRIMITIVE
    )
  );

  int second_test = json_organism_populate(second_organism);
  check(second_test == ERR_INVAL);

  json_organism_destroy(first_organism);
  json_organism_destroy(second_organism);

  return 0;
}

int test_molecule_matches_string() {
  char *molecule_string = "\"match\":12345";
  char *match_test = "match";
  char *first_fail_test = "matchs";
  char *second_fail_test = "matc";
  json_molecule_t *molecule = json_molecule_initialize();

  json_molecule_populate_wrap(molecule, molecule_string);

  check(json_molecule_matches_string(molecule, match_test));
  check(!json_molecule_matches_string(molecule, first_fail_test));
  check(!json_molecule_matches_string(molecule, second_fail_test));

  json_molecule_destroy(molecule);

  return 0;
}

int test_organism_contains_key() {
  char *organism_string = "{\"key1\":12345,\"key2\":\"value\",\"key3\":\"other value\"}";
  char *match_test = "key1";
  char *fail_test = "key11";
  json_organism_t *organism = json_organism_initialize(3, organism_string);
  json_organism_populate(organism);

  check(json_organism_contains_key(organism, match_test));
  check(!json_organism_contains_key(organism, fail_test));

  json_organism_destroy(organism);

  return 0;
}

int test_organism_find() {
  char *organism_string = "{\"key1\":12345,\"key2\":\"value\",\"key3\":\"other value\"}";
  char *match_test = "key1";
  char *fail_test = "key11";
  json_organism_t *organism = json_organism_initialize(3, organism_string);
  json_atom_t *first_atom = json_atom_initialize();
  json_atom_t *second_atom = json_atom_initialize();
  json_organism_populate(organism);

  int first_test = json_organism_find(first_atom, organism, match_test);
  check(
    json_atom_eq_p(
      first_atom,
      organism_string + 8,
      organism_string + 13,
      ATOM_TYPE_PRIMITIVE
    )
  );
  int second_test = json_organism_find(second_atom, organism, fail_test);
  check(!second_test);

  json_organism_destroy(organism);
  json_atom_destroy(first_atom);
  json_atom_destroy(second_atom);

  return 0;
}

int main() {
  printf("##############################\n"
         "##    Test session start    ##\n"
         "##############################\n\n"
       );

  test(test_life_cycle, "life cycle of structures");
  test(test_atom_populate, "atom populate");
  test(test_molecule_populate, "molecule populate");
  test(test_organism_populate, "organism populate");
  test(test_molecule_matches_string, "molecule matches string");
  test(test_organism_contains_key, "organism contains key");
  test(test_organism_find, "organism find value of key");


  printf("\n##############################\n"
         "##    Test session ended    ##\n"
         "##                          ##\n"
         "## failed: %d  |  success: %d ##\n"
         "##############################\n",
         test_failed, test_passed);
  return 0;
}
