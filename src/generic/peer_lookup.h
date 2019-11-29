#pragma once

#include "data_helper.h"

typedef struct {
    uint16_t id;
    uint16_t port;
    uint32_t ip;
} Peer;

/**
 * Checks if the peer is responsible for the data request
 *
 * @param decodedData the data the peer may be responsible for
 * @param this the peer that may be responsible
 * @param prev the previous peer
 * @return whether the peer is responsible
 */
BOOL lookup_is_responsible(uint16_t hash_id, Peer this, Peer prev);
