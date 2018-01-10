#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "./jspr.h"

/**
 * utility: display an standardized error message and exits the program
 * @param err_num error code
 */

void _display_error_and_exit(int err_num) {
  fprintf(stderr, "JSPR FATAL ERROR: %s (%d)\n", strerror(err_num), err_num);
  exit(EXIT_FAILURE);
}

int _display_error_and_return(int err_num, char *string, int length) {
  #ifdef __DEBUG__
  fprintf(stderr, "JSPR ERROR: %d. Check around ", err_num);
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
 *   jspr_atom_t
 *   jspr_molecule_t
 *   jspr_organism_t
 * including an initializer, a setter and a destructor for each structure
 *
 * NOTE: although in most regular use cases setting pointers to NULL at init is kind of a waste
 * (because a setter will always be called somewhere before a destructor)
 * choice has been made to do so in order to allow the use of destructor immediately
 * after an initializer, which seemed like the most coherent behavior.
 */

jspr_atom_t* jspr_atom_initialize(void) {
  jspr_atom_t *atom = malloc(sizeof(jspr_atom_t));
  if (atom == NULL)
    _display_error_and_exit(errno);
  return atom;
}

void jspr_atom_set(jspr_atom_t *atom, char *start, char *end, jspr_atom_type_t type) {
  atom->start = start;
  atom->end = end;
  atom->type = type;
}

void jspr_atom_destroy(jspr_atom_t *atom) {
  free(atom);
}

jspr_molecule_t* jspr_molecule_initialize(void) {
  jspr_molecule_t *molecule = malloc(sizeof(jspr_molecule_t));
  if (molecule == NULL)
    _display_error_and_exit(errno);
  molecule->key = NULL;
  molecule->value = NULL;
  return molecule;
}

void jspr_molecule_set(jspr_molecule_t *molecule, jspr_atom_t *key, jspr_atom_t *value) {
  molecule->key = key;
  molecule->value = value;
}

void jspr_molecule_destroy(jspr_molecule_t *molecule) {
  if (molecule == NULL) return;
  jspr_atom_destroy(molecule->key);
  jspr_atom_destroy(molecule->value);
  free(molecule);
}

jspr_organism_t* jspr_organism_initialize(int size, char *ref_string) {
  // in the end, this will be included somewhere else to avoid the extra cost of strlen
  int ref_string_len = strlen(ref_string);
  jspr_organism_t *organism = malloc(sizeof(jspr_organism_t));

  if (organism == NULL)
    _display_error_and_exit(errno);

  organism->size = size;
  organism->total = 0;
  organism->ref_string = ref_string;
  organism->ref_string_len = ref_string_len;
  organism->molecules = malloc(sizeof(jspr_molecule_t*) * size);

  if (organism->molecules == NULL)
    _display_error_and_exit(errno);

  int i;
  for (i = 0; i < size; i++) {
    organism->molecules[i] = NULL;
  }
  return organism;
}

int jspr_organism_add_molecule(jspr_organism_t* organism, jspr_molecule_t *molecule) {
  if (organism->size == organism->total)
    return -1;
  organism->molecules[organism->total++] = molecule;
  return 0;
}

void jspr_organism_destroy(jspr_organism_t *organism) {
  if (organism == NULL) return;
  int i;
  for (i = 0; i < organism->size; i++) {
    jspr_molecule_destroy(organism->molecules[i]);
  }
  free(organism->molecules);
  free(organism);
}

/**
 * Print methods for
 *   jspr_atom_t
 *   jspr_molecule_t
 *   jspr_organism_t
 * (used for test)
 */

void _jspr_atom_print(jspr_atom_t *atom) {
  printf("JSPR_ATOM\n"
         "  start: %s\n"
         "  end: %s\n",
         atom->start,
         atom->end);
}

void _jspr_molecule_print(jspr_molecule_t* molecule) {
  printf("JSPR_MOLECULE\n"
         "#KEY:\n");
  _jspr_atom_print(molecule->key);
  printf("#VALUE:\n");
  _jspr_atom_print(molecule->value);
}

void _jspr_organism_print(jspr_organism_t *organism) {
  printf("\nJSPR_ORGANISM\n");
  int i;
  for (i = 0; i < organism->size; i++) {
    printf("MOLECULE %d\n", i);
    _jspr_molecule_print(organism->molecules[i]);
  }
}


/**
 * parses the string, counting the number of both separators
 * tests if the string is a valid JSON string, defined for now as:
 *    #(MOLECULE_SPLIT_KEY) + 1 = #(ATOM_SPLIT_KEY)
 *
 * @param  string the string to test
 * @return        number of molecules in string, or -1 if invalid
 */
int jspr_size(char *string) {
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
 * populates the atom structure, performing a few structural checks along the way
 * TODO: should perhaps move the test somewhere else?
 * what is expected here are two pointers delimiting a string of shape
 * "some stuff" OR some_stuff (first case are string value, other is number)
 * ie: "some stuff"        OR 123456789
 *     ^start      ^end       ^start   ^end
 *
 * @param  atom  pointer to the atom structure
 * @param  start
 * @param  end
 * @return       error code
 */
int jspr_atom_populate(jspr_atom_t *atom, char* start, char* end) {
  char is_string_key = '\"';
  int length = end - start;
  char *atom_start;
  char *atom_end;
  jspr_atom_type_t atom_type;
  if (*start == is_string_key) {
    // we have a string token
    atom_type = ATOM_TYPE_STRING;
    atom_start = start + 1;
    if (*(start + length - 1) != is_string_key)
      return _display_error_and_return(ERR_INVAL, start, length);
    atom_end = start + length - 1;
  } else {
    // we have a number token
    atom_type = ATOM_TYPE_PRIMITIVE;
    atom_start = start;
    if (*(start + length - 1) == is_string_key)
      return _display_error_and_return(ERR_INVAL, start, length);
    atom_end = end;
  }
  jspr_atom_set(atom, atom_start, atom_end, atom_type);
  return RETURN_SUCCESS;
}

/**
 * Populates the molecule structure
 * what is expected here pointers delimiting string of shape
 * "key":value        OR "key":"string_value"
 * ^start     ^end       ^start              ^end
 *
 * @param  molecule pointer to the molecule structure
 * @param  start
 * @param  end
 * @return          error code
 */

int jspr_molecule_populate(jspr_molecule_t *molecule, char* start, char* end) {
  char *split_pointer = _find_first_char_between(ATOM_SPLIT_KEY, start, end);
  int r;
  if (split_pointer == NULL)
    return _display_error_and_return(ERR_INVAL, start, end - start);
  jspr_atom_t *key = jspr_atom_initialize();
  if ((r = jspr_atom_populate(key, start, split_pointer)) != RETURN_SUCCESS) {
    jspr_atom_destroy(key);
    return r;
  }
  // test that the key is an atom of type string (strict JSON)
  if (key->type != ATOM_TYPE_STRING) {
    // free malloced memory (avoid leaks)
    jspr_atom_destroy(key);
    return _display_error_and_return(ERR_STRICT_JSON, start, end - split_pointer);
  }

  jspr_atom_t *value = jspr_atom_initialize();
  if((r = jspr_atom_populate(value, split_pointer + 1, end)) != RETURN_SUCCESS) {
    jspr_atom_destroy(value);
    jspr_atom_destroy(key);
    return r;
  }

  jspr_molecule_set(molecule, key, value);
  return RETURN_SUCCESS;
}

/**
 * Populates the organism structure by parsing the JSON string ref_string
 *
 * @param  organism pointer to the organism structure
 * @return          error code
 */
int jspr_organism_populate(jspr_organism_t *organism) {
  // skip the first {
  char *ref_string_start = organism->ref_string;
  char *ref_string_end = organism->ref_string + organism->ref_string_len;
  // should be {some stuff that looks like jspr}
  //           ^start                           ^end
  char *pointer = ref_string_start;
  char *needle = _find_first_char_between(
    MOLECULE_SPLIT_KEY,
    pointer,
    ref_string_end
  );
  int counter = 0;
  int r;
  while (needle != NULL) {
    jspr_molecule_t *molecule = jspr_molecule_initialize();
    if ((r = jspr_molecule_populate(molecule, pointer + 1, needle)) != RETURN_SUCCESS) {
      jspr_molecule_destroy(molecule);
      return r;
    }
    jspr_organism_add_molecule(organism, molecule);
    pointer = needle++;
    counter++;
    needle = _find_first_char_between(
      MOLECULE_SPLIT_KEY,
      needle,
      ref_string_end
    );
  }
  // still need to process the last one
  jspr_molecule_t *molecule = jspr_molecule_initialize();
  if ((r = jspr_molecule_populate(molecule, pointer + 1, ref_string_end - 1)) != RETURN_SUCCESS) {
    jspr_molecule_destroy(molecule);
    return r;
  }
  jspr_organism_add_molecule(organism, molecule);

  #ifdef __DEBUG__
  printf("Finish populating organism of size %d, added %d molecules.\n", organism->size, counter + 1);
  #endif

  return 0;
}

/**
 * Tests if a molecules matches a specific key,
 * by comparing the *values* of that molecules' key atom with the given key
 * @param  molecule pointer to the molecule
 * @param  string   key to test against
 * @return          1 if match, 0 if not
 */
int jspr_molecule_matches_string(jspr_molecule_t *molecule, char* string) {
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

/**
 * Tests if an organism contains a specific key
 * @param  organism pointer to the organism
 * @param  key      key to search for
 * @return          1 if found, 0 if not
 */
int jspr_organism_contains_key(jspr_organism_t *organism, char *key) {
  int i;
  for (i = 0; i < organism ->size; i++) {
    if (jspr_molecule_matches_string(organism->molecules[i], key))
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
int jspr_organism_find(jspr_atom_t *atom, jspr_organism_t *organism, char *key) {
  int i;
  for (i = 0; i < organism->size; i++) {
    if (jspr_molecule_matches_string(organism->molecules[i], key)) {
      jspr_atom_set(
        atom,
        organism->molecules[i]->value->start,
        organism->molecules[i]->value->end,
        organism->molecules[i]->value->type
      );
      return 1;
    }
  }
  return 0;
}
