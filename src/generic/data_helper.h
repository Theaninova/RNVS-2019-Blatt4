#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "data_helper.h"
#include "../helper/wulkanat/descriptive_types.h"
#include <math.h>

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
    byte32 value_length;
    val unknown *key;
    val unknown *value;
} ClientProtocol;

typedef struct {
    bool control;
    bool finger;
    bool fack;
    bool join;
    bool notify;
    bool stabilize;
    bool reply;
    bool lookup;
    byte16 hashId;
    byte16 nodeId;
    byte32 nodeIp;
    byte16 nodePort;
} PeerProtocol;

 struct peer{
    byte16          ip;
    byte16          port;
    byte32          id;
    bool            is_base;
    struct peer*    next_finger;
};
typedef struct peer Peer;

typedef struct {
    Peer    this;
    Peer    next;
    Peer    prev;
    Peer    join;
} PeerInfo;

typedef struct {
    Peer*   fingers;
    Peer*   last_in_list;
    byte16  count;
}raw_fingertable;


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
bool id_is_between(byte16 hash_id, Peer this, Peer prev);

/**
 * Sends a lookup request for when there was a node found that matches
 *
 * @param
 * @return
 */
void send_found_lookup(PeerProtocol *decodedData, Peer next);

/**
 * Recursively lookup the first not used entry in the raw finger table
 *
 * @param   a next peer
 * @param   a raw finger table for count increment
 * @return  a finger table entry not used yet
 */
Peer* find_last_entry(Peer* next, raw_fingertable* raw_fingers);

/**
 * Check raw finger table to meed the chord rules -> these are copied to the Peer's "real" finger table
 *
 * @param   the raw finger table of all received nodes
 * @param   current peer that was requested to build a peer table
 * @return  void
 */
void check_chord_rules(raw_fingertable* raw_fingers, PeerInfo* current);