//
// Created by Jj on 6/23/23.
//

#ifndef KRADZIEJE_NASTROJOW_TYPES_H
#define KRADZIEJE_NASTROJOW_TYPES_H

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>

#include <queue>
#include <thread>
#include <random>
#include <chrono>
#include <algorithm>
#include <vector>
#include <limits>
#include <map>

#include <mutex>
#include <queue>

extern MPI_Datatype MPI_PAKIET_T;


void initPacketType();

#define ROOT 0
#define TAG 2

enum MsgType {
    LAB_REQUEST,
    EQUIPMENT_REQUEST,
    LAB_RELEASE,
    EQUIPMENT_RELEASE,
    ACK
};

extern const char* MsgTypeStrings[];

typedef struct {
    int src;
    int data;
    int timestamp;
} packet_t;


struct PacketComparator {
    bool operator()(const packet_t& p1, const packet_t& p2) const {
        if (p1.timestamp > p2.timestamp) {
            return true;
        } else if (p1.timestamp < p2.timestamp) {
            return false;
        } else {
            // timestamps are equal, compare src fields
            return p1.src > p2.src;
        }
    }
};

typedef struct {
    std::priority_queue<packet_t, std::vector<packet_t>, PacketComparator > labQueue;
    std::priority_queue<packet_t, std::vector<packet_t>, PacketComparator > equipmentQueue;
    int timestamp;
    std::mutex& labMutex;
    std::mutex& equipmentMutex;
    std::mutex& timeMutex;
    std::map<int, int> acks;
} state_t;

#endif
