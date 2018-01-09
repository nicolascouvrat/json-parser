#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "./index.h"
#include "./DynamicArray.h"

#define GET_MOLECULE(organism, index) (json_molecule_t*)dynamic_array_get(organism->molecules, index)
#define GET_SIZE(organism) organism->molecules->size


/**
 * Print methods for
 *   json_atom_t
 *   json_molecule_t
 *   json_organism_t
 * (used for test)
 */

void _json_atom_print(json_atom_t *atom) {
  printf("JSON_ATOM OF TYPE %d\n"
         "  start: %s\n"
         "  end: %s\n",
         atom->type,
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
  for (i = 0; i < organism->molecules->size; i++) {
    printf("MOLECULE %d\n", i);
    _json_molecule_print((json_molecule_t*)organism->molecules->items[i]);
  }
}

/**
 * utility: display an standardized error message and exits the program
 * @param err_num error code
 */

void _display_error_and_exit(int err_num) {
  fprintf(stderr, "JSON_PARSER FATAL ERROR: %s (%d)\n", strerror(err_num), err_num);
  exit(EXIT_FAILURE);
}

int _display_error_and_return(int err_num, char *string, int length) {
  #ifdef __DEBUG__
  fprintf(stderr, "JSON_PARSER ERROR: %d. Check around ", err_num);
  int i;
  for (i = 0; i < length; i++) {
    putchar(string[i]);
  }
  putchar('\n');
  #endif

  return err_num;
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
 * including an initializer, a setter and a destructor for each structure
 *
 * NOTE: although in most regular use cases setting pointers to NULL at init is kind of a waste
 * (because a setter will always be called somewhere before a destructor)
 * choice has been made to do so in order to allow the use of destructor immediately
 * after an initializer, which seemed like the most coherent behavior.
 */

json_atom_t* json_atom_initialize(void) {
  json_atom_t *atom = malloc(sizeof(json_atom_t));
  atom->type = ATOM_TYPE_UNDEFINED;
  if (atom == NULL)
    _display_error_and_exit(errno);
  return atom;
}

void json_atom_set(json_atom_t *atom, char *start, char *end, json_atom_type_t type) {
  atom->start = start;
  atom->end = end;
  atom->type = type;
}

void json_atom_destroy(json_atom_t *atom) {
  // _json_atom_print(atom);
  // printf("call atom destroy\n");
  free(atom);
}

json_molecule_t* json_molecule_initialize(void) {
  json_molecule_t *molecule = malloc(sizeof(json_molecule_t));
  if (molecule == NULL)
    _display_error_and_exit(errno);
  molecule->key = NULL;
  molecule->value = NULL;
  return molecule;
}

void json_molecule_set(json_molecule_t *molecule, json_atom_t *key, json_atom_t *value) {
  molecule->key = key;
  molecule->value = value;
}

void json_molecule_destroy(json_molecule_t *molecule) {
  if (molecule == NULL) return;
  json_atom_destroy(molecule->key);
  json_atom_destroy(molecule->value);
  free(molecule);
}

json_organism_t* json_organism_initialize(char *ref_string) {
  // in the end, this will be included somewhere else to avoid the extra cost of strlen
  // int ref_string_len = strlen(ref_string);
  json_organism_t *organism = malloc(sizeof(json_organism_t));

  if (organism == NULL)
    _display_error_and_exit(errno);

  organism->ref_string = ref_string;
  organism->molecules = dynamic_array_initialize();

  return organism;
}

int json_organism_add_molecule(json_organism_t* organism, json_molecule_t *molecule) {
  return dynamic_array_push(organism->molecules, molecule);
}

void json_organism_destroy(json_organism_t *organism) {
  if (organism == NULL) return;
  int i;
  int max = GET_SIZE(organism);
  // the condition i < max is evaluated before each pass
  // so we need to store the size once
  for (i = 0; i < max; i++) {
    json_molecule_destroy((json_molecule_t*)dynamic_array_pop(organism->molecules));
  }
  dynamic_array_destroy(organism->molecules);
  free(organism);
}



int json_atom_matches_string(json_atom_t *atom, char *string) {
  int counter = 0;
  int key_len = atom->end - atom->start;
  char *pointer = string;
  while (*pointer && counter <= key_len) {
    if (*pointer++ != atom->start[counter])
      return 0;
    counter ++;
  }
  if (counter != key_len)
    return 0;
  return 1;
}

/**
 * Tests if a molecules matches a specific key,
 * by comparing the *values* of that molecules' key atom with the given key
 * @param  molecule pointer to the molecule
 * @param  string   key to test against
 * @return          1 if match, 0 if not
 */
int json_molecule_matches_string(json_molecule_t *molecule, char* string) {
  return json_atom_matches_string(molecule->key, string);
}

/**
 * Tests if an organism contains a specific key
 * @param  organism pointer to the organism
 * @param  key      key to search for
 * @return          1 if found, 0 if not
 */
int json_organism_contains_key(json_organism_t *organism, char *key) {
  int i;
  for (i = 0; i < GET_SIZE(organism); i++) {
    if (json_molecule_matches_string(GET_MOLECULE(organism, i), key))
      return 1;
  }
  return 0;
}

/**
 * Tests if an organism contains a specific key,
 * and fills an atom structure with the value associated to that key (if found)
 * @param  atom     pointer to an atom structure
 * @param  organism pointer to an organism structure
 * @param  key      key to search for
 * @return          1 if found, 0 if not
 */
int json_organism_find(json_atom_t *atom, json_organism_t *organism, char *key) {
  int i;
  for (i = 0; i < GET_SIZE(organism); i++) {
    if (json_molecule_matches_string(GET_MOLECULE(organism, i), key)) {
      json_molecule_t *molecule = GET_MOLECULE(organism, i);
      json_atom_set(
        atom,
        molecule->value->start,
        molecule->value->end,
        molecule->value->type
      );
      return 1;
    }
  }
  return 0;
}

/**
 * returns a pointer to the end of the nearest previous atom,
 * essentially ignoring spaces and thus allowing the parsing of
 * JSON containing spaces as 'padding' between tokens
 * This will also give the previous atom a type of
 * ATOM_TYPE_PRIMITIVE or ATOM_TYPE_STRING
 *
 * @param  atom     pointer to the atom structure to fill
 * @param  end_mark supposed to be the 'close mark' (':' or ',')
 * @return 0
 */
int backtrack(json_atom_t* atom, char *end_mark) {
  do {
    end_mark--;
  } while(*end_mark == ' ');
  if (*end_mark == '"') {
    atom->end = end_mark;
    atom->type = ATOM_TYPE_STRING;
    return 0;
  }
  atom->end = end_mark + 1;
  atom->type = ATOM_TYPE_PRIMITIVE;
  return 0;
}

char* json_noname(json_organism_t *organism, char* start, json_molecule_t *parent) {
  int current_index = GET_SIZE(organism);
  int is_key = 1;
  int is_atom_opened = 0;
  int is_string_opened = 0;
  char *c = start;
  while (*c) {
    if (*c == '}') {
      if (is_string_opened || is_key) {
        // we have an error here
        printf("should not close with '}' on a key alone or if string is opened!\n");
        return NULL;
      }
      if (!is_atom_opened) {
        // special case of empty item
        // TODO: special treatment?
        printf("The parser does not handle empty objects for now!\n");
        return NULL;
      }
      backtrack((GET_MOLECULE(organism, current_index))->value, c);
      return c;
    }
    else if (*c == ':' && !is_string_opened) {
      if (!is_atom_opened) {
        // this is the "key":"value",: case, or "key"::
        // (double closing token)
        // TODO: figure a way to throw?
        printf("double closing token error!\n");
        return NULL;
      }
      backtrack((GET_MOLECULE(organism, current_index))->key, c);
      is_key = 0;
      is_atom_opened = 0;
    }
    else if (*c == ',' && !is_string_opened) {
      if (!is_atom_opened) {
        printf("double closing token error!\n");
        return NULL;
      }
      backtrack((GET_MOLECULE(organism, current_index))->value, c);
      current_index++;
      is_atom_opened = 0;
      is_key = 1;
    }
    else if (*c == '"') is_string_opened = !is_string_opened;
    else if (*c == ' ' && !is_string_opened);
    else if (*c == '{' && !is_string_opened); // TODO: recursion
    else if (!is_atom_opened) {
      if (is_key) {
        json_molecule_t *molecule = json_molecule_initialize();
        json_atom_t *key = json_atom_initialize();
        key->start = c;
        molecule->key = key;
        dynamic_array_push(organism->molecules, molecule);
      } else {
        json_atom_t *value = json_atom_initialize();
        value->start = c;
        (GET_MOLECULE(organism, current_index))->value = value;
      }
      is_atom_opened = 1;
    }
    c ++;
  }
}
