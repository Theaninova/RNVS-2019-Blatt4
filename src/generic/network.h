#pragma once

#include <stddef.h>
#include <stdio.h>
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

#define int_addr_to_str(name, addr) \
char name[15]; \
sprintf(name, "%d.%d.%d.%d", as(byte8, &addr), as(byte8, &addr + 1), as(byte8, &addr + 2), as(byte8, &addr + 3));

#define int_port_to_str(name, port) \
char name[5]; \
sprintf(name, "%d", port);

#define str_port_to_int(name, port) \
int32 name = atoi(port);

#define str_addr_to_int(name, addr) \
int32 name = 0; \
do {\
    size_t total_length = strlen(addr); \
    int32 cursor = 0; \
    for (int i = 0; i < 4; i++) { \
        char buffer[3]; \
        for (int j = 0; cursor < total_length && addr[cursor] != '.'; j++) { \
            buffer[j] = addr[cursor]; \
            cursor++; \
        } \
        cursor++; \
        as(byte8, &name + i) = (byte8) atoi(buffer); \
    } \
} while (0);

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

/**
 * Redirects a request to another socket
 *
 * @param sock_from the socket from which the request comes
 * @param sock_to the socket to which the request goes
 * @return STATUS_OK, STATUS_INTERNAL_SERVER, STATUS_WAITING
 */
int32 redirect(int32 sock_from, int32 sock_to);

/**
 * Directly sends data to an address
 *
 * @param addr the address to send to
 * @param port the port to send to
 * @param data the data to send
 * @param data_size the size of the data
 * @return the socket of the connection
 */
int32 direct_send(string addr, string port, unknown *data, size_t data_size);
