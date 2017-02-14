#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "include/data_struct.h"

// Static functions

float *copy_arr_of_floats(float *arr, size_t arr_size);

// End of static functions region

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
