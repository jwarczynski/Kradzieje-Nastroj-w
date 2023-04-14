#include <map>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>

#include <queue>
#include <mutex>
#include <thread>
#include <random>
#include <chrono>
#include <algorithm>
#include <vector>
#include <limits>

#include "main.h"


/* boolean */
#define TRUE 1
#define FALSE 0

#define ROOT 0
#define EQUIPMENT_NUM 2
#define LAB_NUM 2
#define TAG 2

enum MsgType {
	LAB_REQUEST,
	EQUIPMENT_REQUEST,
	LAB_RELEASE,
	EQUIPMENT_RELEASE,
	ACK 
};

const char* MsgTypeStrings[] = {
  "LAB_REQUEST",
	"EQUIPMENT_REQUEST",
	"LAB_RELEASE",
	"EQUIPMENT_RELEASE",
	"ACK"
};

/* typ pakietu */
typedef struct {
    int src;    

    int data;     /* przykładowe pole z danymi; można zmienić nazwę na bardziej pasującą */

    int timestamp;
} packet_t;


// Custom comparator function
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


void processLabRequest(state_t&, packet_t&, int);
void processEquipmentRequest(state_t&, packet_t&, int);
void processLabReleaseRequest(state_t& ,packet_t&, int);
void processEquipmentReleaseRequest(state_t& ,packet_t&, int);


void incrementLogicTime(state_t& processState) {
	processState.timeMutex.lock();
	processState.timestamp ++;
	processState.timeMutex.unlock();
}
// Define a function to check if a packet with given src is in the top n packets of a priority queue
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

void removePacketWithSrc(std::priority_queue<packet_t, std::vector<packet_t>,PacketComparator>& pq, int src) {
    // Copy packets to a temporary vector, removing the packet with given src
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


/* packet_t ma trzy pola, więc NITEMS=3. Wykorzystane w inicjuj_typ_pakietu */
#define NITEMS 3

/* Typy wiadomości */
#define APP_PKT 1

MPI_Datatype MPI_PAKIET_T;

/* tworzy typ MPI_PAKIET_T
*/
void inicjuj_typ_pakietu()
{
    /* Stworzenie typu */
    /* Poniższe (aż do MPI_Type_commit) potrzebne tylko, jeżeli
       brzydzimy się czymś w rodzaju MPI_Send(&typ, sizeof(pakiet_t), MPI_BYTE....
    */
    /* sklejone z stackoverflow */
    const int nitems=NITEMS; 
    int       blocklengths[NITEMS] = {1,1,1};
    MPI_Datatype typy[NITEMS] = {MPI_INT, MPI_INT, MPI_INT};

    MPI_Aint     offsets[NITEMS]; 
    offsets[0] = offsetof(packet_t, src);
    offsets[1] = offsetof(packet_t, data);
    offsets[2] = offsetof(packet_t, timestamp);

    MPI_Type_create_struct(nitems, blocklengths, offsets, typy, &MPI_PAKIET_T);
    MPI_Type_commit(&MPI_PAKIET_T);
}

void waitRandomTime() {
	  std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(100, 5000);

    int sleepTime = dis(gen);
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
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

void updateAck(state_t& processState, packet_t ackPacket) {
	processState.acks[ackPacket.src] = ackPacket.timestamp;
}

void processResourceMessages(state_t& processState, int rank) {
		packet_t packet;
		MPI_Status status;

		while (true) {
			MPI_Recv( &packet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, TAG, MPI_COMM_WORLD, &status);
			//debug("RECEIVED %s packet from %d with timestamp: %d\n", MsgTypeStrings[packet.data], packet.src, packet.timestamp);
			processState.timeMutex.lock();
			processState.timestamp = std::max(processState.timestamp, packet.timestamp) + 1;
			switch (packet.data) {
				case LAB_REQUEST: processLabRequest(processState, packet, rank); break;
				case EQUIPMENT_REQUEST: processEquipmentRequest(processState, packet, rank); break;
				case LAB_RELEASE: processLabReleaseRequest(processState, packet, rank); break;
				case EQUIPMENT_RELEASE: processEquipmentReleaseRequest(processState, packet, rank); break;
				case ACK: updateAck(processState, packet);
			}
			processState.timeMutex.unlock();
		}
}

void processLabReleaseRequest(state_t& processState, packet_t& packet, int processId) {
		processState.labMutex.lock();
		removePacketWithSrc(processState.labQueue, packet.src);
		processState.labMutex.unlock();
}

void processEquipmentReleaseRequest(state_t& processState, packet_t& packet, int processId) {
		processState.equipmentMutex.lock();
		removePacketWithSrc(processState.equipmentQueue, packet.src);
		processState.equipmentMutex.unlock();
}

void processLabRequest(state_t& processState, packet_t& packet, int rank) {
	processState.labMutex.lock();
	processState.labQueue.push(packet);
	processState.labMutex.unlock();
	packet_t ackPacket = {
		rank,
		ACK,
		processState.timestamp + 1
	};
	processState.timestamp += 1;
	MPI_Send( &ackPacket, 1, MPI_PAKIET_T, packet.src, TAG, MPI_COMM_WORLD);
	debug("SEND ack to %d with timestamp %d", packet.src, processState.timestamp);
}

void processEquipmentRequest(state_t& processState, packet_t& packet, int rank) {
	processState.equipmentMutex.lock();
	processState.equipmentQueue.push(packet);
	processState.equipmentMutex.unlock();
	packet_t ackPacket = {
		rank,
		ACK,
		processState.timestamp + 1
	};
	processState.timestamp += 1;
	MPI_Send( &ackPacket, 1, MPI_PAKIET_T, packet.src, TAG, MPI_COMM_WORLD);
	debug("SEND ack to %d with timestamp %d", packet.src, processState.timestamp);
}

int main(int argc, char **argv)
{
		//std::priority_queue<packet_t, std::vector<packet_t>, PacketComparator> equpimentQueue;
		//packet_t older {
		//	0, 1, 0
		//};
		//
		//packet_t younger {
		//	0, 1, 1
		//};

		//packet_t younger2 {
		//	1, 1, 1
		//};

		//equpimentQueue.push(younger);
		//equpimentQueue.push(older);
		//equpimentQueue.push(younger2);
		
    int rank, size, provided;
    MPI_Status status;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    srand(rank);
    inicjuj_typ_pakietu(); // tworzy typ pakietu
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	//	for(int i = 0; i < equpimentQueue.size(); ++i) {
	//		println("priority:%d time:%d src:%d", i, equpimentQueue.top().timestamp, equpimentQueue.top().src);
	//		equpimentQueue.pop();
	//	}
			int numThreads = std::stoi(argv[1]);	
	//		println("threads: %d", numThreads);

	
		

		std::priority_queue<packet_t, std::vector<packet_t>, PacketComparator> equpimentQueue;
		std::priority_queue<packet_t, std::vector<packet_t>, PacketComparator> labQueue;
		int timestamp = 0;
    std::mutex eqMutex, labMutex, timestampMutex;
		state_t state {
			.labQueue = labQueue,
			.equipmentQueue = equpimentQueue,
			.timestamp = timestamp,
			.labMutex = labMutex,
			.equipmentMutex = eqMutex,
			.timeMutex = timestampMutex,
			.acks = std::map<int, int>(),
		};

		for (int i = 0; i < numThreads; i++) {
				state.acks[i] = -1;
		}
		state.acks[rank] = std::numeric_limits<int>::max();

	  debug("starting processing");
		std::thread kradziejThread(kradziejuj, std::ref(state), std::ref(rank));
		std::thread resourceThread(processResourceMessages, std::ref(state), std::ref(rank));

		kradziejThread.join();
		resourceThread.join();

    MPI_Type_free(&MPI_PAKIET_T);
    MPI_Finalize();
   return 0;
}
