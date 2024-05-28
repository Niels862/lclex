#include "hashmap.h"
#include <stdlib.h>
#include <string.h>

lclex_hash_t lclex_hash_string(void *data) {
    lclex_hash_t hash = 42;

    return hash;
}

bool lclex_equal_string(void *left, void *right) {
    return strcmp(left, right) == 0;
}

void lclex_init_string_hashmap(lclex_hashmap_t *map) {
    map->hash_func = lclex_hash_string;
    map->equal_func = lclex_equal_string;
    map->cap = LCLEX_HASHMAP_INIT_SIZE;
    map->data = calloc(LCLEX_HASHMAP_INIT_SIZE, 
                       sizeof(lclex_hashmap_entry_t *));
}

lclex_hashmap_entry_t *lclex_new_hashmap_entry(void *key, void *value, 
                                               lclex_hash_t hash, 
                                               lclex_hashmap_entry_t *next) {
    lclex_hashmap_entry_t *entry = malloc(sizeof(lclex_hashmap_entry_t));

    entry->key = key;
    entry->value = value;
    entry->hash = hash;
    entry->next = next;

    return entry;
}

void *lclex_lookup_hashmap(lclex_hashmap_t *map, void *key) {
    lclex_hash_t hash = map->hash_func(key);
    size_t idx = hash % map->cap;

    lclex_hashmap_entry_t *entry = map->data[idx];
    void *value = NULL;

    while (entry != NULL && value == NULL) {
        if (entry->hash == hash && map->equal_func(entry->key, key)) {
            value = entry->value;
        } else {
            entry = entry->next;
        }
    }

    return value;
}

void lclex_insert_hashmap(lclex_hashmap_t *map, void *key, void *value) {
    lclex_hash_t hash = map->hash_func(key);
    size_t idx = hash % map->cap;

    map->data[idx] = lclex_new_hashmap_entry(key, value, hash, map->data[idx]);
}
