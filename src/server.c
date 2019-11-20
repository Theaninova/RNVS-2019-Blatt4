#include <sys/socket.h>
#include <netinet/in.h>
#include "generic/network.h"
#include "generic/data_helper.h"
#include "generic/hash_helper.h"
#include "debug.h"
#include "generic/commander.h"

NETWORK_RECEIVE_HANDLER(receive_handler, rec, sock_fd) {
    LOG("Parsing request");

    struct ClientProtocol decodedData = {};
    decode_clientProtocol(rec->data, &decodedData);

    if (decodedData.get) {
        LOG("[GET]");
        HEX_VALUE_LOG(decodedData.key, decodedData.key_length);
        struct HashElement *element = get(decodedData.key, decodedData.key_length);
        if (element == NULL) {
            LOG("Empty value (not found)");
            BYTE res[sizeof(BYTE) + sizeof(decodedData.key_length) + sizeof(decodedData.value_length)];
            *((uint16_t *) (res + sizeof(BYTE))) = 0u;
            *((uint32_t *) (res + sizeof(BYTE) + sizeof(uint16_t))) = 0u;
            *res = ACK_BIT COMBINE GET_BIT;
            LOG("Acknowledged");
            send(sock_fd, &res, sizeof(BYTE) + sizeof(decodedData.key_length) + sizeof(decodedData.value_length));
            return;
        }
        decodedData.value = element->value;
        decodedData.value_length = element->value_length;

        decodedData.act = ACK_BIT;
        size_t length = clientProtocolCalculateSize(&decodedData);
        LOG("Acknowledged");
        send(sock_fd, encode_clientProtocol(&decodedData), length);
    }
    if (decodedData.set) {
        LOG("[SET]");
        set((void *) decodedData.key, decodedData.key_length, decodedData.value, decodedData.value_length);
        HEX_VALUE_LOG(decodedData.value, decodedData.value_length);
        BYTE res[sizeof(BYTE) + sizeof(decodedData.key_length) + sizeof(decodedData.value_length)];
        *((uint16_t *) (res + sizeof(BYTE))) = 0u;
        *((uint32_t *) (res + sizeof(BYTE) + sizeof(uint16_t))) = 0u;
        *res = ACK_BIT COMBINE SET_BIT;
        LOG("Acknowledged");
        send(sock_fd, &res, sizeof(BYTE) + sizeof(decodedData.key_length) + sizeof(decodedData.value_length));
    }
    if (decodedData.delete) {
        LOG("[DELETE]");
        delete_element((void *) decodedData.key, decodedData.key_length);
        BYTE res[sizeof(BYTE) + sizeof(decodedData.key_length) + sizeof(decodedData.value_length)];
        *((uint16_t *) (res + sizeof(BYTE))) = 0u;
        *((uint32_t *) (res + sizeof(BYTE) + sizeof(uint16_t))) = 0u;
        *res = ACK_BIT COMBINE DELETE_BIT;
        LOG("Acknowledged");
        send(sock_fd, &res, sizeof(BYTE) + sizeof(decodedData.key_length) + sizeof(decodedData.value_length));
    }
}

DEBUGGABLE_MAIN(argc, argv)
    STR_ARG(port, 0, "4711")

    LOG("Starting");
    int sock_fd = setup_as_server(port);

    while (1) {
        int code = receive(sock_fd, receive_handler);
        if (code == STATUS_OK) {
            LOG("Status OK");
        } else if (code == STATUS_SOCKET_CLOSED) {
            LOG("Ending Program");
            return 0;
        } else {
            ERROR("Error while connecting");
            THROW(-1)
        }
    }
}
