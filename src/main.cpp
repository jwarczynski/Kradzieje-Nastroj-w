//
// Created by Jj on 6/23/23.
//

#include "kradziej.h"
#include "request_processing.h"
#include "debug_macro.h"
#include "types.h"

int main(int argc, char **argv) {
    int rank, size, provided;
    MPI_Status status;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    srand(rank);
    initPacketType(); // tworzy typ pakietu
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int numThreads = std::stoi(argv[1]);	

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
