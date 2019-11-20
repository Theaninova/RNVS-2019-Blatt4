#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define BYTE uint8_t
#define BOOL BYTE

#define ACK_BIT (uint8_t) (0x01u << 4)
#define GET_BIT (uint8_t) (0x01u << 5)
#define SET_BIT (uint8_t) (0x01u << 6)
#define DELETE_BIT (uint8_t) (0x01u << 7)

#define CONTROL_BIT (0x01u << 0)
#define REPLY_BIT (0x01u << 6)
#define LOOKUP_BIT (0x01u << 7)

#define MASK &
#define COMBINE +

struct ClientProtocol {
    BOOL act;
    BOOL get;
    BOOL set;
    BOOL delete;
    uint16_t key_length;
    uint32_t value_length;
    const void *key;
    const void *value;
};

typedef struct {
    BOOL control;
    BOOL reply;
    BOOL lookup;
    uint16_t hashId;
    uint16_t nodeId;
    uint32_t nodeIp;
    uint16_t nodePort;
} PeerProtocol;

/**
 *
 *
 * @param msg the encoded data
 * @return wether it is a PeerProtocol message
 */
BOOL isPeerProtocol(void *msg);

/**
 * Gets all parts from a binary message
 *
 * @param msg the binary message
 * @param data an empty pointer to the decoded data
 */
void decode_peerProtocol(const void *msg, struct ClientProtocol *data);

/**
 * Encodes a DecodedData message again to binary format
 *
 * @param data the DecodedData to encode
 * @return a pointer to the binary data
 */
void *encode_peerProtocol(struct ClientProtocol *data);

/**
 * Calculates the total size of the data if it is encoded again
 *
 * @param data the decoded data
 * @return the size of [data] if it is encoded again
 */
size_t peerProtocolCalculateSize(struct ClientProtocol *data);

/**
 * Gets all parts from a binary message
 *
 * @param msg the binary message
 * @param data an empty pointer to the decoded data
 */
void decode_clientProtocol(const void *msg, struct ClientProtocol *data);

/**
 * Encodes a DecodedData message again to binary format
 *
 * @param data the DecodedData to encode
 * @return a pointer to the binary data
 */
void *encode_clientProtocol(struct ClientProtocol *data);

/**
 * Calculates the total size of the data if it is encoded again
 *
 * @param data the decoded data
 * @return the size of [data] if it is encoded again
 */
size_t clientProtocolCalculateSize(struct ClientProtocol *data);
