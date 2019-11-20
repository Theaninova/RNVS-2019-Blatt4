#include <sys/socket.h>
#include "generic/network.h"
#include "generic/data_helper.h"
#include "debug.h"
#include "generic/commander.h"

#define GET "GET"
#define SET "SET"
#define DELETE "DELETE"

NETWORK_RECEIVE_HANDLER(receive_handler, rec, _sock_fd) {
    struct DecodedData *decodedData = malloc(sizeof(struct DecodedData));
    decode(rec->data, decodedData);

    if (decodedData->get) {
        fwrite(decodedData->value, decodedData->value_length, 1, stdout);
    }
    if (decodedData->act) {
        LOG("Acknowledged");
    }

    free(decodedData);
}

DEBUGGABLE_MAIN(argc, argv)
    STR_ARG(address, 0, "localhost")
    STR_ARG(port, 1, "4711")
    STR_ARG(method, 2, "GET")
    STR_ARG(key, 3, "key")
    BINARY_ARG(value, value_length, 4)

    struct DecodedData data = {};
    if (strcmp(method, GET) == 0) data.get = GET_BIT;
    else if (strcmp(method, SET) == 0) data.set = SET_BIT;
    else if (strcmp(method, DELETE) == 0) data.delete = DELETE_BIT;

    data.key_length = strlen(key);
    data.key = key;

    if (data.set) {
        data.value_length = value_length;
        data.value = value;
    } else {
        LOG("Setting data to NULL");
        data.value_length = 0u;
        data.value = NULL;
    }

    BYTE *encoded_data = encode(&data);
    size_t encoded_data_len = decodedDataEncodedSize(&data);

    int sock_fd = setup_as_client(address, port);

    LOG("Connection Established");

    send(sock_fd, encoded_data, encoded_data_len);

    LOG("Sent data");

    if (receive(sock_fd, receive_handler) == STATUS_OK) {
        LOG("Status OK");
    } else {
        ERROR("Error while receiving");
        THROW(-1)
    }
}