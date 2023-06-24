//
// Created by Jj on 6/24/23.
//

#include "util.h"
#include "types.h"

const char* MsgTypeStrings[] = {
    "LAB_REQUEST",
    "EQUIPMENT_REQUEST",
	"LAB_RELEASE",
	"EQUIPMENT_RELEASE",
	"ACK"
};

void removePacketWithSrc(std::priority_queue<packet_t, std::vector<packet_t>,PacketComparator>& pq, int src) {
    std::vector<packet_t> temp;
    while (!pq.empty()) {
        if (pq.top().src == src) {
            pq.pop();
            break;
        } else {
            temp.push_back(pq.top());
            pq.pop();
        }
    }
    // Push remaining packets back into the queue
    for (const auto& packet_t : temp) {
        pq.push(packet_t);
    }
}

void waitRandomTime() {
	  std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(100, 5000);

    int sleepTime = dis(gen);
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
}

