//
// Created by Jj on 6/23/23.
//

#include "kradziej.h"
#include "util.h"
#include "debug_macro.h"

void kradziejuj(state_t& processState, int rank) {
		int num_processes;
		MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

		while (true) {
			packet_t requestPkt = sendResourceRequests(processState, rank, num_processes, EQUIPMENT_REQUEST);	
			debug("SEND EQUIPMENT_REQUEST to everyone ts:%d", processState.timestamp);
			waitForAcks(processState, requestPkt.timestamp, num_processes, rank);	
			processState.equipmentMutex.lock();
			processState.equipmentQueue.push(requestPkt);
			processState.equipmentMutex.unlock();
			debug("COLLECTED EQUIPMENT_REQUEST ACKs from everyone ts:%d", processState.timestamp);
			while(!isPacketInTopNPackets(processState.equipmentQueue, EQUIPMENT_NUM, rank)){
				debug("WAIT for equipment");
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
			}
			processState.equipmentMutex.lock();
			removePacketWithSrc(processState.equipmentQueue, rank);
			processState.equipmentMutex.unlock();
			hunt(rank);

			std::thread chargeEquipmentThread(chargeEquipment, std::ref(processState), rank, EQUIPMENT_RELEASE, num_processes);
			chargeEquipmentThread.detach();

			packet_t labRequestPkt = sendResourceRequests(processState, rank, num_processes, LAB_REQUEST);	
			debug("SEND LAB_REQUEST to everyone ts:%d", processState.timestamp);
			waitForAcks(processState, labRequestPkt.timestamp, num_processes, rank);	
			processState.labMutex.lock();
			processState.labQueue.push(requestPkt);
			processState.labMutex.unlock();
			debug("COLLECTED LAB_REQUEST ACKs from everyone\n", rank);
			while(!isPacketInTopNPackets(processState.labQueue, LAB_NUM, rank)){
				debug("WAIT for laboratory");
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
			}
			makeChewingGum(rank);
			debug("MADE chewing gum");
			broadcastLabRelease(processState, num_processes, rank);
			debug("BRODCASTED lab release");
		}
}

void hunt(int rank) {
	println("TOOK equipment");
	waitRandomTime();
	debug("FOUND a victim");
}

void makeChewingGum(int rank) {
	println("USE laboratory");
	waitRandomTime();
	println("LEFT laboratory");
}

bool acksCollected(std::map<int, int>& myMap, int requestTime) {
    for (auto& [key, value] : myMap) {
        if (value < requestTime) {
            return false;
        }
    }
    return true;
}


void waitForAcks(state_t& processState, int requestTime, int num_processes, int rank) {
		while(!acksCollected(processState.acks, requestTime)) {
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
}

packet_t sendResourceRequests(state_t& processState, int rank, int num_processes, MsgType requestType) {
		processState.timeMutex.lock();

		packet_t requestPkt{
			rank,
			requestType,
			processState.timestamp
		};

		for (int dest=0; dest<num_processes; ++dest) {
			if (dest != rank) {
				MPI_Send(&requestPkt, 1, MPI_PAKIET_T, dest, TAG, MPI_COMM_WORLD);
				debug("SEND resource request of type %s to %d with timestamp %d", MsgTypeStrings[requestType], dest, processState.timestamp);
			}	
		}
		processState.timeMutex.unlock();
		return requestPkt;
}

void chargeEquipment(state_t& processState, int rank, MsgType releaseType, int num_processes) {
		processState.timeMutex.lock();

		packet_t releasePkt{
			rank,
			releaseType,
			processState.timestamp
		};

		waitRandomTime();

		for (int dest=0; dest<num_processes; ++dest) {
			if (dest != rank) {
				MPI_Send(&releasePkt, 1, MPI_PAKIET_T, dest, TAG, MPI_COMM_WORLD);
			}	
		}
		processState.timestamp ++;

		processState.timeMutex.unlock();
		println("RELEASED equipment");
}

void brodcastMessage(packet_t& packet, state_t& processState, int num_processes, int rank, int tag) {
	for (int dest=0; dest<num_processes; ++dest) {
			if (dest != rank) {
				incrementLogicTime(processState);
				processState.timeMutex.lock();
				packet.timestamp = processState.timestamp;
				MPI_Send(&packet, 1, MPI_PAKIET_T, dest, TAG, MPI_COMM_WORLD);
				processState.timeMutex.unlock();
			}	
		}
}

void broadcastLabRelease(state_t &processState, int num_processes, int rank) {
	packet_t packet {
		rank,
		LAB_RELEASE,
		processState.timestamp
	};
	
	brodcastMessage(packet, processState, num_processes, rank, TAG);
}

bool isPacketInTopNPackets(std::priority_queue<packet_t, std::vector<packet_t>, PacketComparator > pq, int n, int rank) {
    // Copy top n packets to a temporary vector
    std::vector<packet_t> topNPackets;
    for (int i = 0; i < n && !pq.empty(); i++) {
        topNPackets.push_back(pq.top());
        pq.pop();
    }
    // Check if a packet with given src is in the top n packets
    bool found = false;
		int i = 1;	
    for (const auto& packet : topNPackets) {
				debug("%d priority: rank: %d request timestamp:%d", i, packet.src, packet.timestamp)
        if (packet.src == rank) {
            found = true;
            break;
        }
				i++;
    }
    return found;
}
