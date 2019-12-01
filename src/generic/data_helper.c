#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include "data_helper.h"
#include "../debug.h"
#include "peer_lookup.h"
#include "../helper/Max/extended_math.h"
#include "network.h"

// by using the packed attribute the compiler will not insert any data between or around the struct when in memory for
// optimization. That means that we can layout the data structure that we have for the network protocol in a struct
// and then just access the individual attributes and automatically get the correct positions
typedef struct __attribute__((packed)) {
    BYTE header;
    uint16_t hashID;
    uint16_t nodeID;
    uint32_t nodeIP;
    uint16_t nodePort;
} RawPeerProtocol;

void decode_peerProtocol(const void *msg, PeerProtocol *data) {
    RawPeerProtocol *raw = (RawPeerProtocol *) msg;

    data->control = raw->header MASK CONTROL_BIT;
    data->reply = raw->header MASK REPLY_BIT;
    data->lookup = raw->header MASK LOOKUP_BIT;

    data->hashId = raw->hashID;
    data->nodeId = raw->nodeID;
    data->nodeIp = raw->nodeIP;
    data->nodePort = raw->nodePort;
}

size_t peerProtocolCalculateSize(PeerProtocol *data) {
    return sizeof(RawPeerProtocol);
}

void *encode_peerProtocol(PeerProtocol *data) {
    RawPeerProtocol *msg = calloc(peerProtocolCalculateSize(data), 1);

    msg->header = data->control COMBINE data->reply COMBINE data->lookup;
    msg->hashID = data->hashId;
    msg->nodeID = data->nodeId;
    msg->nodeIP = data->nodeIp;
    msg->nodePort = data->nodePort;

    return msg;
}

BOOL isPeerProtocol(void *msg) {
    return *((BYTE *) msg) MASK CONTROL_BIT;
}

void decode_clientProtocol(const void *msg, ClientProtocol *data) {
    LOG("Decoding...");

    data->act = *((BYTE *) msg) MASK ACK_BIT;
    data->get = *((BYTE *) msg) MASK GET_BIT;
    data->set = *((BYTE *) msg) MASK SET_BIT;
    data->delete = *((BYTE *) msg) MASK DELETE_BIT;

    data->key_length = ntohs(*((uint16_t *) (msg + sizeof(BYTE))));
    data->value_length = ntohl(*((uint32_t *) (msg + sizeof(BYTE) + sizeof(data->key_length))));

    if (data->key_length == 0) {
        data->key = NULL;
    } else {
        data->key = msg + sizeof(BYTE) + sizeof(data->key_length) + sizeof(data->value_length);
    }
    if (data->value_length == 0) {
        data->value = NULL;
    } else {
        data->value = msg + sizeof(BYTE) + sizeof(data->key_length) + sizeof(data->value_length) + data->key_length;
    }

    LOG("Finished decoding");
}

size_t clientProtocolCalculateSize(ClientProtocol *data) {
    return sizeof(BYTE) + sizeof(data->key_length) + sizeof(data->value_length) + data->key_length + data->value_length;
}

void *encode_clientProtocol(ClientProtocol *data) {
    LOG("Encoding...");

    BYTE *msg = calloc(clientProtocolCalculateSize(data), 1);

    *msg = data->act COMBINE data->get COMBINE data->set COMBINE data->delete;

    *((uint16_t *) (msg + sizeof(BYTE))) = htons(data->key_length);
    *((uint32_t *) (msg + sizeof(BYTE) + sizeof(data->key_length))) = htonl(data->value_length);

    // we can do this because memcpy will just not do anything if either length is 0
    memcpy(msg + sizeof(BYTE) + sizeof(data->key_length) + sizeof(data->value_length), data->key,
           (size_t) data->key_length);
    memcpy(msg + sizeof(BYTE) + sizeof(data->key_length) + sizeof(data->value_length) + data->key_length,
           data->value, (size_t) data->value_length);

    LOG("Finished encoding");
    return msg;
}

PeerProtocol make_peerProtocol(bool control, bool reply, bool lookup, byte16 hashID, Peer peer) {
    PeerProtocol result;

    result.control = control ? CONTROL_BIT : 0;
    result.reply   = reply   ? REPLY_BIT   : 0;
    result.lookup  = lookup  ? LOOKUP_BIT  : 0;

    result.hashId = hashID;

    result.nodeId = peer.id;
    result.nodeIp = peer.ip;
    result.nodePort = peer.port;

    return result;
}

bool lookup_is_responsible(uint16_t hash_id, Peer this, Peer prev){
    return is_in_range((uint16) hash_id, (uint16) this.id, (uint16) prev.id);
}

void send_found_lookup(PeerProtocol *Data, Peer next){
    Data->reply     =   REPLY_BIT_BIT;
    Data->lookup    =   LOOKUP_BIT;
    Data->nodePort  =   next.port;
    send(setup_as_client(Data->nodeIp, Data->nodePort), Data, sizeof(Data));
}

void send_lookup_request(PeerProtocol decodedData, Peer next){

}
