#include <sys/socket.h>
#include "generic/network.h"
#include "generic/data_helper.h"
#include "generic/hash_helper.h"
#include "helper/wulkanat/debug.h"
#include "helper/wulkanat/commander.h"
#include "helper/wulkanat/queue.h"
#include "helper/wulkanat/flow_plus.h"

QUEUE(client_requests, unknown*)
QUEUE(sockets, int32)

PeerInfo peer_info;

void respond_as_responsible_peer(ClientProtocol decodedData, int32 sock_fd) {
    LOG("Parsing request");

    if (decodedData.get) {
        LOG("[GET]");
        HEX_VALUE_LOG(decodedData.key, decodedData.key_length);

        struct HashElement *element = get(decodedData.key, decodedData.key_length);

        if (element == NULL) {
            LOG("Empty value (not found)");
            byte8 res[sizeof(byte8) + sizeof(decodedData.key_length) + sizeof(decodedData.value_length)];
            as(byte16, res + sizeof(byte8)) = 0u;
            as(byte32, res + sizeof(byte8) + sizeof(byte16)) = 0u;
            *res = ACK_BIT COMBINE GET_BIT;
            LOG("Acknowledged");
            send(sock_fd, &res, sizeof(byte8) + sizeof(decodedData.key_length) + sizeof(decodedData.value_length));
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
        byte8 res[sizeof(byte8) + sizeof(decodedData.key_length) + sizeof(decodedData.value_length)];
        as(byte16, res + sizeof(byte8)) = 0u;
        as(byte32, res + sizeof(byte8) + sizeof(byte16)) = 0u;
        *res = ACK_BIT COMBINE SET_BIT;
        LOG("Acknowledged");
        send(sock_fd, &res, sizeof(byte8) + sizeof(decodedData.key_length) + sizeof(decodedData.value_length));
    }
    if (decodedData.delete) {
        LOG("[DELETE]");
        delete_element((unknown *) decodedData.key, decodedData.key_length);
        byte8 res[sizeof(byte8) + sizeof(decodedData.key_length) + sizeof(decodedData.value_length)];
        as(byte16, res + sizeof(byte8)) = 0u;
        as(byte32, res + sizeof(byte8) + sizeof(byte16)) = 0u;
        *res = ACK_BIT COMBINE DELETE_BIT;
        LOG("Acknowledged");
        send(sock_fd, &res, sizeof(byte8) + sizeof(decodedData.key_length) + sizeof(decodedData.value_length));
    }
}

void next_job_in_queue() {
    if (queue_is_empty(client_requests)) return;

    LOG("Executing next Job");

    unknown *next_client_job = NULL;
    queue_peek(client_requests, next_client_job)
    ClientProtocol decodedData = {};
    decode_clientProtocol(next_client_job, &decodedData);

    PeerProtocol peer_request = make_peerProtocol(false, true,
                                                  get_hash_value(decodedData.key, decodedData.key_length),
                                                  peer_info.this);
    unknown *encoded_response = encode_peerProtocol(&peer_request);

    int_addr_to_str(next_addr, peer_info.next.ip)
    int_port_to_str(next_port, peer_info.next.port)
    int32 next_peer = setup_as_client(next_addr, next_port);
    send(next_peer, encoded_response, clientProtocolCalculateSize(&decodedData));
}

NETWORK_RECEIVE_HANDLER(receive_handler, rec, sock_fd) {
    LOG("Parsing request");

    if (isPeerProtocol(rec->data)) {
        LOG("Peer");
        PeerProtocol decodedData = {};       //build PeerHeader
        decode_peerProtocol(rec->data, &decodedData);

        if (decodedData.lookup) {
            LOG("Lookup");
            if (lookup_is_responsible(decodedData.hashId, peer_info.next, peer_info.this)) {
                LOG("Next peer is responsible");
                send_found_lookup(&decodedData, peer_info.next);
            } else {
                LOG("Not found, redirecting to next peer");
                int_addr_to_str(nodeIp, peer_info.next.ip)
                int_port_to_str(nodePort, peer_info.next.port)
                direct_send(nodeIp, nodePort, rec->data, rec->data_length);
            }
        } else if (decodedData.reply) {
            LOG("Reply");
            unknown *current_request = NULL;
            queue_pop(client_requests, current_request)
            ClientProtocol current_decoded_request = {};
            decode_clientProtocol(current_request, &current_decoded_request);

            int_addr_to_str(nodeIp, decodedData.nodeIp)
            int_port_to_str(nodePort, decodedData.nodePort)
            int32 next_node_sock = direct_send(nodeIp, nodePort, current_request, clientProtocolCalculateSize(&current_decoded_request));

            int32 client_sock = 0;
            queue_pop(sockets, client_sock)
            redirect(next_node_sock, client_sock);
            next_job_in_queue();

        } else {
            ERROR("Falsy Request");
        }
    } else { // clientProtocol
        LOG("Client");
        ClientProtocol decodedData = {};
        decode_clientProtocol(rec->data, &decodedData);

        int16 hashId = get_hash_value(decodedData.key, decodedData.key_length);
        if (lookup_is_responsible(hashId, peer_info.this, peer_info.prev)) {
            // this peer is responsible
            LOG("This peer is responsible");
            respond_as_responsible_peer(decodedData, sock_fd); //answer to client peer_hash_handler(decodedData, sock_fd);
        } else if (lookup_is_responsible(hashId, peer_info.next, peer_info.this)) {
            // next peer is responsible
            LOG("Next peer is responsible");
            int_addr_to_str(next_addr, peer_info.next.ip)
            int_port_to_str(next_port, peer_info.next.port)
            int32 next_peer = setup_as_client(next_addr, next_port);

            send(next_peer, rec->data, rec->data_length); //TODO: maybe in Bits and not in Bytes
            redirect(next_peer, sock_fd);
        } else {
            // unknown peer is responsible
            LOG("Unknown Peer is responsible, asking next peer.");
            queue_append(sockets, sock_fd)
            size_t data_size = clientProtocolCalculateSize(&decodedData);
            unknown *data = malloc(data_size);
            memcpy(data, rec, data_size);
            queue_append(client_requests, data)

            if (queue_is_empty(client_requests)) {
                LOG("Queue is empty");
                next_job_in_queue();
            }
        }
    }
}

// suppress atio() warning
#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err34-c"
void grab_data_to_peer_info(int32 argc, string argv[]) {
    STR_ARG(myID, 0)
    STR_ARG(myIP, 1)
    STR_ARG(myPORT, 2)
    str_addr_to_int(myAddrInt, myIP)
    str_port_to_int(myPortInt, myPORT)
    peer_info.this.ip = myAddrInt;
    peer_info.this.port = myPortInt;
    peer_info.this.id = atoi(myID);

    STR_ARG(nextID, 3)
    STR_ARG(nextIP, 4)
    STR_ARG(nextPORT, 5)
    str_addr_to_int(nextAddrInt, nextIP)
    str_port_to_int(nextPortInt, nextPORT)
    peer_info.next.ip = nextAddrInt;
    peer_info.next.port = nextPortInt;
    peer_info.next.id = atoi(nextID);

    STR_ARG(prevID, 6)
    STR_ARG(prevIP, 7)
    STR_ARG(prevPORT, 8)
    str_addr_to_int(prevAddrInt, prevIP)
    str_port_to_int(prevPortInt, prevPORT)
    peer_info.prev.ip = prevAddrInt;
    peer_info.prev.port = prevPortInt;
    peer_info.prev.id = atoi(prevID);
}
#pragma clang diagnostic pop

DEBUGGABLE_MAIN(argc, argv)
    grab_data_to_peer_info(argc, argv);
    STR_ARG(myPORT, 2)

    LOG("Starting Peer");
    LOG_STR(myPORT);
    int sock_fd = setup_as_server(myPORT);

    loop {
        // TODO: select
        int new_sock = get_new_connection(sock_fd);
        int code = receive(new_sock, receive_handler);
        if (code == STATUS_OK) {
            LOG("Status OK");
        } else if (code == STATUS_SOCKET_CLOSED) {
            LOG("Connection terminated by client.");
        } else {
            ERROR("Error while connecting");
            THROW(-1)
        }
    }
}