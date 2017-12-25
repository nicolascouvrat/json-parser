#ifndef __INDEX_H__
#define __INDEX_H__

#define MOLECULE_SPLIT_KEY ','
#define ATOM_SPLIT_KEY ':'

#define RETURN_SUCCESS 0
#define ERR_INVAL -1
#define ERR_STRICT_JSON -2

typedef enum {
  ATOM_TYPE_UNDEFINED = 0,
  ATOM_TYPE_PRIMITIVE = 1,
  ATOM_TYPE_STRING = 2
} json_atom_type_t;

typedef struct json_atom {
  char *start;
  char *end;
  json_atom_type_t type;
} json_atom_t;

typedef struct json_molecule {
  json_atom_t *key;
  json_atom_t *value;
} json_molecule_t;

typedef struct json_organism {
  json_molecule_t **molecules;
  int size;
  int total;
  char *ref_string;
  int ref_string_len;
} json_organism_t;

json_atom_t* json_atom_initialize(void);
void json_atom_set(json_atom_t *atom, char* start, char *end, json_atom_type_t type);
void json_atom_destroy(json_atom_t *atom);

json_molecule_t* json_molecule_initialize(void);
void json_molecule_set(json_molecule_t *molecule, json_atom_t *key, json_atom_t *value);
void json_molecule_destroy(json_molecule_t *molecule);

// user needs to provide length!
json_organism_t* json_organism_initialize(int size, char* ref_string, int ref_string_len);
int json_organism_add_molecule(json_organism_t *organism, json_molecule_t *molecule);
void json_organism_destroy(json_organism_t *organism);

#endif
