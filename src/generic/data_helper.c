#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include "data_helper.h"
#include "../debug.h"

void decode_clientProtocol(const void *msg, struct ClientProtocol *data) {
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

size_t clientProtocolCalculateSize(struct ClientProtocol *data) {
    return sizeof(BYTE) + sizeof(data->key_length) + sizeof(data->value_length) + data->key_length + data->value_length;
}

void *encode_clientProtocol(struct ClientProtocol *data) {
    LOG("Encoding...");

    BYTE *msg = malloc(clientProtocolCalculateSize(data));

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
