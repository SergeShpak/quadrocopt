#ifndef UTILS_H
#define UTILS_H

#include <pthread.h>
#include <stdarg.h>

void exit_error(char *err_msg);
void safe_print(char *msg, pthread_mutex_t *mu);

#endif
