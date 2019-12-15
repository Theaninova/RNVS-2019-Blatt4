#pragma once

#include <stddef.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "../helper/wulkanat/descriptive_types.h"
#include "data_helper.h"

#define STATUS_OK 200
#define STATUS_SOCKET_CLOSED 404

#define NETWORK_RECEIVE_FNPTR(respond) void (*respond)(struct Response* rec, int32 sock_fd)
#define NETWORK_RECEIVE_HANDLER(receive_handler, rec, sock_fd) void receive_handler(struct Response* rec, int32 sock_fd)

#define BUFFER_SIZE 65535

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
char name[INET_ADDRSTRLEN]; \
inet_ntop(AF_INET, &addr, name, INET_ADDRSTRLEN);

#define int_port_to_str(name, port) \
char name[5]; \
sprintf(name, "%d", port);

#define str_port_to_int(name, port) \
byte16 name = atoi(port);

#define str_addr_to_int(name, addr) \
int32 name = 0; \
inet_pton(AF_INET, addr, &name);

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
 * Gets a new connection
 *
 * @param sock_fd the connection on which to listen
 * @return the new connection
 */
int32 get_new_connection(int32 sock_fd);

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

/**
 * Joining new arrived node
 *
 * @param address of a known node inside the network
 * @param port of a known node inside the network
 * @param new node's network information
 * @return void
 */
void join(string joinAddrInt, string joinPortInt, Peer join);

/**
 * Notifies to another node about network changes via peer protocol
 *
 * @param address of the node to inform
 * @param port of the node to inform
 * @param network information of the notifying node
 * @return void
 */
void notify(byte32 target_node_IP, byte16 target_node_Port, Peer next_peer_info);

/**
 * Stabilizes the network to meet the general chord rules after changes
 *
 * @param address of the next node
 * @param port of the next node
 * @param network information of the predecessor
 * @return void
 */
void stabilize(byte32 next_node_IP, byte16 next_node_Port, Peer this_peer_info);

/**
 * Build up finger table, function sends message to all peers to get a feedback of all alive. Afterwards the answers
 * of the ids meeting the chord rules are saved to the finger table
 *
 * @param Peer info of the current block that requested the finger table to be build
 * @return void
 */
void buildfinger(byte32 next_node_IP, byte16 next_node_Port, Peer this_peer_info);