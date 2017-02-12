#ifndef IO_STUFF_H
#define IO_STUFF_H

#include <stddef.h>
#include "network_interactions.h"

void write_to_file(char *file_name, char *buf_to_write, size_t bytes_num);

char *gen_file_name();

char *uint_to_bytes_arr(unsigned int);
unsigned int bytes_arr_to_uint(char *);

char *float_to_char_arr(float f);
float char_arr_to_float(char *ca);

char *pack_type_to_bytes(packet_t t);
packet_t char_arr_to_pack_type(char *ca);

float *vec_to_floats(float *vec, size_t len);

void print_pack(Packet *p);

char *float_arr_to_string(float *arr, size_t arr_len);

#endif
