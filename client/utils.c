#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "include/utils.h"

CollectionList *initialize_collection_list(void *el) {
  CollectionList *coll = (CollectionList *) malloc(sizeof(CollectionList));
  coll->next = NULL; 
  return coll;
}

void add_to_collection_list(CollectionList *coll_list, void *el) {
  if (NULL == coll_list->next) {
    CollectionList *new_tail = initialize_collection_list(el);
    coll_list->next = new_tail;
    return;
  }
  add_to_collection_list(coll_list->next, el);
}

void free_collection_list(CollectionList *coll_list, 
                          void (*free_func)(void *el)) {
  CollectionList *curr_node = coll_list;
  while(NULL != curr_node) {
    free_func(curr_node->el);
    curr_node = curr_node->next;
  }
  free(coll_list);
}


void exit_error(char *err_msg) {
  fprintf(stderr, "%s", err_msg);
  exit(1); 
}

void safe_print(char *msg, pthread_mutex_t *mu) {
  pthread_mutex_lock(mu);
  printf("%s", msg);
  pthread_mutex_unlock(mu);  
}

int get_random_int(int low, int high) {
  int r_int = rand() % (high - low) + low;
  return r_int;
}

void rand_sleep(int low, int high) {
  int period_rand = get_random_int(low, high);
  usleep(period_rand);
}

float *get_random_float_buf(size_t buf_len) {
  float *buf = (float *) malloc(sizeof(float) * buf_len);
  int i;
  for (i = 0; i < buf_len; i++) {
    float cur_float = ((float)rand()/(float)(RAND_MAX)) * 100;
    buf[i] = cur_float;
  }
  return buf;
}

float *copy_arr_of_floats(float *arr, size_t arr_size) {
  size_t copy_size = sizeof(float) * arr_size;
  float *copy = (float *) malloc(copy_size);
  memcpy(copy, arr, copy_size);
  return copy;
}
