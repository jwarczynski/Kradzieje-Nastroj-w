//
// Created by Jj on 6/23/23.
//

#ifndef KRADZIEJE_NASTROJOW_REQUEST_PROCESSING_H
#define KRADZIEJE_NASTROJOW_REQUEST_PROCESSING_H

#include "types.h"

void processResourceMessages(state_t& processState, int rank);

void processLabRequest(state_t&, packet_t&, int);
void processEquipmentRequest(state_t&, packet_t&, int);
void processLabReleaseRequest(state_t& ,packet_t&, int);
void processEquipmentReleaseRequest(state_t& ,packet_t&, int);

void updateAck(state_t& processState, packet_t ackPacket);

#endif //KRADZIEJE_NASTROJOW_REQUEST_PROCESSING_H
