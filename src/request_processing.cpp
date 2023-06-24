//
// Created by Jj on 6/24/23.
//

#include "request_processing.h"
#include "util.h"
#include "types.h"
#include "debug_macro.h"

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

void incrementLogicTime(state_t& processState) {
	processState.timeMutex.lock();
	processState.timestamp ++;
	processState.timeMutex.unlock();
}
