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
#include "include/printer.h"
#include "include/nrutil.h"

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
float *listener_batch = NULL;
size_t listener_batch_len;
float *sender_first_batch = NULL;
size_t sender_first_batch_len;
float *sender_second_batch = NULL;
size_t sender_second_batch_len;

CalculatorData *calc_data = NULL;

ThreadConditionPack *calc_to_sender_signal;
ThreadConditionPack *sender_signal;

char *file_out_name = "out.txt";
char *file_in_name = "in.txt";

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
void *run_printer_callback(void*);
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
void write_to_printer_stock();
void calculate_ref();
void get_next_step();
float *get_u();
float *normalize_vector(float *vec, size_t len, float cur_time);

// End of static functions declarations

int main(int argc, char **argv) {
  if (argc < 2) {
    exit_error("Expected server host address\n"); 
    exit(1);
  }
  server_addr_str = argv[1];
  initialize_globals();
  spawn_workers();
  FILE *f = fopen(file_out_name, "w");
  fclose(f);
  f = fopen(file_in_name, "w");
  fclose(f);
  calculate_ref();
  do_client_calculations();
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
  sender_signal = initialize_thread_cond_pack(1);
  calc_to_sender_signal = initialize_thread_cond_pack(0);
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
  PrinterMutexSet *mu_set = initialize_printer_mu_set(io_mu);
  PrinterThreadCondPackets *cond_packs = 
                        initialize_printer_thread_cond_packs(printer_signal, 
                        calc_to_printer_signal);
  PrinterPack *pack = 
                    initialize_printer_pack(mu_set, cond_packs, printer_stock);
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

void *run_printer_callback(void *pp) {
  PrinterPack *printer_pack = (PrinterPack *) pp;
  pthread_barrier_wait(init_barrier);
  run_printer(printer_pack);
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
  FILE *f = fopen(file_in_name, "a+");
  for (int i = 0; i < listener_batch_len; i++) {
    fprintf(f, "%f ", listener_batch[i]); 
  }
  fprintf(f, "\n");
  fclose(f);
  clean_batch_stock(listener_stock);
}

void calculate_res() {
  do_client_calculations(); 
  free(listener_batch);
  add_first_batch_to_sender_stock(sender_stock, 
                                sender_first_batch, sender_first_batch_len);
  add_second_batch_to_sender_stock(sender_stock,
                                sender_second_batch, sender_second_batch_len);
}

void wait_sender() {
  wait_with_pack(sender_signal);
}

void signal_sender() {
  signal_with_pack(calc_to_sender_signal);
}

void do_client_calculations() {
  while(calc_data->curr_time <= calc_data->time_end) {
    get_next_step();
    calc_data->curr_time += calc_data->step;    
    wait_with_pack(printer_signal);
    write_to_printer_stock();
    signal_with_pack(calc_to_printer_signal);
  }
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

void write_to_printer_stock() {
  float *new_printer_batch = (float *) malloc(sizeof(float) * 5);
  new_printer_batch[0] = calc_data->curr_time;
  new_printer_batch[1] = calc_data->Vx;
  new_printer_batch[2] = calc_data->Vy;
  new_printer_batch[3] = calc_data->Vz;
  new_printer_batch[4] = calc_data->Vpsy;
  printer_stock->batch = new_printer_batch;
  printer_stock->batch_len = 5;
}

void calculate_ref() {
  calc_ref(calc_data->time_start, calc_data->time_end, calc_data->step);
}

void get_next_step() {
  float *y,*yout,*dydx;
  y = vector(1,NVAR);
  yout = vector(1,NVAR);
  dydx = vector (1,NVAR);
  y[1] = calc_data->Vx;
  y[2] = calc_data->VdotX;
  y[3] = calc_data->Vy;
  y[4] = calc_data->VdotY;
  y[5] = calc_data->Vz;
  y[6] = calc_data->VdotZ;
  y[7] = calc_data->Vtheta;
  y[8] = calc_data->VdotTheta;
  y[9] = calc_data->Vphi;
  y[10] = calc_data->VdotPhi;
  y[11] = calc_data->Vpsy;
  y[12] = calc_data->VdotPsy;

  float ti = calc_data->curr_time;
  float step = calc_data->step;
  float *payload = get_payload_from_calc_data(calc_data);
  float *u = get_u(payload);
  free(payload);
  (*derivs)(ti,y,dydx, u);
  free(u);

  int i;
  float xh,hh,h6,*dym,*dyt,*yt;
  float x = ti;
  float h = step;

  dym=vector(1,NVAR);
  dyt=vector(1,NVAR);
  yt=vector(1,NVAR);
  hh=h*0.5;
  h6=h/6.0;
  xh=x+hh;
  for (i=1;i<=NVAR;i++) yt[i]=y[i]+hh*dydx[i];
  float *n_vec = normalize_vector(yt, NVAR, xh);
  u = get_u(n_vec);
  free(n_vec);
  (*derivs)(xh,yt,dyt,u);
  free(u);
  for (i=1;i<=NVAR;i++) yt[i]=y[i]+hh*dyt[i];
  n_vec = normalize_vector(yt, NVAR, xh);
  u = get_u(n_vec);
  free(n_vec);
  (*derivs)(xh,yt,dym,u);
  free(u);
  for (i=1;i<=NVAR;i++) {
    yt[i]=y[i]+h*dym[i];
    dym[i] += dyt[i];
  }
  n_vec = normalize_vector(yt, NVAR, x+h);
  u = get_u(n_vec);
  free(n_vec);
  (*derivs)(x+h,yt,dyt,u);
  free(u);
  for (i=1;i<=NVAR;i++)
    yout[i]=y[i]+h6*(dydx[i]+dyt[i]+2.0*dym[i]);
  set_calc_data(calc_data, yout);
  
  free_vector(yt,1,NVAR);
  free_vector(dyt,1,NVAR);
  free_vector(dym,1,NVAR);
  free_vector(yout,1,NVAR);
  free_vector(y,1,NVAR);
  free_vector(dydx,1,NVAR);
}

float *get_u(float *payload) {
  size_t first_bytes_size = sizeof(float) * 6;
  size_t second_bytes_size = sizeof(float) * 7;
  float *first_batch = (float *) malloc(first_bytes_size);
  float *second_batch = (float *) malloc(second_bytes_size);
  memcpy(first_batch, payload, first_bytes_size);
  memcpy(second_batch, payload + 6, second_bytes_size);
  wait_with_pack(sender_signal);
  add_first_batch_to_sender_stock(sender_stock, first_batch, 6);
  add_second_batch_to_sender_stock(sender_stock, second_batch, 7);
  signal_with_pack(calc_to_sender_signal);
  free(first_batch);
  free(second_batch);
  wait_with_pack(listener_signal);
  read_listener_batch();
  float *u = (float *) malloc(sizeof(float) * 4);
  memcpy(u, listener_batch, sizeof(float) * 4);
  free(listener_batch);
  listener_batch = NULL;
  signal_with_pack(calc_to_listener_signal);
  FILE *in = fopen(file_in_name, "a+");
  for (int i = 0; i < 4; i++) {
    fprintf(in, "%f ", u[i]); 
  }
  fprintf(in, "\n");
  fclose(in);
  return u; 
}

float *normalize_vector(float *vec, size_t len, float cur_time) {
  float *new_vec = (float *) malloc(sizeof(float) * (len + 1));
  for (int i = 0; i < len; i++) {
    new_vec[i] = vec[i + 1];
  }
  new_vec[len] = cur_time;
  return new_vec;
}
