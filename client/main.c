#include <errno.h>
#include <signal.h>
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
#include "include/signal_handling.h"

pthread_mutex_t *io_mu = NULL;
ThreadConditionPacksCollection *thread_cond_packs = NULL;
char *server_addr_str;
ServerAddress *server_addr = NULL;
NetworkInterface *net_interface = NULL;
BatchStock *listener_stock = NULL;
PrinterStock *printer_stock = NULL;
SenderStock *sender_stock = NULL;
WorkersCollection *workers = NULL;
pthread_barrier_t *init_barrier = NULL;
float *listener_batch = NULL;
size_t listener_batch_len;
float *sender_first_batch = NULL;
size_t sender_first_batch_len;
float *sender_second_batch = NULL;
size_t sender_second_batch_len;

CalculatorData *calc_data = NULL;

PrinterParamsCollection *printer_params_collection = NULL;

const char *fname_received_params = "out.txt";
const char *fname_sent_params = "in.txt";

const char *fname_time = "time.txt";
const char *fname_x = "xcoord.txt";
const char *fname_y = "ycoord.txt";
const char *fname_z = "zcoord.txt";
const char *fname_psy = "psy.txt";
const char *fname_results = "results.txt";

// Static functions declarations

void initialize_globals(void);
void set_signals(void);
void initialize_mutexes(void);
ThreadConditionPack *initialize_thread_cond_pack(int init_verif_var);
void initialize_barriers(void);
void initialize_condition_packs(void);
PrinterParamsCollection *create_printer_params_collection();
pthread_cond_t *init_cond_var(pthread_condattr_t *attr);
pthread_mutex_t *init_mutex(pthread_mutexattr_t *mu_attr);
void create_output_files(void);
void create_file(const char *file_name);
void free_mutexes(void);
void free_mutex(pthread_mutex_t*);
void *run_listener_callback(void*);
void *run_sender_callback(void*);
void *run_printer_callback(void*);
void create_listener(void);
int start_listener(void);
void spawn_workers(void);
void read_listener_batch(void);

void do_client_calculations(void);

int start_sender(void);
void fill_sender_stock(void);
void create_sender(void);
void calculate_res(void);

void create_printer(void);
int start_printer(void);
void print_calc_data(CalculatorData *calc_data);
float *get_calc_data_for_printer(CalculatorData *calc_data, size_t *data_len);
void calculate_ref(void);
void get_next_step(void);
float *get_u(float *payload);
float *normalize_vector(float *vec, size_t len, float cur_time);

void wait_workers_to_finish(void);
void stop_worker(pthread_t *worker);

// End of static functions declarations

int main(int argc, char **argv) {
  if (argc < 2) {
    exit_error("Expected server host address\n"); 
    exit(1);
  }
  server_addr_str = argv[1];
  initialize_globals();
  set_signals();
  spawn_workers();
  calculate_ref();
  do_client_calculations();
  wait_workers_to_finish();
  return 0;
}

void initialize_globals() {
  net_interface = initialize_network_interface();
  server_addr = set_server_address(server_addr_str);
  net_interface->server_addr = server_addr;
  listener_stock = initialize_batch_stock();
  sender_stock = initialize_sender_stock();
  printer_stock = initialize_printer_stock();
  printer_params_collection = create_printer_params_collection();
  workers = initialize_workers_collection();
  calc_data = initialize_calc_data(0, 0.1);
  initialize_mutexes();
  initialize_barriers();
  initialize_condition_packs();
  fill_sender_stock();
}

void set_signals(void) {
  if (SIG_ERR == signal(SIGUSR1, sig_handler)) {
    pthread_mutex_lock(io_mu);
    printf("Could not register signal handler: %d\n", errno);
    pthread_mutex_unlock(io_mu);
  }
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
  ThreadConditionPack *sender_to_signal = initialize_thread_cond_pack(1);
  ThreadConditionPack *sender_from_signal = initialize_thread_cond_pack(0);
  ThreadConditionPack *listener_to_signal = initialize_thread_cond_pack(1);
  ThreadConditionPack *listener_from_signal = initialize_thread_cond_pack(0);
  ThreadConditionPack *printer_to_signal = initialize_thread_cond_pack(0);
  ThreadConditionPack *printer_from_signal = initialize_thread_cond_pack(1);
  thread_cond_packs = initialize_thread_cond_packs_collection(
                                      sender_to_signal, sender_from_signal,
                                      listener_to_signal, listener_from_signal,
                                      printer_to_signal, printer_from_signal);
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

PrinterParamsCollection *create_printer_params_collection() {
  PrinterParameters *results_params = 
    initialize_printer_params(FOUT, FLOAT_ARR, fname_results, "a+", NULL, -1);
  PrinterParamsCollection *collection = 
                        initialize_printer_params_collection(results_params);
  return collection;
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

void spawn_workers(void) {
  create_listener();
  create_sender();
  create_printer();
  pthread_barrier_wait(init_barrier);
}

void create_listener(void) {  
  int status = start_listener();
  if (status) {
    exit_error("Could not start the sender");
  }
}

void create_output_files(void) {
#ifdef DEBUG
  create_file(fname_received_params);
  create_file(fname_sent_params);
#endif
  create_file(fname_time);
  create_file(fname_x);
  create_file(fname_y);
  create_file(fname_z);
  create_file(fname_psy);
}

void create_file(const char *file_name) {
  FILE *fp = fopen(file_name, "w");
  fclose(fp);
}

int start_listener(void) {
  int status;
  ListenerMutexSet *lms = create_listener_mutex_set(io_mu);
  ListenerThreadCondPackets *ltcp = initialize_cond_packs(
                                    thread_cond_packs->listener_from_signal, 
                                    thread_cond_packs->listener_to_signal);
  int sd = net_interface->sd_in;
  ListenerPack *lp = initialize_listener_pack(sd, lms, ltcp, listener_stock);
  pthread_t *listener_thread = (pthread_t *) malloc(sizeof(pthread_t));
  add_to_workers_collection(listener_thread, workers, &(workers->listener));
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
  pthread_t *sender = (pthread_t *) malloc(sizeof(pthread_t));
  add_to_workers_collection(sender, workers, &(workers->sender));
  SenderMutexSet *mu_set = initialize_sender_mutex_set(io_mu);
  SenderThreadCondPacks *cond_packs = initialize_sender_cond_packs(
                                      thread_cond_packs->sender_from_signal, 
                                      thread_cond_packs->sender_to_signal);
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
  add_to_workers_collection(printer, workers, &(workers->printer));
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  PrinterMutexSet *mu_set = initialize_printer_mu_set(io_mu);
  PrinterThreadCondPackets *cond_packs = initialize_printer_thread_cond_packs(
                                        thread_cond_packs->printer_from_signal, 
                                        thread_cond_packs->printer_to_signal);
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
  run_printer(printer_pack, create_output_files);
  return NULL;
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
  add_second_batch_to_sender_stock(sender_stock,
                                sender_second_batch, sender_second_batch_len);
}

void do_client_calculations() {
  while(calc_data->curr_time <= calc_data->time_end) {
    get_next_step();
    calc_data->curr_time += calc_data->step;  
    print_calc_data(calc_data);  
    // TODO: delete below
    //wait_with_pack(printer_signal);
    //write_to_printer_stock();
    //signal_with_pack(calc_to_printer_signal);
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

void print_calc_data(CalculatorData *calc_data) {
  size_t payload_len;
  float *payload = get_calc_data_for_printer(calc_data, &payload_len);
  size_t payload_len_bytes = payload_len * sizeof(float);
  wait_with_pack(thread_cond_packs->printer_from_signal);
  printer_params_collection->results_params->payload = (void *) payload;
  printer_params_collection->results_params->payload_len = payload_len_bytes;
  printer_stock->params = printer_params_collection->results_params;
  signal_with_pack(thread_cond_packs->printer_to_signal); 
}

float *get_calc_data_for_printer(CalculatorData *calc_data, size_t *data_len) {
  size_t len = 5;
  float *payload = (float *) malloc(sizeof(float) * len);
  payload[0] = calc_data->curr_time;
  payload[1] = calc_data->Vx;
  payload[2] = calc_data->Vy;
  payload[3] = calc_data->Vz;
  payload[4] = calc_data->Vpsy;
  *data_len = len;
  return payload; 
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
  wait_with_pack(thread_cond_packs->sender_from_signal);
  add_first_batch_to_sender_stock(sender_stock, first_batch, 6);
  add_second_batch_to_sender_stock(sender_stock, second_batch, 7);
  signal_with_pack(thread_cond_packs->sender_to_signal);
  free(first_batch);
  free(second_batch);
  wait_with_pack(thread_cond_packs->listener_from_signal);
  read_listener_batch();
  float *u = (float *) malloc(sizeof(float) * 4);
  memcpy(u, listener_batch, sizeof(float) * 4);
  free(listener_batch);
  listener_batch = NULL;
  signal_with_pack(thread_cond_packs->listener_to_signal);
#ifdef DEBUG
  FILE *in = fopen(fname_received_params, "a+");
  for (int i = 0; i < 4; i++) {
    fprintf(in, "%f ", u[i]); 
  }
  fprintf(in, "\n");
  fclose(in);
#endif
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

void wait_workers_to_finish() {
#ifdef DEBUG
  pthread_mutex_lock(io_mu);
  printf("%s\n", "Started cleaning");
  pthread_mutex_unlock(io_mu);
#endif  
  CollectionList *curr_worker = workers->workers;
  pthread_t *curr_thread;  
  while(NULL != curr_worker) {
    curr_thread = (pthread_t *) curr_worker->el;
    stop_worker(curr_thread);
    pthread_join(*curr_thread, NULL);
    curr_worker = curr_worker->next;
  }
#ifdef DEBUG
  pthread_mutex_lock(io_mu);
  printf("%s\n", "All threads joined");
  pthread_mutex_unlock(io_mu);
#endif
  return;
}

void stop_worker(pthread_t *worker) {
  pthread_kill(*worker, SIGUSR1); 
}
