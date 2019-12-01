#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "data_helper.h"
#include "../helper/wulkanat/descriptive_types.h"

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
    bool act;
    bool get;
    bool set;
    bool delete;
    byte16 key_length;
    byte16 value_length;
    val unknown *key;
    val unknown *value;
} ClientProtocol;

typedef struct {
    bool control;
    bool reply;
    bool lookup;
    byte16 hashId;
    byte16 nodeId;
    byte32 nodeIp;
    byte16 nodePort;
} PeerProtocol;

typedef struct {
    byte16 id;
    byte16 port;
    byte32 ip;
} Peer;

typedef struct {
    Peer this;
    Peer next;
    Peer prev;
} PeerInfo;


/**
 *
 *
 * @param msg the encoded data
 * @return wether it is a PeerProtocol message
 */
bool isPeerProtocol(unknown *msg);

/**
 * Gets all parts from a binary message
 *
 * @param msg the binary message
 * @param data an empty pointer to the decoded data
 */
void decode_peerProtocol(val unknown *msg, PeerProtocol *data);

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
PeerProtocol make_peerProtocol(bool reply, bool lookup, byte16 hashID, Peer peer);

/**
 * Creates a peerProtocol from a clientProtocol
 *
 * @param clientProtocol the protocol to use as a template
 * @return the peerProtocol
 */
PeerProtocol peerProtocol_from_clientProtocol(ClientProtocol *clientProtocol, Peer peer);

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
void decode_clientProtocol(val unknown *msg, ClientProtocol *data);

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

/**
 * Checks if the peer is responsible for the data request
 *
 * @param decodedData the data the peer may be responsible for
 * @param this the peer that may be responsible
 * @param prev the previous peer
 * @return whether the peer is responsible
 */
bool lookup_is_responsible(byte16 hash_id, Peer this, Peer prev);

/**
 * Sends a lookup request for when there was a node found that matches
 *
 * @param
 * @return
 */
void send_found_lookup(PeerProtocol *decodedData, Peer next);
