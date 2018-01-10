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

int t_molecule_matches_string() {
  char* test_string = "\"key\":\"value\"";
  json_atom_t* key = json_atom_initialize();
  json_atom_t* value = json_atom_initialize();
  json_molecule_t* molecule = json_molecule_initialize();

  json_atom_set(key, test_string + 1, test_string + 4, 2);
  json_atom_set(value, test_string + 7, test_string + 12, 2);
  json_molecule_set(molecule, key, value);

  check(json_molecule_matches_string(molecule, "key"));
  check(!json_molecule_matches_string(molecule, "ke"));
  check(!json_molecule_matches_string(molecule, "keyy"));
  check(!json_molecule_matches_string(molecule, "key  "));

  json_molecule_destroy(molecule);

  done();
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

int t_main_process() {
  char *basic_test = "{\"key\":\"value\",\"key2\":12345, \"key3\":\"value\", \"key4\":\"value\", \"key3\":\"value\", \"key4\":\"value\"}";
  // trailing spaces should be ignored (trimmed)
  char *trailing_space_test = "{\"key\" : \"value\",\"key2\":  12345}";
  // spaces in string should be kept intact
  char *space_in_string_test = "{\"key with space\"  :\"value\"}";

  json_organism_t *basic = json_organism_initialize(basic_test);
  json_organism_t *trailing_space = json_organism_initialize(trailing_space_test);
  json_organism_t *space_in_string = json_organism_initialize(space_in_string_test);

  json_atom_t *atom = json_atom_initialize();

  json_noname(basic, basic_test, NULL);
  json_noname(trailing_space, trailing_space_test, NULL);
  json_noname(space_in_string, space_in_string_test, NULL);

  check(json_organism_contains_key(basic,"key"));
  check(json_organism_contains_key(trailing_space, "key"));
  check(json_organism_contains_key(space_in_string, "key with space"));

  json_organism_find(atom, trailing_space, "key2");
  check(json_atom_matches_string(atom, "12345"));

  json_organism_destroy(basic);
  json_organism_destroy(trailing_space);
  json_organism_destroy(space_in_string);
  json_atom_destroy(atom);

  done();
}
/**
 * NOTE ON DETECTED SYNTAX ERRORS:
 * Several errors in formatting can be detected during parsing, namely:
 *    - double closing tokens (e.g. 'stuff:,' or 'stuff,:')
 *    - key without values (e.g. '{"key": "value", "key_alone", "key2": "value"}' or '{"key"}')
 * The way the algorithm works is that ALL tokens are allowed when in quotes (string),
 * so tokens that are usually used to specify a change from key to value (':') or value to next key (',')
 * are IGNORED as long as a '"' has been found before! For example '{"key:value"}' will NOT
 * be parsed correctly (and will result in a "key without value" parsing error)
 *
 * This can however lead to strange behavior in edge cases, for example:
 *    - '{"key:"value", "key2":123}' will correctly return a parsing error (key without value on object closing)
 *    - '{"key:"value", "key2":123"}' will also result in a key without value error
 *    - '{"key:"value", "key2":123":123}' WILL BE PARSED,
 *    resulting in a key of 'key:"value", "key2":123' and a value of '123'. This is
 *    because everytime a special ':' or ',' token is encountered, an odd number of '"'
 *    has been detected and the parser assumes that we are in a string and consequently ignores it,
 *    resulting in a unique key:value pair.
 *
 * The conclusion of this is that, while the parser is equipped with basic typo detection that will be
 * enough in most use cases, it is NOT a JSON syntax checker and should not be used as such.
 *
 * TODO: the case described above could be corrected by raising a flag when a '"' is closed,
 * so that the next char can only be a space ' ' or a special character ':', ',', '}'
 */
int t_main_process_fail() {
  char *first_double_closing_test = "{\"key\":,\"key2\":123}";
  char *second_double_closing_test = "{\"key\":  ,\"key2\":123}";
  char *third_double_closing_test = "{\"key:\"value\",\"key2\":123\"}";

  json_organism_t *first_double_closing = json_organism_initialize(first_double_closing_test);
  json_organism_t *second_double_closing = json_organism_initialize(second_double_closing_test);
  json_organism_t *third_double_closing = json_organism_initialize(third_double_closing_test);

  char* first_test = json_noname(first_double_closing, first_double_closing_test, NULL);
  char* second_test = json_noname(second_double_closing, second_double_closing_test, NULL);
  char* third_test = json_noname(third_double_closing, third_double_closing_test, NULL);

  printf("%d", GET_SIZE(third_double_closing));

  check(first_test == NULL);
  check(second_test == NULL);
  check(third_test == NULL);

  json_organism_destroy(first_double_closing);
  json_organism_destroy(second_double_closing);
  json_organism_destroy(third_double_closing);

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
  test(
    t_main_process,
    "parse a simple JSON string (no nested objects), "
    "and should handle spaces correctly (keep the ones in string, ignore others)"
  );
  test(t_molecule_matches_string, "match a string with the molecule key");
  test(
    t_main_process_fail,
    "parse a simple JSON string (no nested objects), "
    "and fails if string is incorrect"
  );

  test_session_end();
  done();
}
