#ifndef IO_STUFF_H
#define IO_STUFF_H

#include <stddef.h>
#include "data_struct.h"

void write_to_file(char *file_name, char *buf_to_write, size_t bytes_num);

char *gen_file_name();

char *uint_to_bytes_arr(unsigned int);
unsigned int bytes_arr_to_uint(char *);

char *float_to_char_arr(float f);
float char_arr_to_float(char *ca);

char *pack_type_to_bytes(packet_t t);
packet_t char_arr_to_pack_type(char *ca);

float *vec_to_floats(float *vec, size_t len);

void print_pack(packet *p);

#endif
