#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "include/data_struct.h"

void add_to_batch(float **dest, float *src, size_t batch_len);
float *copy_batch(float *batch, size_t batch_len);

BatchStock *initialize_batch_stock() {
  BatchStock *bs = (BatchStock *) malloc(sizeof(BatchStock));
  bs->first_batch = NULL;
  bs->second_batch = NULL;
  bs->first_batch_len = 0;
  bs->second_batch_len = 0;
  return bs;
}

void add_first_stock_to_batch(BatchStock *bs, float *batch, size_t batch_len) {
  add_to_batch(&(bs->first_batch), batch, batch_len);
  bs->first_batch_len = batch_len;
}

void add_second_stock_to_batch(BatchStock *bs, float *batch, 
                                size_t batch_len) {
  add_to_batch(&(bs->second_batch), batch, batch_len);
  bs->second_batch_len = batch_len;
}

float *get_first_stock_from_batch(BatchStock *bs, size_t *batch_len) {
  float *result = copy_batch(bs->first_batch, bs->first_batch_len);
  *batch_len = bs->first_batch_len;
  return result;
}

float *get_second_stock_from_batch(BatchStock *bs, size_t *batch_len) {
  float *result = copy_batch(bs->second_batch, bs->second_batch_len);
  *batch_len = bs->second_batch_len;
  return result;
}

void free_batch_stock(BatchStock *bs) {
  free(bs->first_batch);
  free(bs->second_batch);
  free(bs);
}

// Static functions

void add_to_batch(float **dest, float *src, size_t batch_len) {
  if (NULL != *dest) {
    free(*dest);
  }
  *dest = copy_batch(src, batch_len);
}

float *copy_batch(float *batch, size_t batch_len) {
  float *copy = (float *) malloc(sizeof(float) * batch_len);
  memcpy((void*) copy, (void *) batch, batch_len * sizeof(float));
  return copy; 
}
