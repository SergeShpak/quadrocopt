#include <stdlib.h>
#include <string.h>

#include "include/data_struct.h"
#include "include/io_stuff.h"

char *float_pack_to_bytes(packet *pack);
void populate_pack_of_floats(char *ca, packet *pack);
char *get_slice(char *buf, size_t start, size_t end);
packet *fill_packet(float *buf, size_t len);

packet *fill_packet(float *buf, size_t len) {
  float current_float;
  size_t offset = 0;
  packet *pack = (packet*) malloc(sizeof(packet)); 
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

packet *gen_packet_from_floats(float *buf, size_t len) {
  packet *pack = fill_packet(buf, len);
  pack->type = PACK_OF_FLOATS;
  return pack;
}

packet *gen_init_pack(float *buf, size_t len) {
  packet *pack = fill_packet(buf, len);
  pack->type = INIT_PACK;
  return pack;
}

char *pack_to_bytes(packet *pack) {
  switch(pack->type) {
    case PACK_OF_FLOATS:
    case INIT_PACK:
      return float_pack_to_bytes(pack);
      break;
    default:
      // TODO: add error
      exit(1);
  }  
}

char *float_pack_to_bytes(packet *pack) {
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

size_t get_pack_bytes_size(packet *pack) {
  unsigned int size = sizeof(unsigned int) * 2 + pack->length;
  return (size_t) size; 
}

packet *bytes_to_pack(char *ca) {
  char *t_arr = (char *) malloc(sizeof(char) * sizeof(unsigned int));
  memcpy(t_arr, ca, sizeof(unsigned int));
  packet_t t = char_arr_to_pack_type(t_arr);
  free(t_arr);
  packet *pack = (packet *) malloc(sizeof(packet) * 1);
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

void populate_pack_of_floats(char *ca, packet *pack) {
  char *len = (char *) malloc (sizeof(char) * sizeof(unsigned int));
  memcpy(len, ca + sizeof(unsigned int), sizeof(unsigned int));
  unsigned int p_len = bytes_arr_to_uint(len);
  pack->length = p_len;
  pack->buf = (char*) malloc (sizeof(char) * p_len);
  memcpy(pack->buf, ca + sizeof(unsigned int) * 2, p_len);
}

float *get_floats(packet *pack) {
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

size_t get_floats_count(packet *pack) {
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

void free_pack(packet *pack) {
  if (NULL != pack->buf) {
    free(pack->buf);
  }
  free(pack);
}
