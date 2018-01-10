#ifndef __JSPR_H__
#define __JSPR_H__

#define MOLECULE_SPLIT_KEY ','
#define ATOM_SPLIT_KEY ':'

#define RETURN_SUCCESS 0
#define ERR_INVAL -1
#define ERR_STRICT_JSON -2

typedef enum {
  ATOM_TYPE_UNDEFINED = 0,
  ATOM_TYPE_PRIMITIVE = 1,
  ATOM_TYPE_STRING = 2
} jspr_atom_type_t;

typedef struct jspr_atom {
  char *start;
  char *end;
  jspr_atom_type_t type;
} jspr_atom_t;

typedef struct jspr_molecule {
  jspr_atom_t *key;
  jspr_atom_t *value;
} jspr_molecule_t;

typedef struct jspr_organism {
  jspr_molecule_t **molecules;
  int size;
  int total;
  char *ref_string;
  int ref_string_len;
} jspr_organism_t;

jspr_atom_t* jspr_atom_initialize(void);
void jspr_atom_set(jspr_atom_t *atom, char* start, char *end, jspr_atom_type_t type);
void jspr_atom_destroy(jspr_atom_t *atom);

jspr_molecule_t* jspr_molecule_initialize(void);
void jspr_molecule_set(jspr_molecule_t *molecule, jspr_atom_t *key, jspr_atom_t *value);
void jspr_molecule_destroy(jspr_molecule_t *molecule);

jspr_organism_t* jspr_organism_initialize(int size, char* ref_string, int ref_string_len);
int jspr_organism_add_molecule(jspr_organism_t *organism, jspr_molecule_t *molecule);
void jspr_organism_destroy(jspr_organism_t *organism);

int jspr_organism_populate(jspr_organism_t *organism);

int jspr_size(char* string, int string_len);
int jspr_organism_contains_key(jspr_organism_t *organism, char *key);
int jspr_organism_find(jspr_atom_t *atom, jspr_organism_t *organism, char *key);

#endif
