#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>

#include "include/network_interactions.h"
#include "include/data_struct.h"
#include "include/listener.h"
#include "include/utils.h"
#include "include/constants.h"

pthread_mutex_t *batch_stock_first_mu = NULL;
pthread_mutex_t *batch_stock_second_mu = NULL;
pthread_mutex_t *io_mu = NULL;
pthread_mutex_t *calc_batch_first_mu = NULL;
pthread_mutex_t *calc_batch_second_mu = NULL;
pthread_cond_t *calc_read_cond = NULL;
pthread_cond_t *first_listener_wrote_cond = NULL;
pthread_cond_t *second_listener_wrote_cond = NULL;
NetworkInterface *net_interface = NULL;
BatchStock *bs = NULL;
pthread_t *listener_first = NULL, *listener_second = NULL, *sender = NULL;
pthread_barrier_t *listeners_barrier = NULL;
char listeners_count = 0;
pthread_mutex_t listeners_count_mutex;
pthread_cond_t all_listeners_ready;
FILE *log_file = NULL;

// Static functions declarations

void initialize_globals();
void initialize_mutexes();
void initialize_condition_vars();
void initialize_barriers();
pthread_cond_t *init_cond_var(pthread_condattr_t *attr);
pthread_mutex_t *init_mutex(pthread_mutexattr_t *mu_attr);
void free_mutexes();
void free_mutex(pthread_mutex_t*);
void tear_down();
void *run_listener_callback(void*);
void create_listeners();
int start_listener(listener_t type, pthread_cond_t *calc_read, 
              pthread_cond_t *listener_wrote, pthread_mutex_t *batch_stock_mu, 
                                pthread_mutex_t *calc_batch_mu, int sd, 
                                pthread_t *listener_thread);
void create_sender();
void spawn_workers();
void lock_mutexes();
void wait_listeners_write();
void read_batches();
void signal_read();

// End of static functions declarations

int main(int argc, char **argv) {
  log_file = fopen("log", "w");
  initialize_globals();
  lock_mutexes();
  spawn_workers();
   
  while(1) {
    wait_listeners_write();
    read_batches();
    signal_read();     
  }

  tear_down();
  return 0;
}

void initialize_globals() {
  net_interface = initialize_network_interface();
  bs = initialize_batch_stock();
  initialize_mutexes();
  initialize_barriers();
}

void initialize_mutexes() {
  batch_stock_first_mu = init_mutex(NULL);
  batch_stock_second_mu = init_mutex(NULL);
  calc_batch_first_mu = init_mutex(NULL);
  calc_batch_second_mu = init_mutex(NULL);
  io_mu = init_mutex(NULL);
}

void initialize_barriers() {
  pthread_barrierattr_t attr;
  pthread_barrierattr_init(&attr);
  listeners_barrier = (pthread_barrier_t *) malloc(sizeof(pthread_barrier_t));
  pthread_barrier_init(listeners_barrier, &attr, 
                        SERVER_NUMBER_OF_LISTENERS + 1);
}

void initialize_condition_vars() {
  calc_read_cond = init_cond_var(NULL);
  first_listener_wrote_cond = init_cond_var(NULL);
  second_listener_wrote_cond = init_cond_var(NULL);
}

pthread_mutex_t *init_mutex(pthread_mutexattr_t *mu_attr) {
  pthread_mutex_t *mu = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(mu, mu_attr);
  return mu;
}

pthread_cond_t *init_cond_var(pthread_condattr_t *attr) {
  pthread_cond_t *cond_var = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
  pthread_cond_init(cond_var, attr);
  return cond_var;
}

void tear_down() {
  free_batch_stock(bs);
  free_network_interface(net_interface);
  free_mutexes();
}

void free_mutexes() {
  free_mutex(batch_stock_first_mu);
  free_mutex(batch_stock_second_mu);
}

void free_mutex(pthread_mutex_t *mu) {
  if (NULL != mu) {
    pthread_mutex_destroy(mu);
  }
  free(mu);
}

void spawn_workers() {
  create_listeners();
  create_sender();
}

void create_listeners() { 
  int status;
  listener_first = (pthread_t *) malloc(sizeof(pthread_t));
  status = start_listener(LISTENER_FIRST, calc_read_cond, 
      first_listener_wrote_cond, batch_stock_first_mu, 
      calc_batch_first_mu, net_interface->sd_in_first, listener_first);
  if (status) {
    exit_error("Could not start the first listener.");
  }
  listener_second = (pthread_t *) malloc(sizeof(pthread_t));
  status = start_listener(LISTENER_SECOND, calc_read_cond, 
      second_listener_wrote_cond, batch_stock_second_mu, calc_batch_second_mu, 
      net_interface->sd_in_second, listener_second);
  if (status) {
    exit_error("Could not start the second listener");
  }
  pthread_barrier_wait(listeners_barrier);
//  // MUTEX: listeners_count_mutex
//  // waiting while all listeners lock batch store mutex
//  pthread_mutex_lock(&listeners_count_mutex);
//  pthread_cond_wait(&all_listeners_ready, &listeners_count_mutex);
//  pthread_mutex_unlock(&listeners_count_mutex);
//  // MUTEX UNLOCKED: listeners_count_mutex
}

int start_listener(listener_t type, pthread_cond_t *calc_read, 
          pthread_cond_t *listener_wrote, pthread_mutex_t *batch_stock_mu, 
          pthread_mutex_t *calc_batch_mu, int sd, pthread_t *listener_thread) {
  int status;
  ListenerMutexSet *lms = create_listener_mutex_set(calc_read, listener_wrote, 
                                        batch_stock_mu, calc_batch_mu, io_mu);
  ListenerPack *lp = initialize_listener_pack(type, sd, lms, bs, log_file);
  listener_first = (pthread_t *) malloc(sizeof(pthread_t));
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  status = pthread_create(listener_thread, &attr, &run_listener_callback,
                          (void *) lp);
  return status;
}

void create_sender() {
}

void *run_listener_callback(void *lp) {
  ListenerPack *listener_pack = (ListenerPack *) lp;
  pthread_barrier_wait(listeners_barrier);
  //pthread_mutex_lock(listener_pack->mu_set->batch_stock_mu);
//  // MUTEX: listeners_count_mutex
//  // incrementing the number of activated listeners
//  pthread_mutex_lock(&listeners_count_mutex);
//  if (SERVER_NUMBER_OF_LISTENERS - 1 == listeners_count) {
//    pthread_cond_signal(&all_listeners_ready);
//  }
//  listeners_count++;
//  pthread_mutex_unlock(&listeners_count_mutex);
//  // MUTEX UNLOCKED: listeners_count_mutex
  
  run_listener((ListenerPack *) lp);
  return NULL;
}

void lock_mutexes() {
}

void wait_listeners_write() {
  pthread_mutex_lock(batch_stock_first_mu);
  pthread_mutex_lock(batch_stock_second_mu);
  pthread_mutex_unlock(batch_stock_first_mu);
  pthread_mutex_unlock(batch_stock_second_mu);
}

void signal_read() {
  pthread_mutex_lock(calc_batch_first_mu);
  pthread_mutex_lock(calc_batch_second_mu);
  pthread_mutex_unlock(calc_batch_first_mu);
  pthread_mutex_unlock(calc_batch_second_mu);
  pthread_cond_signal(calc_read_cond);
}

void read_batches() {
  size_t first_batch_len;
  float *first_batch = get_first_stock_from_batch(bs, &first_batch_len);
  size_t second_batch_len;
  float *second_batch = get_second_stock_from_batch(bs, &second_batch_len);  
  printf("First batch: ");
  for (int i = 0; i < first_batch_len; i++) {
    printf("%f ", first_batch[i]);
  }
  printf("\nSecond batch: ");
  for(int i = 0; i < second_batch_len; i++) {
    printf("%f ", second_batch[i]);
  }
  free(first_batch);
  free(second_batch);
}
