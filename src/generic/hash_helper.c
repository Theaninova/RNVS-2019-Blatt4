#include <string.h>
#include "hash_helper.h"
#include "../debug.h"

struct HashElement *hash_table_head = NULL; //declare the Hash

void set(const void *key, uint16_t key_len, const void *value, uint32_t value_length) {
    struct HashElement *s = get(key, key_len);

    if (s == NULL) {
        LOG("Element not in database, adding new");
        s = (struct HashElement *) malloc(sizeof(struct HashElement));
        s->key = malloc(key_len);
        memcpy(s->key, key, (size_t) key_len);
        HASH_ADD_KEYPTR(hh, hash_table_head, key, (size_t) key_len, s);
    } else {
        LOG("Replacing existing element");
    }

    s->value_length = value_length;
    s->value = malloc(value_length);
    memcpy(s->value, value, (size_t) value_length);
}

// [GET] value
/*
 * Note
 The middle argument is a pointer to the key. You can’t pass a literal key value to HASH_FIND.
 Instead assign the literal value to a variable, and pass a pointer to the variable.
*/
struct HashElement *get(const void *key, uint16_t key_len) {
    struct HashElement *s;
#ifdef DEBUG
    unsigned int items_count = HASH_COUNT(hash_table_head);
    LOG_INT(items_count);
    HEX_VALUE_LOG(key, key_len)
#endif

    HASH_FIND(hh, hash_table_head, key, key_len, s);  // s: output pointer
#ifdef DEBUG
    if (s == NULL) {
        WARN("Element not found");
    } else {
        LOG("Got one");
    }
#endif
    return s;
}


//use find_Hash to get the pointer for deletion
void delete_element(const void *key, uint16_t key_len) {
    struct HashElement *element = get(key, (size_t) key_len);
    LOG("Deleting element");
    HASH_DEL(hash_table_head, element);   // user: pointer to delete
    free(element);
}
