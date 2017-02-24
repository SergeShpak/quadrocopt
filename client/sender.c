#include <stdlib.h>

#include "include/network_interactions.h"
#include "include/sender.h"

SenderPack *initialize_sender_pack(int sd, ServerAddress *server_addr, 
                                SenderStock *s, SenderMutexSet *mu_set, 
                                          SenderThreadCondPacks *cond_packs) {
  SenderPack *pack = (SenderPack *) malloc(sizeof(SenderPack));
  pack->sd = sd;
  pack->server_addr = server_addr;
  pack->stock = s;
  pack->mu_set = mu_set;
  pack->cond_packs = cond_packs;
  return pack; 
}

void free_sender_pack(SenderPack *sp) {
  free(sp);
}

SenderMutexSet *initialize_sender_mutex_set(pthread_mutex_t *io_mu) {
  SenderMutexSet *mu_set = (SenderMutexSet *) malloc(sizeof(SenderMutexSet)); 
  mu_set->io_mu = io_mu;
  return mu_set;
}

void free_sender_mu_set(SenderMutexSet *mu_set) {
  free(mu_set);
}

SenderThreadCondPacks *initialize_sender_cond_packs(
                                  ThreadConditionPack *sender_signal,
                                  ThreadConditionPack *calc_to_sender_signal) {
  SenderThreadCondPacks *cond_packs = 
              (SenderThreadCondPacks *) malloc(sizeof(SenderThreadCondPacks));
  cond_packs->calc_to_sender_signal = calc_to_sender_signal;
  cond_packs->sender_signal = sender_signal;
  return cond_packs;
}

void free_sender_cond_packs(SenderThreadCondPacks *cond_packs) {
  free(cond_packs);
}
