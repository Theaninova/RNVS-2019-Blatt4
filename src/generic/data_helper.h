#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define BYTE uint8_t
#define BOOL BYTE

#define ACK_BIT (uint8_t) 0x10u
#define GET_BIT (uint8_t) 0x20u
#define SET_BIT (uint8_t) 0x40u
#define DELETE_BIT (uint8_t) 0x80u

#define MASK &
#define COMBINE +

struct DecodedData {
    BOOL act;
    BOOL get;
    BOOL set;
    BOOL delete;
    uint16_t key_length;
    uint32_t value_length;
    const void *key;
    const void *value;
};

/**
 * Gets all parts from a binary message
 *
 * @param msg the binary message
 * @param data an empty pointer to the decoded data
 */
void decode(const void *msg, struct DecodedData *data);

/**
 * Encodes a DecodedData message again to binary format
 *
 * @param data the DecodedData to encode
 * @return a pointer to the binary data
 */
void *encode(struct DecodedData *data);

/**
 * Calculates the total size of the data if it is encoded again
 *
 * @param data the decoded data
 * @return the size of [data] if it is encoded again
 */
size_t decodedDataEncodedSize(struct DecodedData *data);
