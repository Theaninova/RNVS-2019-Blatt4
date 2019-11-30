#pragma once

#include <string.h>
#include "../libraries/uthash.h"
#include "wulkanat/helper/descriptive_types.h"

struct HashElement {
    void *key;
    void *value;
    uint32_t value_length;
    UT_hash_handle hh;
};

/**
 * Set an element in the Hash table
 *
 * Does not create duplicate elements for same keys
 *
 * @param key the key of the element
 * @param key_len the length of the key
 * @param value the value of the element
 * @param value_length the length of the value
 */
void set(const void *key, uint16_t key_len, const void *value, uint32_t value_length);

/**
 * Retreives an element from the hash table
 *
 * @param key the key of the element
 * @param key_len the length of the key
 * @return a pointer to the element
 */
struct HashElement* get(const void *key, uint16_t key_len);

/**
 * Deletes an element from the hash table
 *
 * Can be safely called without the element existing
 *
 * @param key the key of the element
 * @param key_len the length of the key
 */
void delete_element(const void *key, uint16_t key_len);


/**
 * Gets the hash value of the key
 *
 * @param key the key to hash
 * @param key_len the length of the key
 * @return the hash of the key
 */
byte32 get_hash_value(val unknown *key, byte16 key_len);