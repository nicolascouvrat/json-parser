# JSPR

JSPR (pronounced "jasper") is a C library dedicated to parsing JSON strings, used as a subroutine in one of my IoT projects. As such, it is heavily tailored to fit my needs, and does not (yet) incorporate many features that a full fledged library would need.

Although implementation is planned at a later date, the parser supports only primitive types and strings as values (i.e. no nested objects or arrays).

Still, the parser in itself is (decently) efficient, with a time complexity of `O(n)` (this is perfectible: as of now, the parser does 3 loops over the string). To save space, the parser consists of an array of pointers to the initial string, marking the beginning and end of each `atom`.

## Installation

After cloning this repository, run `make` from the `src` directory. You can then install using `make install` (this might require administrator privileges).

The code is fully covered by tests, that can be launched using `make test`.

### Dependencies

None! The idea is to stay as simple and library-free as possible. The code only uses `stdlib`, `stdio` and `string`, and tries to stay concise.

### Usage

```c
char *json_string = "{\"key1\":\"value\",\"key2\":12345,\"key3\":\"value\"}";

int string_len = strlen(json_string);
// run preliminary checks to see if the string is a valid JSON string,
// and to assess the number of atoms
int number_of_pairs = jspr_size(json_string, string_len);
if (number_of_pairs == -1) {
  // the string was invalid
}
jspr_organism_t *parser = jspr_organism_initialize(number_of_pairs, json_string, string_len);
if ((r = jspr_organism_populate(parser)) != 0) {
  // parsing error
}
// from on, the string has been parsed.
// Checks can be made for keys...
char *key = "key3";
int test = jspr_organism_contains_key(organism, key);
// ... and values retrieved as a jspr_atom_t
jspr_atom_t *value = jspr_atom_initialize();
if (jspr_organism_find(value, organism, key)) {
  // value has been found
}

// ...

// don't forget to free the structures
jspr_atom_destroy(value);
jspr_organism_destroy(parser);
```

## Documentation

### Data structures

The parser uses pointers to the initial string to split it in units of various sizes:

* **`jspr_atom_t`**: smallest unit, such as a value, a key...,
* **`jspr_molecule_t`**: a `key:value` pair, composed of two `jspr_atom_t`,
* **`jspr_organism_t`**: a representation of the object behind the JSON string, containing an array of pointers towards individual `jspr_molecule_t`.

`jspr_atom_t` has the following structure:

```c
typedef struct jspr_atom{
  char* start;
  char* end;
  jspr_atom_type_t type;
} jspr_atom_t;
```
where `jspr_atom_type_t` defines the type of the atom, that can be `ATOM_TYPE_STRING` or `ATOM_TYPE_PRIMITIVE`, and `start`, `end` are pointers to the starting and ending position of the atom in the original string.

### Complexity

The pointer system allows for a `O(M)` space complexity, where `M` is the number of atoms in the initial string.

Time complexity for the parser is `O(n)`.

Time complexity of retrieval operations is `O(n)`, as the library does not implement a hash table yet.

### Robustness

As of today, the parser checks that:
* the number of `:` is equal to the number of `,` plus one,
* atoms either start _and_ end with `"` (strings), or do not have any `"` (primitives),
* atoms that are keys (left of `:`) are of type string (strict JSON)

Outside of this, the parser assumes a standard JSON format (for instance, no spaces around `,` or `:`), _and will break if this is not the case._

## TODO list

In order of importance:

* improve robustness by adding more checks,
* improving search complexity by implementing a hash table for data retrieving,
* adding nested object and arrays support,
* improving parsing speed by reducing constants.
