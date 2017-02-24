#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include "include/network_interactions.h"

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
  int i;
  for (i = 0; i < 4; i++) {
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
  int i;
  for (i = 0; i < len; i++) {
    new_vec[i] = vec[i + 1];
  } 
  return new_vec;
}

void print_pack(Packet *p) {
  float *payload = get_floats(p);
  size_t floats_count = get_floats_count(p);
  int i;
  for (i = 0; i < floats_count; i++) {
    printf("%f ", payload[i]);
  }
}

char *float_arr_to_string(float *arr, size_t arr_len) {
  if ((NULL == arr) || (0 == arr_len)) {
    return "";       
  }
  char buf[100];
  size_t total_len = 0;
  char *float_strings[arr_len];
  float curr_float;
  int i;
  for (i = 0; i < arr_len; i++) {
    curr_float = arr[i];
    snprintf (buf, 100, "%f", curr_float);
    size_t fl_len = strlen(buf);
    total_len += (fl_len + 1);
    char *fl_repr = (char *) malloc(sizeof(char) * (fl_len + 1));
    memcpy(fl_repr, buf, fl_len);
    fl_repr[fl_len] = '\0';
    float_strings[i] = fl_repr;
  }
  char *curr_fl_repr;
  char *result = (char *) malloc(sizeof(char) * total_len);
  size_t curr_offset = 0;
  for (i = 0; i < arr_len; i++) {
    curr_fl_repr = float_strings[i];
    size_t fl_len = strlen(curr_fl_repr);
    memcpy((void *) result + curr_offset, (void *)curr_fl_repr, fl_len);
    if (i == 3) {
      i = 3;
    }
    result[curr_offset + fl_len] = ' ';
    curr_offset += fl_len + 1;
  }
  result[curr_offset - 1] = '\0';
  // TODO: why failing when freeing the last element??!
  for (i = 0; i < arr_len; i++) {
    free(float_strings[i]);
  }
  return result;
}
