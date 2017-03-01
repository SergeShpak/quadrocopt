#ifndef DATA_STRUCT_H
#define DATA_STRUCT_H

#include <stddef.h>

#include "printer.h"

typedef struct _BatchStock BatchStock;
typedef struct _SenderStock SenderStock;
typedef struct _PrinterParamsCollection PrinterParamsCollection;

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

#endif
