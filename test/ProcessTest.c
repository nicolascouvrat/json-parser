#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/index.c"

int _test_create_atom() {
  char *test_key = "\"longkey\":";
  char *test_value = "567";
  char *start = test_key;
  char *end = start + 8;
  printf("testing atom creation for start: %s, end %s\n", start, end);
  json_atom_t *key_atom = create_atom(start, end);
  if (key_atom == NULL)
    return EXIT_FAILURE;

  start = test_value;
  end = start + 2;
  json_atom_t *value_atom = create_atom(start, end);

  printf("KEY ATOM:\n");
  _json_atom_print(key_atom);
  printf("VALUE ATOM:\n");
  _json_atom_print(value_atom);
  printf(
    "key is: %s, value is: %s\n",
    get_value_from_atom(key_atom),
    get_value_from_atom(value_atom)
  );
  json_atom_destroy(value_atom);
  json_atom_destroy(key_atom);
  return 0;
}

int _test_create_molecule() {
  char *test_molecule = "\"key\":\"string value\"";
  char *start = test_molecule;
  char *end = start + strlen(test_molecule) - 1;
  printf("testing molecule creation for start: %s, end %s\n", start, end);

  json_molecule_t *molecule = create_molecule(start, end);
  printf(
    "key is: %s, value is %s\n",
    get_value_from_atom(molecule->key),
    get_value_from_atom(molecule->value)
  );
  json_molecule_destroy(molecule);
  return 0;
}

int _test_populate_organism() {
  char *test_string = "{\"key1\":\"value1\",\"key2\":5,\"key3\":\"value3\"}";
  int test_string_len = strlen(test_string);
  json_organism_t *organism = json_organism_initialize(3, test_string, test_string_len);
  json_organism_populate(organism);

  _json_organism_print(organism);

  json_organism_destroy(organism);
}

int _test_organism_contains_key() {
  char *test_string = "{\"key1\":\"value1\",\"key2\":5,\"key3\":\"value3\"}";
  int test_string_len = strlen(test_string);
  char *first_key = "key1";
  char *second_key = "key5";
  json_organism_t *organism = json_organism_initialize(3, test_string, test_string_len);
  json_organism_populate(organism);

  printf(
    "organism contains '%s'? %d\n"
    "organism contains '%s'? %d\n",
    first_key, json_organism_contains_key(organism, first_key),
    second_key, json_organism_contains_key(organism, second_key)
  );

  json_organism_destroy(organism);
}

int run_tests() {
  printf("-- create atom --\n");
  _test_create_atom();
  printf("-- create molecule --\n");
  _test_create_molecule();
  printf("-- create organism --\n");
  _test_populate_organism();
  printf("-- organism contains key --\n");
  _test_organism_contains_key();
  return 0;
}

int main() {
  run_tests();
  return 0;
}
