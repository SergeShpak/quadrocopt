#ifndef DATA_STRUCT_H
#define DATA_STRUCT_H

#include <stddef.h>

typedef struct _BatchStock BatchStock;
typedef struct _CalculationsStock CalculationsStock;

struct _BatchStock {
  float *first_batch;
  size_t first_batch_len;
  float *second_batch;
  size_t second_batch_len;
};

struct _CalculationsStock {
  float *calculations;
  size_t calculations_len;
};

BatchStock *initialize_batch_stock();
void add_first_stock_to_batch(BatchStock *bs, float *batch, size_t batch_len);
void add_second_stock_to_batch(BatchStock *bs, float *batch, size_t batch_len);
float *get_first_stock_from_batch(BatchStock *bs, size_t *batch_len);
float *get_second_stock_from_batch(BatchStock *bs, size_t *batch_len);
void clean_batch_stock(BatchStock *bs);
void free_batch_stock(BatchStock *bs);


CalculationsStock *initialize_calculations_stock();
void add_to_calculations_stock(CalculationsStock *cs,
          float *calcs, size_t calcs_len);
float *get_calculations_from_stock(CalculationsStock *cs, size_t *calcs_len);
void clean_calculations_stock(CalculationsStock *cs);
void free_calculations_stock(CalculationsStock *cs);

#endif
