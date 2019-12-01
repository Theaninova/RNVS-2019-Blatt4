#pragma once

#include <stddef.h>
#include "../helper/wulkanat/descriptive_types.h"

#define STATUS_OK 200
#define STATUS_SOCKET_CLOSED 404

#define NETWORK_RECEIVE_FNPTR(respond) void (*respond)(struct Response* rec, int32 sock_fd)
#define NETWORK_RECEIVE_HANDLER(receive_handler, rec, sock_fd) void receive_handler(struct Response* rec, int32 sock_fd)

#define BUFFER_SIZE 512u

#define send(sock_fd, encoded_data, encoded_data_len) send(sock_fd, encoded_data, encoded_data_len, 0)

typedef struct addrinfo AddressInfo;
typedef struct sockaddr_storage AddressInfoStorage;

struct Response {
    string addr;
    size_t data_length;
    unknown *data;
    int32 status_code;
};

typedef struct Response Response;

/**
 * Sets up a socket as a server
 *
 * @param port the port to wait on
 * @return the socked handle of the connection
 */
int32 setup_as_server(string port);

/**
 * Sets up a socket as a client
 *
 * @param addr the address of the machine to connect to
 * @param port the port of the machine to connect to
 * @return the socket handle of the connection
 */
int32 setup_as_client(string addr, string port);

/**
 * Receives data at a socket
 *
 * @param sock_fd the socket handle of the connection
 * @param callback the function that is being called upon full data retreival
 * @return STATUS_OK, STATUS_INTERNAL_SERVER, STATUS_WAITING
 */
int32 receive(int32 sock_fd, NETWORK_RECEIVE_FNPTR(callback));
