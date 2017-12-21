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

void* _display_parsing_error_and_return_null(char *description, char *string, int length) {
  fprintf(
    stderr,
    "JSON_PARSER PARSING ERROR: %s. Check around %s (length %d)\n",
    description, string, length
  );
  return NULL;
}

/**
 * returns a pointer to the first needle between start and end,
 * or NULL if not found
 * @param  needle the char being researched
 * @param  start  pointer to the beginning of search zone
 * @param  end    pointer to end of search zone
 * @return        pointer to the first needle, or NULL if not found
 */

char* _find_first_char_between(char needle, char *start, char *end) {
  char *pointer = start;
  while (pointer != end) {
    if (*pointer == needle)
      return pointer;
    pointer++;
  }
  return NULL;
}

/**
 * Data lifecycle methods for
 *   json_atom_t
 *   json_molecule_t
 *   json_organism_t
 * including initialization, value set and destruction
 */

json_atom_t* json_atom_initialize(void) {
  json_atom_t *atom = malloc(sizeof(json_atom_t));
  if (atom == NULL)
    _display_error_and_exit(errno);
  return atom;
}

void json_atom_set(json_atom_t *atom, char *start, char *end, char* type) {
  atom->start = start;
  atom->end = end;
  atom->type = type;
}

void json_atom_destroy(json_atom_t *atom) {
  // since free marks the *target* of the pointer as free
  // we do NOT want to free start & end
  // as we do not want to alter the initial string
  free(atom);
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

json_organism_t* json_organism_initialize(int size, char *ref_string, int ref_string_len) {
  if (*ref_string != '{' || *(ref_string + ref_string_len - 1) != '}')
    return _display_parsing_error_and_return_null("Invalid organism.", ref_string, ref_string_len);
  json_organism_t *organism = malloc(sizeof(json_organism_t));
  if (organism == NULL)
    _display_error_and_exit(errno);

  organism->size = size;
  organism->total = 0;
  organism->ref_string = ref_string;
  organism->ref_string_len = ref_string_len;
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

/**
 * Print methods for
 *   json_atom_t
 *   json_molecule_t
 *   json_organism_t
 * (used for test)
 */

void _json_atom_print(json_atom_t *atom) {
  printf("JSON_ATOM\n"
         "  start: %s\n"
         "  end: %s\n",
         atom->start,
         atom->end);
}

void _json_molecule_print(json_molecule_t* molecule) {
  printf("JSON_MOLECULE\n"
         "#KEY:\n");
  _json_atom_print(molecule->key);
  printf("#VALUE:\n");
  _json_atom_print(molecule->value);
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

/**
 * what is expected here are two pointers delimiting a string of shape
 * "some stuff" OR some_stuff (first case are string value, other is number)
 * ie: "some stuff"        OR 123456789
 *     ^start     ^end        ^start  ^end
 * @param  start begin
 * @param  end   end
 * @return       new json atom
 */
json_atom_t* create_atom(char* start, char* end) {
  char is_string_key = '\"';
  int length = end - start;
  char *atom_start;
  char *atom_end;
  char *atom_type;
  if (*start == is_string_key) {
    // we have a string token
    atom_type = ATOM_TYPE_STRING;
    atom_start = start + 1;
    if (*(start + length) != is_string_key)
      return _display_parsing_error_and_return_null("Invalid atom", start, length);
    atom_end = start + length;
  } else {
    // we have a number token
    atom_type = ATOM_TYPE_NUMBER;
    atom_start = start;
    if (*(start + length) == is_string_key)
      return _display_parsing_error_and_return_null("Invalid atom", start, length);
    atom_end = end + 1;
  }
  json_atom_t *atom = json_atom_initialize();
  json_atom_set(atom, atom_start, atom_end, atom_type);
  return atom;
}

/**
 * what is expected here pointers delimiting string of shape
 * "key":value OR "key":"string_value"
 * @param  start begin
 * @param  end   end
 * @return       a new json molecule
 */

json_molecule_t* create_molecule(char* start, char* end) {
  char *split_pointer = _find_first_char_between(ATOM_SPLIT_KEY, start, end);
  if (split_pointer == NULL)
    return _display_parsing_error_and_return_null("Invalid molecule", start, end - start);
  json_atom_t *key = create_atom(start, split_pointer - 1);
  // test that the key is indeed surrounded by " "
  if (key->type != ATOM_TYPE_STRING) {
    json_atom_destroy(key);
    return _display_parsing_error_and_return_null(
      "Invalid molecule (key is invalid)",
      start, end - split_pointer + 1
    );
  }
  json_atom_t *value = create_atom(split_pointer + 1, end);
  json_molecule_t *molecule = json_molecule_initialize();
  json_molecule_set(molecule, key, value);
  return molecule;
}

void json_organism_populate(json_organism_t *organism) {
  // skip the first {
  char *ref_string_start = organism->ref_string;
  char *ref_string_end = organism->ref_string + organism->ref_string_len - 1;
  // should be {some stuff that looks like json}
  //           ^start                          ^end
  char *pointer = ref_string_start;
  char *needle = _find_first_char_between(
    MOLECULE_SPLIT_KEY,
    pointer,
    ref_string_end
  );
  int counter = 0;
  while (needle != NULL) {
    printf("needle %s\n", needle);
    json_molecule_t *molecule = create_molecule(pointer + 1, needle -1);

    if (molecule != NULL)
      json_organism_add_molecule(organism, molecule);
    pointer = needle++;
    counter++;
    needle = _find_first_char_between(
      MOLECULE_SPLIT_KEY,
      needle,
      ref_string_end
    );
  }
  // still need to process the last one
  json_molecule_t *molecule = create_molecule(pointer + 1, ref_string_end - 1);
  if (molecule != NULL) {
    json_organism_add_molecule(organism, molecule);
    counter ++;
  }
  if (counter != organism->size)
    fprintf(stderr, "JSON_PARSER FATAL: counter != size!\n");
}

char* get_value_from_atom(json_atom_t *atom) {
  int length = atom->end - atom->start;
  char *string = malloc(sizeof(char) * (length + 1));
  strncpy(string, atom->start, length);
  string[length] = '\0';
  return string;
}

int json_molecule_matches_string(json_molecule_t *molecule, char* string) {
  int counter = 0;
  int key_len = molecule->key->end - molecule->key->start;
  char *pointer = string;
  while (*pointer && counter <= key_len) {
    if (*pointer++ != molecule->key->start[counter])
      return 0;
    counter ++;
  }
  if (counter != key_len)
    return 0;
  return 1;
}

int json_organism_contains_key(json_organism_t *organism, char *key) {
  int i;
  for (i = 0; i < organism ->size; i++) {
    if (json_molecule_matches_string(organism->molecules[i], key))
      return 1;
  }
  return 0;
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
