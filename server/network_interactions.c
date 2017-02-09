#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#include "include/network_interactions.h"
#include "include/utils.h"
#include "include/constants.h"
#include "include/io_stuff.h"

/******************************************************************************
**  NetworkInterface structure  ***********************************************
******************************************************************************/

void bind_sockaddr(int port, struct sockaddr_in*, int *sd);


NetworkInterface *initialize_network_interface() {
  NetworkInterface *interf = 
                (NetworkInterface*) malloc(sizeof(NetworkInterface)); 
  interf->sd_in_first = socket(PF_INET, SOCK_DGRAM, 0);
  interf->sd_in_second = socket(PF_INET, SOCK_DGRAM, 0);
  interf->sd_out = socket(PF_INET, SOCK_DGRAM, 0);
  if (interf->sd_in_first < 0 || interf->sd_in_second < 0 
                              || interf->sd_out < 0) {
    exit_error("Problem creating sockets\n");
  }
  bind_sockaddr(SERVER_IN_FIRST_PORT, 
                &(interf->skaddr_in_first), &(interf->sd_in_first));
  bind_sockaddr(SERVER_IN_SECOND_PORT,
                &(interf->skaddr_in_second), &(interf->sd_in_second));
  bind_sockaddr(SERVER_OUT_PORT,
                &(interf->skaddr_out), &(interf->sd_out));
  return interf;
}

void bind_sockaddr(int port, struct sockaddr_in *skaddr, int *sd) {
  skaddr->sin_family = AF_INET;
  (skaddr->sin_addr).s_addr = htonl(INADDR_ANY);
  skaddr->sin_port = htons((uint16_t) port);
  int bind_status = bind(*sd, (struct sockaddr *) skaddr, sizeof(*skaddr)); 
  if (bind_status < 0) {
    exit_error("Error getting socket name\n");
  }
}

void free_network_interface(NetworkInterface *ni) {
  if (NULL == ni) {
    return;
  }
  free(ni);
}

/******************************************************************************
**  End of NetworkInterface structure region  *********************************
******************************************************************************/


/******************************************************************************
**  Packet structure **********************************************************
******************************************************************************/

char *float_pack_to_bytes(Packet *pack);
void populate_pack_of_floats(char *ca, Packet *pack);
char *get_slice(char *buf, size_t start, size_t end);
Packet *fill_packet(float *buf, size_t len);

Packet *fill_packet(float *buf, size_t len) {
  float current_float;
  size_t offset = 0;
  Packet *pack = (Packet*) malloc(sizeof(Packet)); 
  pack->length = len * sizeof(float);
  pack->buf = (char *) malloc(sizeof(char) * sizeof(float) * len);
  int i;
  for (i = 0; i < len; i++) {
    current_float = buf[i]; 
    char *float_repr = float_to_char_arr(current_float);
    memcpy((pack->buf) + offset, float_repr, sizeof(float));
    offset += 4;
    free(float_repr);
  }
  return pack;
}

Packet *gen_packet_from_floats(float *buf, size_t len) {
  Packet *pack = fill_packet(buf, len);
  pack->type = PACK_OF_FLOATS;
  return pack;
}

Packet *gen_init_pack(float *buf, size_t len) {
  Packet *pack = fill_packet(buf, len);
  pack->type = INIT_PACK;
  return pack;
}

char *pack_to_bytes(Packet *pack) {
  switch(pack->type) {
    case PACK_OF_FLOATS:
    case INIT_PACK:
      return float_pack_to_bytes(pack);
      break;
    default:
      exit_error("Unknown type of packet received");
      exit(1);
  }  
}

char *float_pack_to_bytes(Packet *pack) {
  size_t result_len = sizeof(unsigned int) * 2 + pack->length;
  char *buf = (char*) malloc(sizeof(char) * result_len);
  char *t = pack_type_to_bytes(pack->type);
  memcpy(buf, t, sizeof(unsigned int));
  char *len = uint_to_bytes_arr(pack->length);
  memcpy(buf + sizeof(unsigned int), len, sizeof(unsigned int));
  memcpy(buf + (sizeof(unsigned int) * 2), pack->buf, pack->length);
  free(t);
  free(len);
  return buf; 
}

size_t get_pack_bytes_size(Packet *pack) {
  unsigned int size = sizeof(unsigned int) * 2 + pack->length;
  return (size_t) size; 
}

Packet *bytes_to_pack(char *ca) {
  char *t_arr = (char *) malloc(sizeof(char) * sizeof(unsigned int));
  memcpy(t_arr, ca, sizeof(unsigned int));
  packet_t t = char_arr_to_pack_type(t_arr);
  free(t_arr);
  Packet *pack = (Packet *) malloc(sizeof(Packet) * 1);
  switch (t) {
    case INIT_PACK:
    case PACK_OF_FLOATS:
      pack->type = t;
      populate_pack_of_floats(ca, pack);
      break;
    default:
      exit(1);
  }
  return pack;
}

void populate_pack_of_floats(char *ca, Packet *pack) {
  char *len = (char *) malloc (sizeof(char) * sizeof(unsigned int));
  memcpy(len, ca + sizeof(unsigned int), sizeof(unsigned int));
  unsigned int p_len = bytes_arr_to_uint(len);
  pack->length = p_len;
  pack->buf = (char*) malloc (sizeof(char) * p_len);
  memcpy(pack->buf, ca + sizeof(unsigned int) * 2, p_len);
}

float *get_floats(Packet *pack) {
  unsigned int floats_count = 
    (unsigned int) (pack->length / (sizeof(unsigned int)));
  float *result = (float*) malloc(sizeof(float) * floats_count);
  int i;
  size_t offset = 0;
  for (i = 0; i < floats_count; i++) {
    char *slice = get_slice(pack->buf, offset, offset + sizeof(unsigned int)); 
    result[i] = char_arr_to_float(slice);
    free(slice);
    offset += 4;
  } 
  return result;
}

size_t get_floats_count(Packet *pack) {
  if (PACK_OF_FLOATS != pack->type) {
    return -1;
  }
  return (size_t) (pack->length / sizeof(float));
}

char *get_slice(char *buf, size_t start, size_t end) {
  size_t diff = end - start;
  char *slice = (char *) malloc(sizeof(char) * diff);
  memcpy(slice, buf + start, diff);
  return slice;
}

void free_pack(Packet *pack) {
  if (NULL != pack->buf) {
    free(pack->buf);
  }
  free(pack);
}

/******************************************************************************
**  End of Packet structure region  *******************************************
******************************************************************************/


/******************************************************************************
**  ListenerPack structure  ***************************************************
******************************************************************************/

ListenerPack *initialize_listener_pack(int sd, pthread_mutex_t *batch_stock_mu,
                                        BatchStock *bs) {
  ListenerPack *lp = (ListenerPack *) malloc(sizeof(ListenerPack));
  lp->sd = sd;
  lp->batch_stock_mu = batch_stock_mu;
  lp->batch_stock = bs;
  return lp;
}

void free_listener_pack(ListenerPack *lp) {
  free(lp);
}

/******************************************************************************
**  End of ListenerPac structure region  **************************************
******************************************************************************/


