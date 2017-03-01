#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "include/data_struct.h"
#include "include/utils.h"

/******************************************************************************
 * BatchStock structure  ***********************************************
 *****************************************************************************/

BatchStock *initialize_batch_stock() {
  BatchStock *bs = (BatchStock *) malloc(sizeof(BatchStock));
  bs->batch = NULL;
  bs->batch_len = 0;
  return bs;
}

void add_to_batch_stock(BatchStock *bs, float *batch, size_t batch_len) {
  bs->batch = copy_arr_of_floats(batch, batch_len);
  bs->batch_len = batch_len;
}

float *get_batch_from_stock(BatchStock *bs, size_t *batch_len) {
  float *batch_copy = copy_arr_of_floats(bs->batch, bs->batch_len);
  return batch_copy;
}

void clean_batch_stock(BatchStock *bs) {
  free(bs->batch);
  bs->batch = NULL;
  bs->batch_len = 0;
}

void free_batch_stock(BatchStock *bs) {
  free(bs->batch);
  free(bs);
}

/******************************************************************************
 * End of BatchStock structure region  *********************************
 *****************************************************************************/

SenderStock *initialize_sender_stock() {
  SenderStock *s = (SenderStock *) malloc(sizeof(SenderStock));
  s->first_batch = NULL;
  s->first_batch_len = 0;
  s->second_batch = NULL;
  s->second_batch_len = 0;
  return s;
}

void add_first_batch_to_sender_stock(SenderStock *s, 
                                      float *batch, size_t batch_len) {
  s->first_batch = copy_arr_of_floats(batch, batch_len);
  s->first_batch_len = batch_len;
}

void add_second_batch_to_sender_stock(SenderStock *s, 
                                      float *batch, size_t batch_len) {
  s->second_batch = copy_arr_of_floats(batch, batch_len);
  s->second_batch_len = batch_len;
}

void clean_sender_stock(SenderStock *s) {
  free(s->first_batch);
  s->first_batch_len = 0;
  free(s->second_batch);
  s->second_batch_len = 0;
}

void free_sender_stock(SenderStock *s) {
  clean_sender_stock(s);
  free(s);
}


PrinterParamsCollection *initialize_printer_params_collection(
                                          PrinterParameters *results_params) {
  PrinterParamsCollection *coll = 
          (PrinterParamsCollection *) malloc(sizeof(PrinterParamsCollection));
  coll->results_params = results_params;
  return coll;
}

void free_printer_params_collection(PrinterParamsCollection *coll) {
  free_printer_params(coll->results_params);
  free(coll);
}
