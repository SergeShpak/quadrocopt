#ifndef UTILS_H
#define UTILS_H

#include <pthread.h>
#include <stdarg.h>

void exit_error(char *err_msg);
void safe_print(char *msg, pthread_mutex_t *mu);
int get_random_int(int low, int high);
void rand_sleep(int low, int high);
float *get_random_float_buf(size_t len);

#endif
