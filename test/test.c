#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "./testutil.h"
#include "../src/jspr.c"

char* pointer_to_end_of_string(char* string) {
  int len = strlen(string);
  return string + len;
}

int t_nested_molecule_matches_string() {
  char* test_string = "\"key\":{\"key2\":123}";

  jspr_atom_t* first_key = jspr_atom_initialize();
  jspr_atom_t* first_value = jspr_atom_initialize();
  jspr_molecule_t* first_molecule = jspr_molecule_initialize();
  jspr_atom_t* second_key = jspr_atom_initialize();
  jspr_atom_t* second_value = jspr_atom_initialize();
  jspr_molecule_t* second_molecule = jspr_molecule_initialize();

  jspr_atom_set(first_key, test_string + 1, test_string + 4, 2);
  jspr_atom_set(first_value, test_string + 7, test_string + 17, 3);
  jspr_molecule_set(first_molecule, first_key, first_value);

  jspr_atom_set(second_key, test_string + 8, test_string + 12, 2);
  jspr_atom_set(second_value, test_string + 14, test_string + 17, 1);
  jspr_molecule_set(second_molecule, second_key, second_value);
  second_molecule->parent = first_molecule;

  check(jspr_molecule_matches_string(second_molecule, "key.key2"));

  check(!jspr_molecule_matches_string(second_molecule, "key.key2.key3"));
  check(!jspr_molecule_matches_string(second_molecule, "key3.key.key2"));
  check(!jspr_molecule_matches_string(second_molecule, "key2"));
  check(!jspr_molecule_matches_string(second_molecule, "k.key2"));

  jspr_molecule_destroy(first_molecule);
  jspr_molecule_destroy(second_molecule);

  done();
}

int t_molecule_matches_string() {
  char* test_string = "\"key\":\"value\"";
  jspr_atom_t* key = jspr_atom_initialize();
  jspr_atom_t* value = jspr_atom_initialize();
  jspr_molecule_t* molecule = jspr_molecule_initialize();

  jspr_atom_set(key, test_string + 1, test_string + 4, 2);
  jspr_atom_set(value, test_string + 7, test_string + 12, 2);
  jspr_molecule_set(molecule, key, value);

  check(jspr_molecule_matches_string(molecule, "key"));
  check(!jspr_molecule_matches_string(molecule, "ke"));
  check(!jspr_molecule_matches_string(molecule, "keyy"));
  check(!jspr_molecule_matches_string(molecule, "key  "));

  jspr_molecule_destroy(molecule);

  done();
}

int jspr_atom_eq_p(jspr_atom_t *atom, char *start, char *end, jspr_atom_type_t type) {
  char *_start = start, *_end = end;
  if (type == ATOM_TYPE_STRING) {
    _start += 1;
    _end -= 1;
  }
  return (atom->start == _start && atom->end == _end && atom->type == type);
}

int jspr_atom_eq(jspr_atom_t *atom, char *snippet, jspr_atom_type_t type) {
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
  jspr_organism_t *organism = jspr_organism_initialize("");
  jspr_molecule_t *molecule = jspr_molecule_initialize();
  jspr_atom_t *key = jspr_atom_initialize();
  jspr_atom_t *value = jspr_atom_initialize();

  jspr_molecule_set(molecule, key, value);
  jspr_organism_add_molecule(organism, molecule);

  jspr_organism_destroy(organism);
  done();
}

int t_backtrack() {
  char *first_test = "endofkey\":";
  char *second_test = "endofvalue:";
  char *third_test = "threespaces   :";

  jspr_atom_t *atom = jspr_atom_initialize();

  backtrack(atom, pointer_to_end_of_string(first_test) - 1);
  check(atom->end == pointer_to_end_of_string(first_test) - 2);
  check(atom->type == ATOM_TYPE_STRING);

  backtrack(atom, pointer_to_end_of_string(second_test) - 1);
  check(atom->end == pointer_to_end_of_string(second_test) - 1);
  check(atom->type == ATOM_TYPE_PRIMITIVE);

  backtrack(atom, pointer_to_end_of_string(third_test) - 1);
  check(atom->end == pointer_to_end_of_string(third_test) - 4);
  check(atom->type == ATOM_TYPE_PRIMITIVE);

  jspr_atom_destroy(atom);

  done();
}

int t_main_process() {
  char *basic_test = "{\"key\":\"value\",\"key2\":12345, \"key3\":\"value\", \"key4\":\"value\", \"key3\":\"value\", \"key4\":\"value\"}";
  // trailing spaces should be ignored (trimmed)
  char *trailing_space_test = "{\"key\" : \"value\",\"key2\":  12345}";
  // spaces in string should be kept intact
  char *space_in_string_test = "{\"key with space\"  :\"value\"}";

  jspr_organism_t *basic = jspr_organism_initialize(basic_test);
  jspr_organism_t *trailing_space = jspr_organism_initialize(trailing_space_test);
  jspr_organism_t *space_in_string = jspr_organism_initialize(space_in_string_test);

  jspr_atom_t *atom = jspr_atom_initialize();

  jspr_noname(basic, basic_test, NULL);
  jspr_noname(trailing_space, trailing_space_test, NULL);
  jspr_noname(space_in_string, space_in_string_test, NULL);

  check(jspr_organism_contains_key(basic,"key"));
  check(jspr_organism_contains_key(trailing_space, "key"));
  check(jspr_organism_contains_key(space_in_string, "key with space"));

  jspr_organism_find(atom, trailing_space, "key2");
  check(jspr_atom_matches_string(atom, "12345", 5));

  jspr_organism_destroy(basic);
  jspr_organism_destroy(trailing_space);
  jspr_organism_destroy(space_in_string);
  jspr_atom_destroy(atom);

  done();
}
/**
 * NOTE ON DETECTED SYNTAX ERRORS:
 * Several errors in formatting can be detected during parsing, namely:
 *    - double closing tokens (e.g. 'stuff:,' or 'stuff,:')
 *    - key without values (e.g. '{"key": "value", "key_alone", "key2": "value"}' or '{"key"}')
 *    - incorrect usage of '"' (e.g. '{"key: "value"}')
 * The way the algorithm works is that ALL tokens are allowed when in quotes (string),
 * so tokens that are usually used to specify a change from key to value (':') or value to next key (',')
 * are IGNORED as long as a '"' has been found before! For example '{"key:value"}' will NOT
 * be parsed correctly (and will result in a "key without value" parsing error).
 * Moreover, the only characters authorized after a 'closing' (i.e. a even number of) '"'
 * are ',', ':', '}' and ' '. Others will result in a 'incorrect usage of '"'' error.
 *
 * While the parser is equipped with basic typo detection that will be
 * enough in most use cases, it is NOT a JSON syntax checker and should not be used as such.
 */
int t_main_process_fail() {
  char *first_double_closing_test = "{\"key\":,\"key2\":123}";
  char *second_double_closing_test = "{\"key\":  ,\"key2\":123}";

  char *first_string_token_test = "{\"key:\"value\",\"key2\":123\"}";
  char *second_string_token_test = "{\"key:\"value\", \"key2\":123\":123}";

  char *first_key_no_value_test = "{\"key\" : \"value\", \"key2\"}";
  char *second_key_no_value_test = "{\"key:value\"}";

  jspr_organism_t *first_double_closing = jspr_organism_initialize(first_double_closing_test);
  jspr_organism_t *second_double_closing = jspr_organism_initialize(second_double_closing_test);
  jspr_organism_t *first_string_token = jspr_organism_initialize(first_string_token_test);
  jspr_organism_t *second_string_token = jspr_organism_initialize(second_string_token_test);
  jspr_organism_t *first_key_no_value = jspr_organism_initialize(first_key_no_value_test);
  jspr_organism_t *second_key_no_value = jspr_organism_initialize(second_key_no_value_test);

  char* first_test = jspr_noname(first_double_closing, first_double_closing_test, NULL);
  char* second_test = jspr_noname(second_double_closing, second_double_closing_test, NULL);
  char* third_test = jspr_noname(first_string_token, first_string_token_test, NULL);
  char* fourth_test = jspr_noname(second_string_token, second_string_token_test, NULL);
  char* fifth_test = jspr_noname(first_key_no_value, first_key_no_value_test, NULL);
  char* sixth_test = jspr_noname(second_key_no_value, second_key_no_value_test, NULL);

  check(first_test == NULL);
  check(second_test == NULL);
  check(third_test == NULL);
  check(fourth_test == NULL);
  check(fifth_test == NULL);
  check(sixth_test == NULL);

  jspr_organism_destroy(first_double_closing);
  jspr_organism_destroy(second_double_closing);
  jspr_organism_destroy(first_string_token);
  jspr_organism_destroy(second_string_token);
  jspr_organism_destroy(first_key_no_value);
  jspr_organism_destroy(second_key_no_value);

  done();
}

int t_main_process_nested() {
  char *nested_json_test = "{\"key\":123, \"obj\":{\"key2\":\"value\"}}";

  jspr_organism_t *nested_json = jspr_organism_initialize(nested_json_test);

  char *test = jspr_noname(nested_json, nested_json_test, NULL);
  check(test != NULL);
  check(jspr_organism_contains_key(nested_json, "key"));
  check(jspr_organism_contains_key(nested_json, "obj.key2"));

  jspr_organism_destroy(nested_json);

  done();
}

int main() {
  test_session_start();

  // test(t_life_cycle, "life cycle of structures");
  // test(
  //   t_backtrack,
  //   "backtrack function that, given the position of a closing token "
  //   "will return end of previous atom"
  // );
  // test(
  //   t_main_process,
  //   "parse a simple JSON string (no nested objects), "
  //   "and should handle spaces correctly (keep the ones in string, ignore others)"
  // );
  // test(t_molecule_matches_string, "match a string with the molecule's key");
  // test(t_nested_molecule_matches_string, "match a string with a nested molecule's full key");
  // test(
  //   t_main_process_fail,
  //   "parse a simple JSON string (no nested objects), "
  //   "and fails if string is incorrect"
  // );
  test(t_main_process_nested, "parse a JSON string with nested objects");

  test_session_end();
  done();
}
