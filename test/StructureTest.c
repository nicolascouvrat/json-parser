#include <stdio.h>
#include <stdlib.h>
#include "../src/index.c"

int _atom_test() {
  json_atom_t *key_atom = json_atom_initialize();
  json_atom_t *value_atom = json_atom_initialize();
  char *test_key = "\"key\"";
  char *test_value = "\"value\"";
  char *start = test_key + 1;
  char *end = start + 3;
  json_atom_set(key_atom, start, end, ATOM_TYPE_STRING);
  start = test_value + 1;
  end = start + 5;
  json_atom_set(value_atom, start, end, ATOM_TYPE_STRING);

  _json_atom_print(value_atom);
  _json_atom_print(key_atom);

  json_atom_destroy(value_atom);
  json_atom_destroy(key_atom);

  return 0;
}

int _molecule_test() {
  json_molecule_t *molecule = json_molecule_initialize();
  json_atom_t *key_atom = json_atom_initialize();
  json_atom_t *value_atom = json_atom_initialize();
  char *test_key = "\"key\"";
  char *test_value = "\"value\"";
  char *start = test_key + 1;
  char *end = start + 3;
  json_atom_set(key_atom, start, end, ATOM_TYPE_STRING);
  start = test_value + 1;
  end = start + 5;
  json_atom_set(value_atom, start, end, ATOM_TYPE_STRING);
  json_molecule_set(molecule, key_atom, value_atom);
  _json_molecule_print(molecule);
  json_molecule_destroy(molecule);

  return 0;
}

int _organism_test() {
  char *test_ref_string = "\"key1\":\"value1\",\"key2\":\"value2\"";
  json_organism_t *organism = json_organism_initialize(2, test_ref_string);
  // definitions
  json_molecule_t *first_molecule = json_molecule_initialize();
  json_atom_t *first_key_atom = json_atom_initialize();
  json_atom_t *first_value_atom = json_atom_initialize();
  char *first_test_key = "\"key1\"";
  char *first_test_value = "\"value1\"";

  json_molecule_t *second_molecule = json_molecule_initialize();
  json_atom_t *second_key_atom = json_atom_initialize();
  json_atom_t *second_value_atom = json_atom_initialize();
  char *second_test_key = "\"key2\"";
  char *second_test_value = "\"value2\"";
  // first molecule
  char *start = first_test_key + 1;
  char *end = start + 4;
  json_atom_set(first_key_atom, start, end, ATOM_TYPE_STRING);
  start = first_test_value + 1;
  end = start + 6;
  json_atom_set(first_value_atom, start, end, ATOM_TYPE_STRING);
  json_molecule_set(first_molecule, first_key_atom, first_value_atom);

  start = second_test_key + 1;
  end = start + 4;
  json_atom_set(second_key_atom, start, end, ATOM_TYPE_STRING);
  start = second_test_value + 1;
  end = start + 6;
  json_atom_set(second_value_atom, start, end, ATOM_TYPE_STRING);
  json_molecule_set(second_molecule, second_key_atom, second_value_atom);

  json_organism_add_molecule(organism, first_molecule);
  json_organism_add_molecule(organism, second_molecule);

  _json_organism_print(organism);

  json_organism_destroy(organism);
  return 0;
}

int _run_tests() {
  printf("-- atom test --\n");
  _atom_test();
  printf("-- molecule test --\n");
  _molecule_test();
  printf("-- organism test --\n");
  _organism_test();

  return 0;
}

int main() {
  return _run_tests();
}
