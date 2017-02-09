#ifndef DATA_STRUCT_H
#define DATA_STRUCT_H

#include <stddef.h>

typedef enum _packet_t {
  PACK_OF_FLOATS,
  INIT_PACK
} packet_t;

typedef struct _packet {
  packet_t type;
  unsigned int length;
  char *buf;
} packet;

packet *gen_packet_from_floats(float *buf, size_t len);
packet *gen_init_pack(float *buf, size_t len);
char *pack_to_bytes(packet *pack);
size_t get_pack_bytes_size(packet *pack);
packet *bytes_to_pack(char *ca);
float *get_floats(packet*);
void free_pack(packet* pack_to_free);
size_t get_floats_count(packet *pack);

#endif
