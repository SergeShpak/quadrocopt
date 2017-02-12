#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "include/data_struct.h"

// Static functions

float *copy_arr_of_floats(float *arr, size_t arr_size);

// End of static functions region


/******************************************************************************
 * BatchStock structure *******************************************************
 *****************************************************************************/

// BatchStock static functions

void add_to_batch(float **dest, float *src, size_t batch_len);

// End of BatchStock static functions

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
  float *result = copy_arr_of_floats(bs->first_batch, bs->first_batch_len);
  *batch_len = bs->first_batch_len;
  return result;
}

float *get_second_stock_from_batch(BatchStock *bs, size_t *batch_len) {
  float *result = copy_arr_of_floats(bs->second_batch, bs->second_batch_len);
  *batch_len = bs->second_batch_len;
  return result;
}

void clean_batch_stock(BatchStock *bs) {
  free(bs->first_batch);
  bs->first_batch = NULL;
  bs->first_batch_len = 0;
  free(bs->second_batch);
  bs->second_batch = NULL;
  bs->second_batch_len = 0;
}

void free_batch_stock(BatchStock *bs) {
  free(bs->first_batch);
  free(bs->second_batch);
  free(bs);
}

// BatchStock Static functions

void add_to_batch(float **dest, float *src, size_t batch_len) {
  if (NULL != *dest) {
    free(*dest);
  }
  *dest = copy_arr_of_floats(src, batch_len);
}

// End of BatchStock static functions region

/******************************************************************************
 * End of BatchStock structure region *****************************************
 *****************************************************************************/


/******************************************************************************
 * CalculationsStock structure  ***********************************************
 *****************************************************************************/

CalculationsStock *initialize_calculations_stock() {
  CalculationsStock *cs = 
              (CalculationsStock *) malloc(sizeof(CalculationsStock));
  cs->calculations = NULL;
  cs->calculations_len = 0;
  return cs;
}

void add_to_calculations_stock(CalculationsStock *cs, float *calcs, 
                                size_t calcs_len) {
  cs->calculations = copy_arr_of_floats(calcs, calcs_len);
  cs->calculations_len = calcs_len;
}

float *get_calculations_from_stock(CalculationsStock *cs, size_t *calcs_len) {
  float *calcs_copy = copy_arr_of_floats(cs->calculations, 
                                          cs->calculations_len);
  return calcs_copy;
}

void clean_calculations_stock(CalculationsStock *cs) {
  free(cs->calculations);
  cs->calculations = NULL;
  cs->calculations_len = 0;
}

void free_calculations_stock(CalculationsStock *cs) {
  free(cs->calculations);
  free(cs);
}

/******************************************************************************
 * End of CalculationsStock structure region  *********************************
 *****************************************************************************/


// Static functions

float *copy_arr_of_floats(float *arr, size_t arr_size) {
  size_t copy_size = sizeof(float) * arr_size;
  float *copy = (float *) malloc(copy_size);
  memcpy(copy, arr, copy_size);
  return copy;
}

// End of static functions region
