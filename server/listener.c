#include <pthread.h>
#include <stdlib.h>
#include <netinet/in.h>

#include "include/listener.h"
#include "include/threading_stuff.h"

ListenerPack *initialize_listener_pack(listener_t type, int sd, 
                      ClientAddress *client_addr, ListenerMutexSet *mu_set, 
                      ListenerThreadCondPackets *cond_packs, BatchStock *bs) {
  ListenerPack *lp = (ListenerPack *) malloc(sizeof(ListenerPack));
  lp->type = type;
  lp->sd = sd;
  lp->client_addr = client_addr;
  lp->mu_set = mu_set;
  lp->batch_stock = bs;
  lp->cond_packs = cond_packs;
  return lp;
}

void free_listener_pack(ListenerPack *lp) {
  free(lp);
}

ListenerMutexSet *create_listener_mutex_set(
                                        pthread_mutex_t *batch_stock_access_mu,
                                        pthread_mutex_t *client_addr_mu, 
                                        pthread_mutex_t *io_mu) {
  ListenerMutexSet *lms = (ListenerMutexSet *)malloc(sizeof(ListenerMutexSet));
  lms->batch_stock_access_mu = batch_stock_access_mu;
  lms->client_addr_mu = client_addr_mu;
  lms->io_mu = io_mu;
  return lms;
}

void free_listener_mutex_set(ListenerMutexSet *mu_set) {
  free(mu_set);
}

ListenerThreadCondPackets *initialize_cond_packs(
                                ThreadConditionPack *listener_signal,
                                ThreadConditionPack *calc_to_listener_signal) {
  ListenerThreadCondPackets *cond_packs = 
      (ListenerThreadCondPackets *) malloc(sizeof(ListenerThreadCondPackets));
  cond_packs->listener_signal = listener_signal;
  cond_packs->calc_to_listener_signal = calc_to_listener_signal;
  return cond_packs;
}
