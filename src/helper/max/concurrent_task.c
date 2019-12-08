#include <stdio.h>
#include <unistd.h>
#include "concurrent_task.h"
#include "../wulkanat/debug.h"
#include "../../generic/network.h"
#include "../../generic/data_helper.h"

void *Stabilize_caller(void *block)
{
    Stabilizer_ctrl_block *current_Peer_stabilizer = (Stabilizer_ctrl_block *)block;
    current_Peer_stabilizer->control = 1;

    LOG("Starting Stabilizer");
    while(current_Peer_stabilizer->control != 0) {
        sleep(2);
        int_addr_to_str(currentNodeIP, current_Peer_stabilizer->current_Peer.this.ip)
        int_port_to_str(currentNodePort, current_Peer_stabilizer->current_Peer.this.port)
        Response stabilize_request;
        PeerProtocol stabilize_data = make_peerProtocol(0, 0, 0, current_Peer_stabilizer->current_Peer.this);
        stabilize_data.stabilize = 1;
        stabilize_request.data = (PeerProtocol*) &stabilize_data;

        direct_send(currentNodeIP, currentNodePort, stabilize_request.data, stabilize_request.data_length);
    }
    LOG("Ending Stabilizer");
    return NULL;
}

