#include <sys/socket.h>
#include <netinet/in.h>
#include "generic/network.h"
#include "generic/data_helper.h"
#include "generic/hash_helper.h"
#include "debug.h"
#include "generic/commander.h"

typedef struct{
    uint32_t ip;
    uint16_t id;
    uint16_t port;
} Peer;

typedef struct {
    Peer this;
    Peer next;
    Peer prev;
} PeerInfo;

const PeerInfo peer_info;

NETWORK_RECEIVE_HANDLER(receive_handler, rec, sock_fd) {
    LOG("Parsing request");




    if (isPeerProtocol(rec)){
        PeerProtocol decodedData = {};       //build PeerHeader
        decode_peerProtocol(rec->data, &decodedData);

        if(decodedData.lookup){
            LOG(["LOOKUP"]);
            if(lookup_is_Responsible(decodedData, peer_info.this, peer_info.prev)) { //if thisIsResponsible or nextIsResponsible
                sendFoundLookup(decodedData, peer_info.this); //setReplyBit setThisHashProtocol
            } else if (lookup_is_Responsible(decodedData, peer_info.next, peer_info.this)){
                sendFoundLookup(decodedData, peer_info.next);
            } else {
                sendLookupRequest(decodedData, peer_info.next); //else ask next one
            }
        } else if(decodedData.reply){ //is lookup answer
            decodedData = peer_to_hash_Protocol(decodedData);
            request(decodedData,decodedData.length); //get data
        } else {
            ERROR("false Request"));
        }
    } else{ //ClientProtocol
        struct ClientProtocol decodedData = {};
        decode_clientProtocol(rec->data, &decodedData);
        if(isResponsibleforHash(decodedData)){
            send(sock_fd, proceedRequest(decodedData), proceedRequest(decodedData).length);
        } else {
            sendLookup(peerInfo.nextPeer,buildPeerProtocpl(decodedData));
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
        int code = receive(sock_fd, receive_handler);
        if (code == STATUS_OK) {
            LOG("Status OK");
        } else {
            ERROR("Error while connecting");
            THROW(-1)
        }
    }
}
