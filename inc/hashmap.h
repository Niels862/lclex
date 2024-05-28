#ifndef LCLEX_HASHMAP_H
#define LCLEX_HASHMAP_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define LCLEX_HASHMAP_INIT_SIZE 32

#define LCLEX_HASHMAP_LOAD_FACTOR 0.75

typedef uint64_t lclex_hash_t;

typedef lclex_hash_t(*lclex_hash_function_t)(void *);

typedef bool(*lclex_equal_function_t)(void *, void *);

typedef void(*lclex_free_function_t)(void *);

typedef struct lclex_hashmap_entry_t {
    void *key;
    void *value;
    lclex_hash_t hash;
    struct lclex_hashmap_entry_t *next;
} lclex_hashmap_entry_t;

typedef struct {
    lclex_hash_function_t hash_func;
    lclex_equal_function_t equal_func;
    lclex_free_function_t key_free_func;
    lclex_free_function_t value_free_func;
    size_t cap;
    lclex_hashmap_entry_t **data;
} lclex_hashmap_t;

lclex_hash_t lclex_hash_string(void *data);

bool lclex_equal_string(void *left, void *right);

void lclex_init_string_hashmap(lclex_hashmap_t *map, 
                               lclex_free_function_t value_free_func);

lclex_hashmap_entry_t *lclex_new_hashmap_entry(void *key, void *value, 
                                               lclex_hash_t hash, 
                                               lclex_hashmap_entry_t *next);

void lclex_destruct_hashmap(lclex_hashmap_t *map);

void *lclex_lookup_hashmap(lclex_hashmap_t *map, void *key);

void lclex_insert_hashmap(lclex_hashmap_t *map, void *key, void *value);

#endif
