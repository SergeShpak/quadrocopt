#ifndef NETWORK_INTERACTIONS_H
#define NETWORK_INTERACTIONS_H

#include <stddef.h>
#include <netinet/in.h>
#include <pthread.h>

#include "data_struct.h"

/******************************************************************************
**  NetworkInterface structure  ***********************************************
******************************************************************************/

/**
 * @brief A structure representing network interface of server.
 *
 * Represents the network interface of server. Contains sockets to be used by
 * two listeners and sender. Sockets are bound to ports deined in 
 * ./include/constants.h.
 */
typedef struct _NetworkInterface {
  int sd_in_first;
  int sd_in_second;
  int sd_out;
  struct sockaddr_in skaddr_in_first;
  struct sockaddr_in skaddr_in_second;
  struct sockaddr_in skaddr_out;
} NetworkInterface;

/**
 * @brief Constructor for \ref NetworkInterface structure.
 *
 * Creates a NetworkInterface datastructure on the heap. It binds the sockets
 * to ports defined in ./include/constants.h.
 * @return Pointer to initialized \ref NetworkInterface structure.
 */
NetworkInterface *initialize_network_interface();

/**
 * @brief Frees \ref NetworkInterface structure.
 *
 * Frees passed NetworkInterface structure. If a NULL pointer is passed, does
 * nothing. As NetworkInterface structure does not have any fancy internals
 * at the moment, just calls free on the passed pointer.
 * param[in] ni A pointer to the NetworkInterface structure to be freed.
 */
void free_network_interface(NetworkInterface *ni); 

/******************************************************************************
**  End of NetworkInterface structure region  *********************************
******************************************************************************/


/******************************************************************************
**  Packet structure **********************************************************
******************************************************************************/

// TODO: description
typedef enum _packet_t {
  PACK_OF_FLOATS,
  INIT_PACK
} packet_t;

typedef struct _Packet {
  packet_t type;
  unsigned int length;
  char *buf;
} Packet;

Packet *gen_Packet_from_floats(float *buf, size_t len);
Packet *gen_init_pack(float *buf, size_t len);
char *pack_to_bytes(Packet *pack);
size_t get_pack_bytes_size(Packet *pack);
Packet *bytes_to_pack(char *ca);
float *get_floats(Packet*);
void free_pack(Packet* pack_to_free);
size_t get_floats_count(Packet *pack);

/******************************************************************************
**  End of Packet structure region  *******************************************
******************************************************************************/


/******************************************************************************
**  ListenerPack structure  ***************************************************
******************************************************************************/

typedef struct _ListenerPack {
  int sd;
  pthread_mutex_t *batch_stock_mu;  
  BatchStock *batch_stock;
} ListenerPack;

ListenerPack *initialize_listener_pack(int sd, pthread_mutex_t *batch_stock_mu,
                                        BatchStock *bs);
void free_listener_pack(ListenerPack *lp);

/******************************************************************************
**  End of ListenerPac structure region  **************************************
******************************************************************************/

#endif
