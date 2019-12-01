#include <sys/socket.h>
#include <netinet/in.h>
#include "generic/network.h"
#include "generic/data_helper.h"
#include "generic/hash_helper.h"
#include "debug.h"
#include "generic/commander.h"
#include "generic/peer_lookup.h"

typedef struct {
    Peer this;
    Peer next;
    Peer prev;
} PeerInfo;

const PeerInfo peer_info;


void peer_hash_handler(ClientProtocol decodedData,int sock_fd) {
    LOG("Parsing request");

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






NETWORK_RECEIVE_HANDLER(receive_handler, rec, sock_fd) {
    LOG("Parsing request");


    if (isPeerProtocol(rec)) {
        PeerProtocol decodedData = {};       //build PeerHeader
        decode_peerProtocol(rec->data, &decodedData);

        if (decodedData.lookup) {
            LOG("LOOKUP");
            if (lookup_is_responsible(decodedData.hashId, peer_info.next, peer_info.this)) {
                send_found_lookup(decodedData, peer_info.next);
            } else {
                send_lookup_request(decodedData, peer_info.next); //else ask next one
            }
        } else if (decodedData.reply) {
            int sock_reply_server = setup_as_client(decodedData.nodeIp, decodedData.nodePort);
            send(sock_reply_server, first_in_queue.data, first_in_queue.data_length);
            receive(sock_reply_server, /*TODO...*/ );
            send(first_in_queue.socket, fromServerRecievedData, fromServerRecievedData.length);
            pop_from_queue();
        } else {
            ERROR("false Request"));
        }
    } else { //isClientProtocol(rev)

        ClientProtocol decodedData = {};
        decode_clientProtocol(rec->data, &decodedData);

        if(lookup_is_free) {
            PeerProtocol hash_data = client_to_peer_protocol(decodedData);
            if (lookup_is_responsible(hash_data.hashId, peer_info.this, peer_info.prev)) { //if thisIsResponsible
                peer_hash_handler(decodedData, sock_fd); //answer to client
            } else if (lookup_is_responsible(hash_data.hashId, peer_info.next, peer_info.this)) {
                int sock_reply_server = setup_as_client(hash_data.nodeIp, hash_data.nodePort);
                send(sock_reply_server, decodedData, sizeof(decodedData)); //TODO: maybe in Bits and not in Bytes
                receive(sock_reply_server, //TODO);
                send(sock_fd, fromServerRecievedData, fromServerRecievedData.length);
            } else {
                hash_data.lookup = LOOKUP_BIT;
                send_lookup_request(hash_data, peer_info.next); //else ask next one
                push_in_queue(decodedData);
            }
        }
    }
}



//TODO: how implement more than one clientRequest
//TODO create hashHead
//TODO call lookup(hashValue)
//TODO request to peer
//TODO answer to peer
//TODO answer to client
/*
check protocolBIT (controlBIT 1 or 0)
    if !isPeerProtocol():
        build peerProtocol;
        if responsible for hashID:
            useHashTable(headClient);
            answer to client; BREAK;
        else:
            lookup_BIT; lookup();
    else: //(isPeerProtocol)
        controlBIT == 1:
        if lookupBIT == 1;
            lookup();
        else if requestBIT:
            answer = sendRequestToServer();
            answer to client; BREAK;

 */
}


DEBUGGABLE_MAIN(argc, argv)

    //TODO: read data throught  pipeline

    STR_ARG(myID, 0)
    STR_ARG(myIP, 1)
    STR_ARG(myPORT, 2)

    STR_ARG(nextID, 3)
    STR_ARG(nextIP, 4)
    STR_ARG(nextPORT, 5)

    STR_ARG(prevID, 6)
    STR_ARG(prevIP, 7)
    STR_ARG(prevPORT, 9)

    LOG("Starting Peer");
    int sock_fd = setup_as_server(myPORT);

    while (1) {

        //receive or pop_queue
        int code = receive(sock_fd, receive_handler);
        if (code == STATUS_OK) {
            LOG("Status OK");
        } else {
            ERROR("Error while connecting");
            THROW(-1)
        }
    }
}
