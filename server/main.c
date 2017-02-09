#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

#include "include/network_interactions.h"
#include "include/data_struct.h"

pthread_mutex_t *batch_stock_first_mu = NULL;
pthread_mutex_t *batch_stock_second_mu = NULL;
NetworkInterface *net_interface = NULL;
BatchStock *bs = NULL;

// Static functions declarations

void initialize_globals();
void initialize_mutexes();
pthread_mutex_t *init_mutex(pthread_mutexattr_t *mu_attr);
void free_mutexes();
void free_mutex(pthread_mutex_t*);
void tear_down();
int run_listener(ListenerPack *lp);

// End of static functions declarations

int main(int argc, char **argv) {
  initialize_globals();
  ListenerPack *first_listener_pack = 
    initialize_listener_pack(net_interface->sd_in_first, 
                              batch_stock_first_mu, bs);
  ListenerPack *second_listener_pack =
   initialize_listener_pack(net_interface->sd_in_second, 
                              batch_stock_second_mu, bs); 
  run_listener(first_listener_pack);
  run_listener(second_listener_pack);
  tear_down();
  return 0;
}

int run_listener(ListenerPack *lp) {
  return 0; 
}

void initialize_globals() {
  net_interface = initialize_network_interface();
  bs = initialize_batch_stock();
  initialize_mutexes();
}

void initialize_mutexes() {
  batch_stock_first_mu = init_mutex(NULL);
  batch_stock_second_mu = init_mutex(NULL);
}

pthread_mutex_t *init_mutex(pthread_mutexattr_t *mu_attr) {
  pthread_mutex_t *mu = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(mu, mu_attr);
  return mu;
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
