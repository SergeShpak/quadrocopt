#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "include/data_struct.h"

char *uint_to_bytes_arr(unsigned int repr);
unsigned int bytes_arr_to_uint(char *ca);

void write_to_file(char *file_name, char *buf_to_write, size_t bytes_num) {
  FILE *fp = fopen(file_name, "a+");
  fwrite(buf_to_write, sizeof(char), bytes_num, fp);
  fclose(fp);
}

char *gen_file_name() {
  time_t t = time(NULL);
  char buf[255];
  char *t_repr = ctime(&t);
  sprintf(buf, "./log/REC_%s", t_repr);
  char *p = buf;
  for (; *p; ++p) {
    if (*p == ' ') {
      *p = '_';
    }
  }
  size_t res_len = strlen(buf);
  char *res = (char *) malloc(sizeof(char) * (res_len + 1));
  strncpy(res, buf, (res_len - 6));
  res[res_len] = '\0';
  return res;
}

char *float_to_char_arr(float f) {
  unsigned int int_repr = *((int*)&f);
  char *result = uint_to_bytes_arr(int_repr);
  return result;
}

float char_arr_to_float(char *ca) {
  unsigned int ui_result = bytes_arr_to_uint(ca);
  float result = *((float*)&ui_result);
  return result;
}

char *uint_to_bytes_arr(unsigned int int_repr) {
  char *result = (char *) malloc(sizeof(char) * 4);
  int i;
  for (i = 0; i < 4; i++) {
    result[i] = (int_repr >> (8 * i)) & 0xFF; 
  }
  return result;
}

unsigned int bytes_arr_to_uint(char *ca) {
  unsigned int result = 0;
  for (int i = 0; i < 4; i++) {
    result |= ((ca[i] << (8 * i)) & (0xff << (8 * i)));
  }
    return result;
}

char *pack_type_to_bytes(packet_t t) {
  unsigned int ui_repr = (unsigned int) t;
  return uint_to_bytes_arr(ui_repr); 
}

packet_t char_arr_to_pack_type(char *ca) {
  unsigned int ui_repr = bytes_arr_to_uint(ca);
  packet_t t = (packet_t) ui_repr;
  return t;
}

float *vec_to_floats(float *vec, size_t len) {
  float *new_vec = (float*) malloc(sizeof(float) * len);
  for (int i = 0; i < len; i++) {
    new_vec[i] = vec[i + 1];
  } 
  return new_vec;
}

void print_pack(packet *p) {
  float *payload = get_floats(p);
  size_t floats_count = get_floats_count(p);
  for (int i = 0; i < floats_count; i++) {
    printf("%f ", payload[i]);
  }
}
