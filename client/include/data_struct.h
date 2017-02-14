#ifndef DATA_STRUCT_H
#define DATA_STRUCT_H

#include <stddef.h>

typedef struct _CalculationsStock CalculationsStock;

struct _CalculationsStock {
  float *calculations;
  size_t calculations_len;
};

CalculationsStock *initialize_calculations_stock();
void add_to_calculations_stock(CalculationsStock *cs,
          float *calcs, size_t calcs_len);
float *get_calculations_from_stock(CalculationsStock *cs, size_t *calcs_len);
void clean_calculations_stock(CalculationsStock *cs);
void free_calculations_stock(CalculationsStock *cs);

#endif
