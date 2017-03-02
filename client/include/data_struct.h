#ifndef DATA_STRUCT_H
#define DATA_STRUCT_H

#include <stddef.h>

#include "printer.h"
#include "utils.h"

typedef struct _BatchStock BatchStock;
typedef struct _SenderStock SenderStock;
typedef struct _PrinterParamsCollection PrinterParamsCollection;
typedef struct _ThreadConditionPacksCollection ThreadConditionPacksCollection;
typedef struct _WorkersCollection WorkersCollection;


struct _BatchStock {
  float *batch;
  size_t batch_len;
};

BatchStock *initialize_batch_stock();
void add_to_batch_stock(BatchStock *bs, float *batch, size_t batch_len);
float *get_batch_from_stock(BatchStock *bs, size_t *calcs_len);
void clean_batch_stock(BatchStock *bs);
void free_batch_stock(BatchStock *bs);


struct _SenderStock {
  float *first_batch;
  size_t first_batch_len;
  float *second_batch;
  size_t second_batch_len;
};

SenderStock *initialize_sender_stock();
void add_first_batch_to_sender_stock(SenderStock *s, 
                                      float *batch, size_t batch_len);
void add_second_batch_to_sender_stock(SenderStock *s, 
                                      float *batch, size_t batch_len);
void clean_sender_stock(SenderStock *s);
void free_sender_stock(SenderStock *s);


struct _PrinterParamsCollection {
  PrinterParameters *results_params; 
};

PrinterParamsCollection *initialize_printer_params_collection(
                                          PrinterParameters *results_params);
void free_printer_params_collection(PrinterParamsCollection *collection);


struct _ThreadConditionPacksCollection {
  ThreadConditionPack *sender_from_signal;
  ThreadConditionPack *sender_to_signal;
  ThreadConditionPack *listener_to_signal;
  ThreadConditionPack *listener_from_signal;
  ThreadConditionPack *printer_from_signal;
  ThreadConditionPack *printer_to_signal;
};

ThreadConditionPacksCollection *initialize_thread_cond_packs_collection(
                                    ThreadConditionPack *sender_to_signal,
                                    ThreadConditionPack *sender_from_signal,
                                    ThreadConditionPack *listener_to_signal,
                                    ThreadConditionPack *listener_from_signal,
                                    ThreadConditionPack *printer_to_signal,
                                    ThreadConditionPack *printer_from_signal);
void free_thread_cond_packs_collection(
                                  ThreadConditionPacksCollection *collection);


struct _WorkersCollection {
  pthread_t *listener;
  pthread_t *sender;
  pthread_t *printer;  
  CollectionList *workers;  
};

WorkersCollection *initialize_workers_collection();
int add_to_workers_collection(pthread_t *worker, WorkersCollection *coll, 
                              pthread_t **dst);
void free_workers_collection(WorkersCollection *collection);

#endif
