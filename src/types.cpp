
#include "types.h"


#define NITEMS 3

/* Typy wiadomości */
#define APP_PKT 1

MPI_Datatype MPI_PAKIET_T;

/* tworzy typ MPI_PAKIET_T
*/
void initPacketType()
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

