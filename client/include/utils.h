#ifndef UTILS_H
#define UTILS_H

#include <pthread.h>
#include <stdarg.h>

typedef enum _ThreadId ThreadId;
typedef struct _CollectionList CollectionList;

enum _ThreadId {
  MAIN_THREAD,
  LISTENER_THREAD,
  SENDER_THREAD,
  PRINTER_THREAD
};

struct _CollectionList {
  void *el;
  CollectionList *next;
};

CollectionList *initialize_collection_list(void *el);
void add_to_collection_list(CollectionList *coll_list, void *el);
void free_collection_list(CollectionList *coll_list, 
                          void (*free_func)(void *el));

void exit_error(char *err_msg);
void safe_print(char *msg, pthread_mutex_t *mu);
int get_random_int(int low, int high);
void rand_sleep(int low, int high);
float *get_random_float_buf(size_t len);
float *copy_arr_of_floats(float *arr, size_t arr_size);

#endif
