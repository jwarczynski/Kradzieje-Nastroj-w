//
// Created by Jj on 6/23/23.
//

#ifndef KRADZIEJE_NASTROJOW_KRADZIEJ_H
#define KRADZIEJE_NASTROJOW_KRADZIEJ_H

#include "types.h"


#define EQUIPMENT_NUM 2
#define LAB_NUM 2


void kradziejuj(state_t& processState, int rank);

void hunt(int rank);
void makeChewingGum(int rank);
bool acksCollected(std::map<int, int>& myMap, int requestTime);
void waitForAcks(state_t& processState, int requestTime, int num_processes, int rank);
packet_t sendResourceRequests(state_t& processState, int rank, int num_processes, MsgType requestType);
void chargeEquipment(state_t& processState, int rank, MsgType releaseType, int num_processes);
void brodcastMessage(packet_t& packet, state_t& processState, int num_processes, int rank, int tag);
void broadcastLabRelease(state_t &processState, int num_processes, int rank);
void incrementLogicTime(state_t& processState);
bool isPacketInTopNPackets(std::priority_queue<packet_t, std::vector<packet_t>, PacketComparator > pq, int n, int rank);


#endif //KRADZIEJE_NASTROJOW_KRADZIEJ_H
