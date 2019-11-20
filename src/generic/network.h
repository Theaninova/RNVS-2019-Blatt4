#pragma once

#include <stddef.h>

#define STATUS_OK 200
#define STATUS_SOCKET_CLOSED 404

#define NETWORK_RECEIVE_FNPTR(respond) void (*respond)(struct Response* rec, int sock_fd)
#define NETWORK_RECEIVE_HANDLER(receive_handler, rec, sock_fd) void receive_handler(struct Response* rec, int sock_fd)

#define BUFFER_SIZE 512u

#define send(sock_fd, encoded_data, encoded_data_len) send(sock_fd, encoded_data, encoded_data_len, 0)

struct Response {
    char *addr;
    size_t data_length;
    void *data;
    int status_code;
};

/**
 * Sets up a socket as a server
 *
 * @param port the port to wait on
 * @return the socked handle of the connection
 */
int setup_as_server(char *port);

/**
 * Sets up a socket as a client
 *
 * @param addr the address of the machine to connect to
 * @param port the port of the machine to connect to
 * @return the socket handle of the connection
 */
int setup_as_client(char *addr, char *port);

/**
 * Receives data at a socket
 *
 * @param sock_fd the socket handle of the connection
 * @param callback the function that is being called upon full data retreival
 * @return STATUS_OK, STATUS_INTERNAL_SERVER, STATUS_WAITING
 */
int receive(int sock_fd, NETWORK_RECEIVE_FNPTR(callback));
