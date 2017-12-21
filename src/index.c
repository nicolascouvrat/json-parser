#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "./index.h"

#define TEST_STRING "{\"key1\":\"value1\",\"key2\":5,\"key3\":\"value3\"}"
#define TEST_MOLECULE_STRING "\"key1\":\"value1\""
#define ATOM_TEST_STRING "\"key1\""

/**
 * utility: display an standardized error message and exits the program
 * @param err_num error code
 */

void _display_error_and_exit(int err_num) {
  fprintf(stderr, "JSON_PARSER FATAL: %s (%d)\n", strerror(err_num), err_num);
  exit(EXIT_FAILURE);
}

json_atom_t* json_atom_initialize(void) {
  json_atom_t *atom = malloc(sizeof(json_atom_t));
  if (atom == NULL)
    _display_error_and_exit(errno);
  return atom;
}

void json_atom_set(json_atom_t *atom, char *start, char *end) {
  atom->start = start;
  atom->end = end;
}

void json_atom_destroy(json_atom_t *atom) {
  // since free marks the *target* of the pointer as free
  // we do NOT want to free start & end
  // as we do not want to alter the initial string
  free(atom);
}

void _json_atom_print(json_atom_t *atom) {
  printf("JSON_ATOM\n"
         "  start: %s\n"
         "  end: %s\n",
         atom->start,
         atom->end);
}

json_molecule_t* json_molecule_initialize(void) {
  json_molecule_t *molecule = malloc(sizeof(json_molecule_t));
  if (molecule == NULL)
    _display_error_and_exit(errno);
  return molecule;
}

void json_molecule_set(json_molecule_t *molecule, json_atom_t *key, json_atom_t *value) {
  molecule->key = key;
  molecule->value = value;
}

void json_molecule_destroy(json_molecule_t *molecule) {
  json_atom_destroy(molecule->key);
  json_atom_destroy(molecule->value);
  free(molecule);
}

void _json_molecule_print(json_molecule_t* molecule) {
  printf("JSON_MOLECULE\n"
         "#KEY:\n");
  _json_atom_print(molecule->key);
  printf("#VALUE:\n");
  _json_atom_print(molecule->value);
}

json_organism_t* json_organism_initialize(int size, char *ref_string) {
  json_organism_t *organism = malloc(sizeof(json_organism_t));
  if (organism == NULL)
    _display_error_and_exit(errno);

  organism->size = size;
  organism->total = 0;
  organism->ref_string = ref_string;
  organism->molecules = malloc(sizeof(json_molecule_t*) * size);
  if (organism->molecules == NULL)
    _display_error_and_exit(errno);

  return organism;
}

int json_organism_add_molecule(json_organism_t* organism, json_molecule_t *molecule) {
  if (organism->size == organism->total)
    return -1;
  organism->molecules[organism->total++] = molecule;
  return 0;
}

void json_organism_destroy(json_organism_t *organism) {
  // json_molecule_t **molecule = organism->molecules;
  int i;
  for (i = 0; i < organism->size; i++) {
    json_molecule_destroy(organism->molecules[i]);
  }
  free(organism->molecules);
  free(organism);
}

void _json_organism_print(json_organism_t *organism) {
  printf("\nJSON_ORGANISM\n");
  int i;
  for (i = 0; i < organism->size; i++) {
    printf("MOLECULE %d\n", i);
    _json_molecule_print(organism->molecules[i]);
  }
}


/**
 * parses the string, counting the number of both separators
 * tests if the string is a valid JSON string
 *  defined for now as #(MOLECULE_SEP) + 1 = #(ATOM_SEP)
 *
 * @param  string the string to test
 * @return        number of molecules in string, or -1 if invalid
 */
int json_size(char *string) {
  int atom_sep_counter = 0;
  int molecule_sep_counter = 0;
  char *c = string;
  while (*c) {
    if (*c == MOLECULE_SPLIT_KEY)
      molecule_sep_counter++;
    if (*c == ATOM_SPLIT_KEY)
      atom_sep_counter++;
    c++;
  }
  // check if we have a valid JSON shape
  if (atom_sep_counter != molecule_sep_counter + 1)
    return -1;
  // return number of molecules (sep + 1)
  return molecule_sep_counter + 1;
}

int process(char *string) {
  int size = json_size(string);
  if (size == -1) {
    fprintf(stderr, "Invalid JSON: %s\n", string);
    return -1;
  }
  printf("Number of molecules: %d \n", size);
  return 0;
}



// int main() {
//   printf("Now testing %s\n", TEST_STRING);
//   return 0;
// }
