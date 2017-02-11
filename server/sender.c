#include <stdlib.h>

#include "include/sender.h"

SenderPack *initialize_sender_pack(int sd, CalculationsStock *cs, 
                  SenderMutexSet *mu_set, SenderThreadCondPacks *cond_packs) {
  SenderPack *pack = (SenderPack *) malloc(sizeof(SenderPack));
  pack->sd = sd;
  pack->cs = cs;
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
                  ThreadConditionPack *reader_calculated_cond_pack,
                  ThreadConditionPack *sender_sent_cond_pack) {
  SenderThreadCondPacks *cond_packs = 
              (SenderThreadCondPacks *) malloc(sizeof(SenderThreadCondPacks));
  cond_packs->reader_calculated_cond_pack = reader_calculated_cond_pack;
  cond_packs->sender_sent_cond_pack = sender_sent_cond_pack;
  return cond_packs;
}

void free_sender_cond_packs(SenderThreadCondPacks *cond_packs) {
  free(cond_packs);
}
