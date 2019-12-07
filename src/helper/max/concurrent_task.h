#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "../../generic/data_helper.h"

typedef struct{
    int8_t control;     //switch to turn thread off = 0
    pthread_t pid;
    PeerInfo Peer;
}Stabilizer_ctrl_block;

/**
 * Concurrently call stabilize function
 *
 * @param pointer to control block
 * @return (void) called using an independent thread
 */
void *Stabilize_caller(void *block);