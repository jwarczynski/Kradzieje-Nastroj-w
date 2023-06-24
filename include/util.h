//
// Created by Jj on 6/23/23.
//

#ifndef KRADZIEJE_NASTROJOW_UTIL_H
#define KRADZIEJE_NASTROJOW_UTIL_H

#include "types.h"

void removePacketWithSrc(std::priority_queue<packet_t, std::vector<packet_t>,PacketComparator>& pq, int src);
void waitRandomTime();

#endif //KRADZIEJE_NASTROJOW_UTIL_H
