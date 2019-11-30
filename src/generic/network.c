#include "../debug.h"
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include "network.h"

#define BACKLOG 10     // how many pending connections queue will hold

void sigchld_handler(int s) {
    (void) s; // quiet unused variable warning

    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

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

int setup_as_client(char *addr, char *port) {
    int
        sock_fd = -1,
        rv;
    struct addrinfo hints;
    struct addrinfo *servinfo;
    struct addrinfo *p;
    char s[INET6_ADDRSTRLEN];

    //define hints
    memset(&hints, 0, sizeof hints); //empty hint
    hints.ai_family = AF_UNSPEC; //IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; //TCP Protocol

    if ((rv = getaddrinfo(addr, port, &hints, &servinfo)) != 0) {
        ERROR("Malformed address or port");
        return -1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
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

    freeaddrinfo(servinfo);

    return sock_fd;
}

int setup_as_server(char *port) {
    int sock_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sigaction sa;
    const int yes = 1;

    //check Port
    if (atoi(port) < 0 || atoi(port) > 65535) {
        ERROR("Invalid Port");
        return -1;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    int rv;
    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        ERROR("Could not get Address");
        return -1;
    }

    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
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

    freeaddrinfo(servinfo); // all done with this structure

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

    struct sockaddr_storage their_addr;
    char* s;

    size_t sin_size = sizeof(their_addr);
    int new_fd;
    LOG("Waiting for Connection");
    while ((new_fd = accept(sock_fd, (struct sockaddr *) &their_addr, (socklen_t*) &sin_size)) == -1) {/*noop*/}

    inet_ntop(their_addr.ss_family,
              get_in_addr((struct sockaddr *) &their_addr),
              s, sizeof s);
    LOG("Got Connection");
    close(sock_fd);

    return new_fd;
}

int receive(int sock_fd, NETWORK_RECEIVE_FNPTR(callback)) {
    struct Response response = {};

    char buffer[BUFFER_SIZE];
    // int number_of_bytes;
    size_t total_number_of_bytes = recv(sock_fd, buffer, BUFFER_SIZE, 0);
    response.data = NULL;

    if (total_number_of_bytes == 0)
        ERROR("Socket is closed");

    // TODO?
    /*while ((number_of_bytes = recv(sock_fd, buffer, BUFFER_SIZE, 0)) > 0) {
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
    }*/

    if (total_number_of_bytes == 0) {
        LOG("Socket Closed");
        return STATUS_SOCKET_CLOSED;
    }
    LOG_INT((int) total_number_of_bytes);
    LOG_INT((int) total_number_of_bytes);
    response.data = malloc(total_number_of_bytes);
    memcpy(response.data, buffer, total_number_of_bytes);
    response.data_length = total_number_of_bytes;
    response.status_code = 200;

    callback(&response, sock_fd);

    return STATUS_OK;
}
