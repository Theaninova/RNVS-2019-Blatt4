#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "wulkanat/helper/descriptive_types.h"

#define BYTE uint8_t
#define BOOL BYTE

#define ACK_BIT ((uint8_t) (0x01u << 4u))
#define GET_BIT ((uint8_t) (0x01u << 5u))
#define SET_BIT ((uint8_t) (0x01u << 6u))
#define DELETE_BIT ((uint8_t) (0x01u << 7u))

#define CONTROL_BIT (0x01u << 0u)
#define REPLY_BIT (0x01u << 6u)
#define LOOKUP_BIT (0x01u << 7u)

#define MASK &
#define COMBINE +

typedef struct {
    BOOL act;
    BOOL get;
    BOOL set;
    BOOL delete;
    uint16_t key_length;
    uint32_t value_length;
    const void *key;
    const void *value;
} ClientProtocol;

typedef struct {
    BOOL control;
    BOOL reply;
    BOOL lookup;
    uint16_t hashId;
    uint16_t nodeId;
    uint32_t nodeIp;
    uint16_t nodePort;
} PeerProtocol;

typedef struct {
    uint16_t id;
    uint16_t port;
    uint32_t ip;
} Peer;

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
void decode_peerProtocol(const void *msg, PeerProtocol *data);

/**
 * Encodes a DecodedData message again to binary format
 *
 * @param data the DecodedData to encode
 * @return a pointer to the binary data
 */
void *encode_peerProtocol(PeerProtocol *data);

/**
 * Creates a PeerProtocol from the data provided
 *
 * @param control the control bit
 * @param reply the reply bit
 * @param lookup the lookup bit
 * @param hashID the hashID
 * @param peer the peer
 * @return the finished PeerProtocol
 */
PeerProtocol make_peerProtocol(bool control, bool reply, bool lookup, byte16 hashID, Peer peer);

/**
 * Calculates the total size of the data if it is encoded again
 *
 * @param data the decoded data
 * @return the size of [data] if it is encoded again
 */
size_t peerProtocolCalculateSize(PeerProtocol *data);

/**
 * Gets all parts from a binary message
 *
 * @param msg the binary message
 * @param data an empty pointer to the decoded data
 */
void decode_clientProtocol(const void *msg, ClientProtocol *data);

/**
 * Encodes a DecodedData message again to binary format
 *
 * @param data the DecodedData to encode
 * @return a pointer to the binary data
 */
void *encode_clientProtocol(ClientProtocol *data);

/**
 * Calculates the total size of the data if it is encoded again
 *
 * @param data the decoded data
 * @return the size of [data] if it is encoded again
 */
size_t clientProtocolCalculateSize(ClientProtocol *data);
