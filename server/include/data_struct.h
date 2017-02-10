#ifndef DATA_STRUCT_H
#define DATA_STRUCT_H

#include <stddef.h>

typedef struct _BatchStock {
  float *first_batch;
  size_t first_batch_len;
  float *second_batch;
  size_t second_batch_len;
} BatchStock;

BatchStock *initialize_batch_stock();
void add_first_stock_to_batch(BatchStock *bs, float *batch, size_t batch_len);
void add_second_stock_to_batch(BatchStock *bs, float *batch, size_t batch_len);
float *get_first_stock_from_batch(BatchStock *bs, size_t *batch_len);
float *get_second_stock_from_batch(BatchStock *bs, size_t *batch_len);
void free_batch_stock(BatchStock *bs);

#endif
