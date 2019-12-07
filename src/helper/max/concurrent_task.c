#include <stdio.h>
#include <unistd.h>
#include "concurrent_task.h"
#include "../wulkanat/debug.h"
#include "../wulkanat/queue.h"

void *Stabilize_caller(void *block)
{
    Stabilizer_ctrl_block *data = (Stabilizer_ctrl_block *)block;
    data->control = 1;

    LOG("Starting Stabilizer");
    while(data->control != 0) {
        sleep(2);
        //queue_append(client_requests, )
        //printf("test, %i\n", data->num);
    }
    LOG("Ending Stabilizer");
    return NULL;
}

