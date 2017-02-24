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

pthread_mutex_t *io_mu = NULL;
ThreadConditionPack *sender_signal = NULL;
ThreadConditionPack *calc_to_sender_signal = NULL;
ThreadConditionPack *listener_signal = NULL;
ThreadConditionPack *calc_to_listener_signal = NULL;
ThreadConditionPack *printer_signal = NULL;
ThreadConditionPack *calc_to_printer_signal = NULL;
char *server_addr_str;
ServerAddress *server_addr = NULL;
NetworkInterface *net_interface = NULL;
BatchStock *listener_stock = NULL;
BatchStock *printer_stock = NULL;
SenderStock *sender_stock = NULL;
pthread_t *sender = NULL;
pthread_barrier_t *init_barrier = NULL;
float *printer_batch;
float printer_batch_len;
float *listener_batch;
size_t listener_batch_len;
float *sender_first_batch;
size_t sender_first_batch_len;
float *sender_second_batch;
size_t sender_second_batch_len;

CalculatorData *calc_data = NULL;

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
void *run_listener_callback(void*);
void *run_sender_callback(void*);
void create_listener();
int start_listener();
void spawn_workers();
void read_listener_batch();
void wait_listener();
void signal_listener();

void do_client_calculations();

int start_sender();
void fill_sender_stock();
void create_sender();
void calculate_res();
void wait_sender();
void signal_sender();

void create_printer();
int start_printer();

// End of static functions declarations

int main(int argc, char **argv) {
  if (argc < 2) {
    exit_error("Expected server host address\n"); 
    exit(1);
  }
  server_addr_str = argv[1];
  initialize_globals();
  spawn_workers();
  int rounds = 0;
  create_output_files();
  while(calc_data->curr_time <= calc_data->time_end) {
    rounds++;
    pthread_mutex_lock(io_mu);
    printf("[CALCULATOR]: round %d\n", rounds);
    pthread_mutex_unlock(io_mu);
    wait_listener();
    read_listener_batch();
    safe_print("[CALCULATOR]: batch read\n", io_mu);
    signal_listener();
     
    calculate_res();
    safe_print("[CALCULATOR]: results calculated\n", io_mu);
    wait_sender();
    signal_sender();
  }
  return 0;
}

void initialize_globals() {
  net_interface = initialize_network_interface();
  server_addr = set_server_address(server_addr_str);
  net_interface->server_addr = server_addr;
  listener_stock = initialize_batch_stock();
  sender_stock = initialize_sender_stock();
  printer_stock = initialize_batch_stock();
  fill_sender_stock();
  calc_data = initialize_calc_data(0, 10);
  initialize_mutexes();
  initialize_barriers();
  initialize_condition_packs();
}

void initialize_mutexes() {
  io_mu = init_mutex(NULL);
}

void initialize_barriers() {
  pthread_barrierattr_t attr;
  pthread_barrierattr_init(&attr);
  init_barrier = (pthread_barrier_t *) malloc(sizeof(pthread_barrier_t));
  pthread_barrier_init(init_barrier, &attr, 4);
}

void initialize_condition_packs() {
  sender_signal = initialize_thread_cond_pack(0);
  calc_to_sender_signal = initialize_thread_cond_pack(1);
  listener_signal = initialize_thread_cond_pack(0);
  calc_to_listener_signal = initialize_thread_cond_pack(1);
  printer_signal = initialize_thread_cond_pack(1);
  calc_to_printer_signal = initialize_thread_cond_pack(0);
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

void free_mutex(pthread_mutex_t *mu) {
  if (NULL != mu) {
    pthread_mutex_destroy(mu);
  }
  free(mu);
}

void spawn_workers() {
  create_listener();
  create_sender();
  create_printer();
  pthread_barrier_wait(init_barrier);
}

void create_listener() {  
  int status = start_listener();
  if (status) {
    exit_error("Could not start the sender");
  }
}

int start_listener() {
  int status;
  ListenerMutexSet *lms = create_listener_mutex_set(io_mu);
  ListenerThreadCondPackets *ltcp = initialize_cond_packs(listener_signal, 
                                                      calc_to_listener_signal);
  int sd = net_interface->sd_in;
  ListenerPack *lp = initialize_listener_pack(sd, lms, ltcp, listener_stock);
  pthread_t *listener_thread = (pthread_t *) malloc(sizeof(pthread_t));
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
  SenderMutexSet *mu_set = initialize_sender_mutex_set(io_mu);
  SenderThreadCondPacks *cond_packs = 
            initialize_sender_cond_packs(sender_signal, calc_to_sender_signal);
  SenderPack *pack = initialize_sender_pack(net_interface->sd_out, server_addr, 
                                            sender_stock, mu_set, cond_packs);
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  int status = 
        pthread_create(sender, &attr, &run_sender_callback, (void *) pack);
  return status;
}

void create_printer() {
  int status = start_printer();
  if (status) {
    exit_error("Could not start the printer");
  }
}

int start_printer() {
  pthread_t *printer = (pthread_t *) malloc(sizeof(pthread_t));
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  int status = 
          pthread_create(printer, &attr, &run_printer_callback, (void *) pack);
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

void wait_listener() {
  wait_with_pack(listener_signal);
}

void signal_listener() {
  signal_with_pack(calc_to_listener_signal);
}

void read_listener_batch() {
  listener_batch = get_batch_from_stock(listener_stock, &listener_batch_len);  
  clean_batch_stock(listener_stock);
}

void calculate_res() {
  do_client_calculations(); 
  free(listener_batch);
  add_first_batch_to_sender_stock(sender_stock, 
                                sender_first_batch, sender_first_batch_len);
  add_first_batch_to_sender_stock(sender_stock,
                                sender_second_batch, sender_second_batch_len);
}

void wait_sender() {
  wait_with_pack(sender_signal);
}

void signal_sender() {
  signal_with_pack(calc_to_sender_signal);
}

void do_client_calculations() {
  AngleCoordCommand *angle_coord_command = initialize_angle_coord_command();
  set_angle_command_u(angle_coord_command, listener_batch);
  do_next_step(calc_data, angle_coord_command);    
  free(sender_first_batch);
  free(sender_second_batch);
  sender_first_batch = get_first_batch_from_calc_data(calc_data);
  sender_second_batch = get_second_batch_from_calc_data(calc_data);
}

void fill_sender_stock() {
  size_t init_first_batch_len = 6;
  size_t init_second_batch_len = 7;
  float *init_first_batch = 
                        (float *) calloc(init_first_batch_len, sizeof(float));
  float *init_second_batch =
                        (float *) calloc(init_second_batch_len, sizeof(float));
  add_first_batch_to_sender_stock(sender_stock, 
                                  init_first_batch, init_first_batch_len);
  add_second_batch_to_sender_stock(sender_stock, 
                                  init_second_batch, init_second_batch_len);
}
