#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include "network.h"
#include "../helper/wulkanat/debug.h"
#include "data_helper.h"
#include <assert.h>

#define BACKLOG 10     // how many pending connections queue will hold

void sigchld_handler(int32 s) {
    (void) s; // quiet unused variable warning

    // waitpid() might overwrite errno, so we save and restore it:
    int32 saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

int32 setup_as_client(string addr, string port) {
    int32 sock_fd = -1;
    int32 rv;
    AddressInfo hints;
    AddressInfo *server_info;
    AddressInfo *p;
    char s[INET6_ADDRSTRLEN];

    LOG_STR(addr);
    LOG_STR(port);

    //define hints
    memset(&hints, 0, sizeof hints); //empty hint
    hints.ai_family = AF_UNSPEC; //IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; //TCP Protocol

    if ((rv = getaddrinfo(addr, port, &hints, &server_info)) != 0) {
        ERROR("Malformed address or port");
        return -1;
    }

    // loop through all the results and connect to the first we can
    for(p = server_info; p != NULL; p = p->ai_next) {
        if ((sock_fd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            WARN("(CONNECTIONS) Socket is -1");
            continue;
        }

        if (connect(sock_fd, p->ai_addr, p->ai_addrlen) == -1) {
            WARN("(CONNECTIONS) Connection Failed to connect");
            close(sock_fd);
            continue;
        }

        break;
    }

    if (p == NULL) {
        ERROR("Failed to connect");
        return -1;
    }
    LOG("Connecting...");

    inet_ntop(p->ai_family, get_in_addr(p->ai_addr),
              s, sizeof s);

    freeaddrinfo(server_info);

    return sock_fd;
}

int32 setup_as_server(string port) {
    int32 sock_fd;  // listen on sock_fd, new connection on new_fd
    AddressInfo hints;
    AddressInfo *server_info;
    AddressInfo *p;
    struct sigaction sa;
    val int32 yes = 1;

    //check Port
    if (atoi(port) < 0 || atoi(port) > 65535) {
        ERROR("Invalid Port");
        return -1;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    int32 rv;
    if ((rv = getaddrinfo(NULL, port, &hints, &server_info)) != 0) {
        ERROR("Could not get Address");
        return -1;
    }

    // loop through all the results and bind to the first we can
    for (p = server_info; p != NULL; p = p->ai_next) {
        if ((sock_fd = socket(p->ai_family, p->ai_socktype,
                              p->ai_protocol)) == -1) {
            WARN("(CONNECTIONS) Connection Failed to connect");
            continue;
        }

        if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            WARN("(CONNECTIONS) Connection Failed to connect");
            return -1;
        }

        if (bind(sock_fd, p->ai_addr, p->ai_addrlen) == -1) {
            WARN("(CONNECTIONS) Failed to bind");

            close(sock_fd);
            continue;
        }

        break;
    }

    freeaddrinfo(server_info); // all done with this structure

    if (p == NULL) {
        ERROR("Failed to bind on Port");
        return -1;
    }

    if (listen(sock_fd, BACKLOG) == -1) {
        ERROR("Listen");
        return -1;
    }
    LOG("Listen");

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        ERROR("Sigaction");
        return -1;
    }

    return sock_fd;
}

int32 get_new_connection(int32 sock_fd) {
    AddressInfoStorage their_addr;
    string s;

    size_t sin_size = sizeof(their_addr);

    int32 new_fd;

    LOG("Waiting for Connection");
    while ((new_fd = accept(sock_fd, (struct sockaddr *) &their_addr, (socklen_t*) &sin_size)) == -1) {/*noop*/}

    inet_ntop(their_addr.ss_family,
              get_in_addr((struct sockaddr *) &their_addr),
              s, sizeof s);
    LOG("Got Connection");
    close(sock_fd);

    return new_fd;
}


Response direct_receive(int32 sock_fd, bool status) {                                              // not yet testet
    Response response;
    response.data = malloc(BUFFER_SIZE * sizeof(uint8_t));
    response.data_length = (recv(sock_fd, (uint8_t *) response.data, BUFFER_SIZE, 0));

    if(response.data_length < BUFFER_SIZE){ // one call or last call
        response.data = realloc((uint8_t *) response.data, response.data_length);
    }
    else if(response.data_length == BUFFER_SIZE) {  // Buffer full -> next recur.
        Response conc = direct_receive(sock_fd, 0);
        uint8_t *data_conc = realloc(response.data, conc.data_length + response.data_length);
        memcpy(data_conc, response.data, response.data_length);    // <...block(n-1)...|...empty...|>
        memcpy((data_conc + response.data_length), conc.data, conc.data_length);   // <...block(n-1)...|...block(n)...|>
        response.data_length += conc.data_length;
        if(status == 0) free(conc.data);
    }
    else{   // no more data after Buffer full or didn't receive data
        ERROR("Socket closed / Receive complete");
        free(response.data);
    }

    if(status == 1){
    #ifdef DEBUG
            printf("[LOG]:   Received chunk with %zu bytes\n", response.data_length);
    #endif
            response.status_code = STATUS_OK;
            LOG_INT((int32) response.data_length);
    }

    return response;
}

int32 receive(int32 sock_fd, NETWORK_RECEIVE_FNPTR(callback)) {
    Response response = direct_receive(sock_fd, 1);

    callback(&response, sock_fd);

    return response.status_code;
}

int32 redirect(int32 sock_from, int32 sock_to) {
    Response response = direct_receive(sock_from, 1);

    send(sock_to, response.data, response.data_length);

    return response.status_code;
}

int32 direct_send(string addr, string port, unknown *data, size_t data_size) {
    int32 sock_fd = setup_as_client(addr, port);
    send(sock_fd, data, data_size);

    return sock_fd;
}

void join(string joinAddrInt, string joinPortInt, Peer new_peer_info) {
    PeerProtocol *join_request;
    join_request->join       =   1;
    join_request->control    =   1;
    join_request->nodeId     =   new_peer_info.id;
    join_request->nodeIp     =   new_peer_info.ip;
    join_request->nodePort   =   new_peer_info.port;
    direct_send(joinAddrInt, joinPortInt, (PeerProtocol*) join_request, sizeof(PeerProtocol));  // correct size?
    if(sizeof(PeerProtocol) == sizeof(join_request) == NULL) LOG("size ok, join, Network.c");
    new_peer_info.next_finger = calloc(1, sizeof(Peer));                      // TODO soll der nächste Nachbar hier schon in die Liste eingefügt werden?
}

void notify(byte32 target_node_IP, byte16 target_node_Port, Peer next_peer_info){
    PeerProtocol *notify_request;
    notify_request->control     =   1;
    notify_request->notify      =   1;
    notify_request->nodeId      =   next_peer_info.id;
    notify_request->nodeIp      =   next_peer_info.ip;
    notify_request->nodePort    =   next_peer_info.port;
    int_addr_to_str(notifyaddr_str, target_node_IP)
    int_port_to_str(notifyport_str, target_node_Port)
    direct_send(notifyaddr_str, notifyport_str, (PeerProtocol*) notify_request, sizeof(PeerProtocol));  // correct size?
    if(sizeof(PeerProtocol) == sizeof(notify_request) == NULL) LOG("size ok, nofity, Network.c");
}

void stabilize(byte32 next_node_IP, byte16 next_node_Port, Peer this_peer_info){
    PeerProtocol *stabilize_request;
    stabilize_request->stabilize    =   1;
    stabilize_request->control      =   1;
    stabilize_request->nodeId       =   this_peer_info.id;
    stabilize_request->nodeIp       =   this_peer_info.ip;
    stabilize_request->nodePort     =   this_peer_info.port;
    int_addr_to_str(nextaddr_str, next_node_IP)
    int_port_to_str(nextport_str, next_node_Port)
    direct_send(nextaddr_str, nextport_str, (PeerProtocol*) stabilize_request, sizeof(PeerProtocol));  // correct size?
    if(sizeof(PeerProtocol) == sizeof(stabilize_request) == NULL) LOG("size ok, stabilize, Network.c");
}


void buildfinger(byte32 next_node_IP, byte16 next_node_Port, Peer this_peer_info){
    PeerProtocol *finger_request;
    finger_request->lookup  =   1;
    finger_request->control =   1;
    finger_request->finger  =   1;                          // sollen die anderen Nodes dann auch direkt ihre Fingertable aufbauen oder bezieht sich das auf jeden Node?
    finger_request->nodeIp  =   this_peer_info.ip;
    finger_request->nodePort=   this_peer_info.port;

    Peer* tmp = &this_peer_info.next_finger;
    //while(tmp->next_finger != NULL) {
    for(byte16 i = 0; i < 16; i++){                         // numbers can be 16 bits
        if(this_peer_info.next_finger != NULL) {
            finger_request->nodeId  =  this_peer_info.id + ( (2 << i) % 65536);
            int_addr_to_str(nextaddr_str, next_node_IP)
            int_port_to_str(nextport_str, next_node_Port)
            direct_send(nextaddr_str, nextport_str, (PeerProtocol *) finger_request, sizeof(PeerProtocol));
        }
    }
}






/*
Response direct_receive_(int32 sock_fd) {                                               // not used (to be removed)
    Response response = {};

    char buffer[BUFFER_SIZE];
    // int number_of_bytes;
    size_t total_number_of_bytes = recv(sock_fd, buffer, BUFFER_SIZE, 0);
    response.data = NULL;

    if (total_number_of_bytes == 0)
        ERROR("Socket is closed");

    // TODO?
    while ((number_of_bytes = recv(sock_fd, buffer, BUFFER_SIZE, 0)) > 0) {
#ifdef DEBUG
        printf("[LOG]:   Received chunk with %d bytes\n", number_of_bytes);
#endif
        void *tmp = malloc(total_number_of_bytes + number_of_bytes);
        if (response.data != NULL) {
            memcpy(tmp, response.data, total_number_of_bytes);
            free(response.data);
        }
        memcpy(tmp + total_number_of_bytes, buffer, number_of_bytes);
        response.data = tmp;
        total_number_of_bytes += number_of_bytes;
    }

    if (total_number_of_bytes == 0) {
        LOG("Socket Closed");
        response.status_code = STATUS_SOCKET_CLOSED;
        return response;
    }
    LOG_INT((int32) total_number_of_bytes);
    response.data = malloc(total_number_of_bytes);
    response.data_length = total_number_of_bytes;
    response.status_code = STATUS_OK;
    memcpy(response.data, buffer, total_number_of_bytes);

    return response;
}

 */