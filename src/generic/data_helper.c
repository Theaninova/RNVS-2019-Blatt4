#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include "data_helper.h"
#include "../helper/wulkanat/debug.h"
#include "../helper/max/extended_math.h"
#include "network.h"
#include "hash_helper.h"

// by using the packed attribute the compiler will not insert any data between or around the struct when in memory for
// optimization. That means that we can layout the data structure that we have for the network protocol in a struct
// and then just access the individual attributes and automatically get the correct positions
typedef struct __attribute__((packed)) {
    byte8 header;
    byte16 hashID;
    byte16 nodeID;
    byte32 nodeIP;
    byte16 nodePort;
} RawPeerProtocol;

void decode_peerProtocol(val unknown *msg, PeerProtocol *data) {
    RawPeerProtocol *raw = (RawPeerProtocol *) msg;

    data->control = raw->header MASK CONTROL_BIT;
    data->reply = raw->header MASK REPLY_BIT;
    data->lookup = raw->header MASK LOOKUP_BIT;

    data->hashId = ntohs(raw->hashID);
    data->nodeId = ntohs(raw->nodeID);
    data->nodeIp = ntohl(raw->nodeIP); // TODO: is that right?
    data->nodePort = /*ntohs(*/raw->nodePort/*)*/;
}

size_t peerProtocolCalculateSize(PeerProtocol *data) {
    return sizeof(RawPeerProtocol);
}

void *encode_peerProtocol(PeerProtocol *data) {
    RawPeerProtocol *msg = calloc(peerProtocolCalculateSize(data), 1);

    msg->header = data->control COMBINE data->reply COMBINE data->lookup;
    msg->hashID = htons(data->hashId);
    msg->nodeID = htons(data->nodeId);
    msg->nodeIP = htonl(data->nodeIp);
    msg->nodePort = htons(data->nodePort);

    return msg;
}

bool isPeerProtocol(void *msg) {
    return truthy(as(byte8, msg) MASK CONTROL_BIT);
}

void decode_clientProtocol(const void *msg, ClientProtocol *data) {
    LOG("Decoding...");

    data->act = *((byte8 *) msg) MASK ACK_BIT;
    data->get = *((byte8 *) msg) MASK GET_BIT;
    data->set = *((byte8 *) msg) MASK SET_BIT;
    data->delete = *((byte8 *) msg) MASK DELETE_BIT;

    data->key_length = ntohs(*((uint16_t *) (msg + sizeof(byte8))));
    data->value_length = ntohl(*((uint32_t *) (msg + sizeof(byte8) + sizeof(data->key_length))));

    if (data->key_length == 0) {
        data->key = NULL;
    } else {
        data->key = msg + sizeof(byte8) + sizeof(data->key_length) + sizeof(data->value_length);
    }
    if (data->value_length == 0) {
        data->value = NULL;
    } else {
        data->value = msg + sizeof(byte8) + sizeof(data->key_length) + sizeof(data->value_length) + data->key_length;
    }

    LOG("Finished decoding");
}

size_t clientProtocolCalculateSize(ClientProtocol *data) {
    return sizeof(byte8) + sizeof(data->key_length) + sizeof(data->value_length) + data->key_length + data->value_length;
}

void *encode_clientProtocol(ClientProtocol *data) {
    LOG("Encoding...");

    unknown *msg = calloc(clientProtocolCalculateSize(data), 1);

    as(byte8, msg) = data->act COMBINE data->get COMBINE data->set COMBINE data->delete;

    as(byte16, msg + sizeof(byte8)) = htons(data->key_length);
    as(byte32, msg + sizeof(byte8) + sizeof(data->key_length)) = htonl(data->value_length);

    // we can do this because memcpy will just not do anything if either length is 0
    memcpy(msg + sizeof(byte8) + sizeof(data->key_length) + sizeof(data->value_length), data->key,
           (size_t) data->key_length);
    memcpy(msg + sizeof(byte8) + sizeof(data->key_length) + sizeof(data->value_length) + data->key_length,
           data->value, (size_t) data->value_length);

    LOG("Finished encoding");
    return msg;
}

PeerProtocol make_peerProtocol(bool reply, bool lookup, byte16 hashID, Peer peer) {
    PeerProtocol result;

    result.control = CONTROL_BIT;
    result.reply   = reply   ? REPLY_BIT   : 0;
    result.lookup  = lookup  ? LOOKUP_BIT  : 0;

    result.hashId = hashID;

    result.nodeId = peer.id;
    result.nodeIp = peer.ip;
    result.nodePort = peer.port;

    return result;
}

bool lookup_is_responsible(byte16 hash_id, Peer this, Peer prev){
    return in_range((byte16) hash_id, (byte16) this.id, (byte16) prev.id);
}

PeerProtocol peerProtocol_from_clientProtocol(ClientProtocol *clientProtocol, Peer peer) {
    PeerProtocol result = make_peerProtocol(false, true,
            get_hash_value(clientProtocol->key, clientProtocol->key_length),
            peer);

    return result;
}

void send_found_lookup(PeerProtocol *data, Peer next){
    data->control   =   CONTROL_BIT;
    data->reply     =   REPLY_BIT;
    data->lookup    =   0u;
    data->nodePort  =   next.port;
    data->nodeIp    =   next.ip;
    data->nodeId    =   next.id;

    int_addr_to_str(nodeIp, data->nodeIp)
    int_port_to_str(nodePort, data->nodePort)

    unknown *encodedData = encode_peerProtocol(data);

    direct_send(nodeIp, nodePort, encodedData, peerProtocolCalculateSize(data));
}
