#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <unistd.h>

#include "include/network_interactions.h"
#include "include/calculate.h"
#include "include/data_struct.h"
#include "include/listener.h"
#include "include/sender.h"
#include "include/utils.h"
#include "include/constants.h"
#include "include/threading_stuff.h"
#include "include/io_stuff.h"

// mutex to lock on BatchStock access
pthread_mutex_t *batch_stock_access_mu = NULL;  
pthread_mutex_t *io_mu = NULL;
pthread_mutex_t *client_addr_mu = NULL; //TODO: get rid of this mutex
ThreadConditionPack *listener_one_signal = NULL;
ThreadConditionPack *listener_two_signal = NULL;
ThreadConditionPack *calc_signal_to_listener_one = NULL;
ThreadConditionPack *calc_signal_to_listener_two = NULL;
ClientAddress *client_addr = NULL;
NetworkInterface *net_interface = NULL;
BatchStock *bs = NULL;
CalculationsStock *cs = NULL;
pthread_t *listener_first = NULL, *listener_second = NULL, *sender = NULL;
pthread_barrier_t *init_barrier = NULL;
float *first_batch;
size_t first_batch_len;
float *second_batch;
size_t second_batch_len;
float *calculated;

ThreadConditionPack *calc_to_sender_signal;
ThreadConditionPack *sender_signal;

// Static functions declarations

void initialize_globals();
void initialize_mutexes();
ThreadConditionPack *initialize_thread_cond_pack(int init_verif_var);
void initialize_barriers();
void initialize_condition_packs();
pthread_cond_t *init_cond_var(pthread_condattr_t *attr);
pthread_mutex_t *init_mutex(pthread_mutexattr_t *mu_attr);
void free_mutexes();
void free_mutex(pthread_mutex_t*);
void tear_down();
void *run_listener_callback(void*);
void *run_sender_callback(void*);
void create_listeners();
int start_listener(listener_t type, 
                    pthread_mutex_t *batch_stock_access_mu, 
                    ThreadConditionPack *full_signal,
                    ThreadConditionPack *calc_empty_signal, 
                    int sd, 
                    pthread_t *listener_thread);
void spawn_workers();
void wait_listeners_write();
void wait_listener(ThreadConditionPack *listener_signal);
void read_batches();
void signal_read();
void signal_listener(ThreadConditionPack *calc_to_listener_signal);

int start_sender();
void create_sender();
void calculate_res();
void signal_calculated();
void wait_for_sender();

// End of static functions declarations

int main(int argc, char **argv) {
  initialize_globals();
  spawn_workers();
  int rounds = 0;
  while(1) {
    rounds++;
    pthread_mutex_lock(io_mu);
    printf("[CALCULATOR]: round %d\n", rounds);
    pthread_mutex_unlock(io_mu);
    wait_listeners_write();
    read_batches();
    safe_print("[CALCULATOR]: batches read\n", io_mu);
    rand_sleep(0, 1500 * 1000);
    signal_read();
     
    calculate_res();
    safe_print("[CALCULATOR]: results calculated\n", io_mu);
    wait_for_sender();
    signal_calculated();
  }

  tear_down();
  return 0;
}

void initialize_globals() {
  net_interface = initialize_network_interface();
  client_addr = initialize_client_address();
  bs = initialize_batch_stock();
  cs = initialize_calculations_stock();
  initialize_mutexes();
  initialize_barriers();
  initialize_condition_packs();
}

void initialize_mutexes() {
  batch_stock_access_mu = init_mutex(NULL);
  client_addr_mu = init_mutex(NULL);
  io_mu = init_mutex(NULL);
}

void initialize_barriers() {
  pthread_barrierattr_t attr;
  pthread_barrierattr_init(&attr);
  init_barrier = (pthread_barrier_t *) malloc(sizeof(pthread_barrier_t));
  pthread_barrier_init(init_barrier, &attr, 
                    SERVER_NUMBER_OF_LISTENERS + SERVER_NUMBER_OF_SENDERS + 1);
}

void initialize_condition_packs() {
  listener_one_signal = initialize_thread_cond_pack(0);
  listener_two_signal = initialize_thread_cond_pack(0);
  calc_signal_to_listener_one = initialize_thread_cond_pack(1);
  calc_signal_to_listener_two = initialize_thread_cond_pack(1);
  calc_to_sender_signal = initialize_thread_cond_pack(0);
  sender_signal = initialize_thread_cond_pack(1);
}

ThreadConditionPack *initialize_thread_cond_pack(int init_verif_var) {
  pthread_cond_t *cond_var = init_cond_var(NULL);
  pthread_mutex_t *cond_mu = init_mutex(NULL);
  ThreadConditionPack *pack = init_thread_cond_pack(cond_var, cond_mu);
  if (!init_verif_var) {
    set_cond_to_verify_to_false(pack);
    return pack;
  }
  set_cond_to_verify_to_true(pack);
  return pack;
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
  pthread_barrier_wait(init_barrier);
}

void create_listeners() { 
  int status;
  status = start_listener(LISTENER_FIRST, batch_stock_access_mu,
                          listener_one_signal, calc_signal_to_listener_one, 
                          net_interface->sd_in_first, listener_first);
  if (status) {
    exit_error("Could not start the first listener.");
  }
  status = start_listener(LISTENER_SECOND, batch_stock_access_mu,
                          listener_two_signal, calc_signal_to_listener_two,
                          net_interface->sd_in_second, listener_second);
  if (status) {
    exit_error("Could not start the second listener");
  }
}

int start_listener(listener_t type, pthread_mutex_t *batch_stock_access_mu, 
                  ThreadConditionPack *listener_signal, 
                  ThreadConditionPack *calc_to_listener_signal,
                  int sd, pthread_t *listener_thread) {
  int status;
  ListenerMutexSet *lms = 
              create_listener_mutex_set(batch_stock_access_mu, 
                                        client_addr_mu, 
                                        io_mu);
  ListenerThreadCondPackets *ltcp = initialize_cond_packs(listener_signal, 
                                                      calc_to_listener_signal);
  ListenerPack *lp = initialize_listener_pack(type, sd, client_addr, lms, ltcp, 
                                              bs);
  listener_thread = (pthread_t *) malloc(sizeof(pthread_t));
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  status = pthread_create(listener_thread, &attr, &run_listener_callback,
                          (void *) lp);
  return status;
}

void create_sender() {
  int status = start_sender();
  if (status) {
    exit_error("Could not start the sender");
  } 
}

int start_sender() {
  sender = (pthread_t *) malloc(sizeof(pthread_t));
  SenderMutexSet *mu_set = initialize_sender_mutex_set(client_addr_mu, io_mu);
  SenderThreadCondPacks *cond_packs = 
            initialize_sender_cond_packs(calc_to_sender_signal, sender_signal);
  SenderPack *pack = 
                initialize_sender_pack(net_interface->sd_out, client_addr, cs, 
                                      mu_set, cond_packs);
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  int status = 
        pthread_create(sender, &attr, &run_sender_callback, (void *) pack);
  return status;
}

void *run_listener_callback(void *lp) {
  pthread_barrier_wait(init_barrier);
  run_listener((ListenerPack *) lp);
  return NULL;
}

void *run_sender_callback(void *sp) {
  SenderPack *sender_pack = (SenderPack *) sp;
  pthread_barrier_wait(init_barrier);
  run_sender(sender_pack);
  return NULL;
}

void wait_listeners_write() {
  wait_listener(listener_one_signal);
  wait_listener(listener_two_signal);
}

void wait_listener(ThreadConditionPack *listener_signal) {
  pthread_mutex_lock(listener_signal->mutex_to_use);
  while(is_cond_false(listener_signal)) {
    pthread_cond_wait(listener_signal->cond_var, 
                      listener_signal->mutex_to_use); 
  }
  set_cond_to_verify_to_false(listener_signal);
  pthread_mutex_unlock(listener_signal->mutex_to_use);
}

void signal_read() {
  signal_listener(calc_signal_to_listener_one);
  signal_listener(calc_signal_to_listener_two);
}

void signal_listener(ThreadConditionPack *calc_to_listener_signal) {
  pthread_mutex_lock(calc_to_listener_signal->mutex_to_use);
  set_cond_to_verify_to_true(calc_to_listener_signal);
  pthread_cond_signal(calc_to_listener_signal->cond_var);
  pthread_mutex_unlock(calc_to_listener_signal->mutex_to_use);
}

void read_batches() {
  first_batch = get_first_stock_from_batch(bs, &first_batch_len);
  second_batch = get_second_stock_from_batch(bs, &second_batch_len);  
  clean_batch_stock(bs);
  printf("First batch: ");
  int i;
  for (i = 0; i < first_batch_len; i++) {
    printf("%f ", first_batch[i]);
  }
  printf("\nSecond batch: ");
  for (i = 0; i < second_batch_len; i++) {
    printf("%f ", second_batch[i]);
  }
  printf("\n");
}

void calculate_res() {
  size_t received_data_len = first_batch_len + second_batch_len;
  float *received_data = (float *) malloc(sizeof(float) * received_data_len);
  memcpy((void *) received_data, (void *) first_batch, 
          first_batch_len * sizeof(float));
  memcpy((void *)received_data + (sizeof(float) * first_batch_len), 
          (void *) second_batch, second_batch_len * sizeof(float));
  
  // TODO: add calculations!!!
  free(received_data);
  received_data = get_random_float_buf(13);
  float *calcs_result = server_calculations(received_data);
  add_to_calculations_stock(cs, calcs_result, SERVER_TO_CLIENT_PARAMS_COUNT); 
}

void signal_calculated() {
  pthread_mutex_lock(calc_to_sender_signal->mutex_to_use);
  set_cond_to_verify_to_true(calc_to_sender_signal);
  pthread_cond_signal(calc_to_sender_signal->cond_var);
  pthread_mutex_unlock(calc_to_sender_signal->mutex_to_use); 
}

void wait_for_sender() {
  pthread_mutex_lock(sender_signal->mutex_to_use);
  while(is_cond_false(sender_signal)) {
    pthread_cond_wait(sender_signal->cond_var, 
                      sender_signal->mutex_to_use); 
  }
  set_cond_to_verify_to_false(sender_signal);
  pthread_mutex_unlock(sender_signal->mutex_to_use);
}
