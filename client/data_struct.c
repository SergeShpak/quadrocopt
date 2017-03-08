#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "include/constants.h"
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


ThreadConditionPacksCollection *initialize_thread_cond_packs_collection (
                                    ThreadConditionPack *sender_to_signal,
                                    ThreadConditionPack *sender_from_signal,
                                    ThreadConditionPack *listener_to_signal,
                                    ThreadConditionPack *listener_from_signal,
                                    ThreadConditionPack *printer_to_signal,
                                    ThreadConditionPack *printer_from_signal) {
  ThreadConditionPacksCollection *collection = 
                                  (ThreadConditionPacksCollection *) malloc(
                                      sizeof(ThreadConditionPacksCollection));
  collection->sender_to_signal = sender_to_signal;
  collection->sender_from_signal = sender_from_signal;
  collection->listener_from_signal = listener_from_signal;
  collection->listener_to_signal = listener_to_signal;
  collection->printer_to_signal = printer_to_signal;
  collection->printer_from_signal = printer_from_signal;
  return collection;
}

void free_thread_cond_packs_collection(
                                ThreadConditionPacksCollection *collection) {
  free_thread_cond_pack(collection->sender_to_signal);
  free_thread_cond_pack(collection->sender_from_signal);
  free_thread_cond_pack(collection->listener_to_signal);
  free_thread_cond_pack(collection->listener_from_signal);
  free_thread_cond_pack(collection->printer_to_signal);
  free_thread_cond_pack(collection->printer_from_signal);
  free(collection);
}


WorkersCollection *initialize_workers_collection() {
  WorkersCollection *collection = 
                      (WorkersCollection *) malloc(sizeof(WorkersCollection));
  collection->listener = NULL;
  collection->sender = NULL;
  collection->printer = NULL;
  collection->workers = NULL; 
  return collection;
}

int add_to_workers_collection(pthread_t *worker, WorkersCollection *coll, 
                              pthread_t **dst) {
  if (NULL != *dst) {
    return -1;
  }
  *dst = worker;
  SimpleNode *worker_node = initialize_simple_node(worker);
  if (NULL == coll->workers) {
    coll->workers = initialize_simple_linked_list(worker_node);
    return 0; 
  }
  add_to_simple_linked_list(coll->workers, worker_node);
  return 0;
}

void free_workers_collection(WorkersCollection *collection) {
  free_simple_linked_list(collection->workers, free);
  free(collection);
}   
