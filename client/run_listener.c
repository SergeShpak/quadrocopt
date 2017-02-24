#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "include/utils.h"
#include "include/listener.h"
#include "include/constants.h"
#include "include/threading_stuff.h"

void receive_packet(ListenerPack *lp, char *buf);
void store_batch(Packet *pack, ListenerPack *lp);
char *allocate_buf();

void run_listener(ListenerPack *lp) {
  int rounds = 0;
  char *bufin = allocate_buf(); 
  while(1) {
    rounds++;
    pthread_mutex_lock(lp->mu_set->io_mu);
    printf("[LISTENER]: round %d\n", rounds);
    pthread_mutex_unlock(lp->mu_set->io_mu);
    receive_packet(lp, bufin); 
    Packet *received_pack = bytes_to_pack(bufin);
    wait_with_pack(lp->cond_packs->calc_to_listener_signal);
    store_batch(received_pack, lp);
    signal_with_pack(lp->cond_packs->listener_signal);
  }
}

void receive_packet(ListenerPack *lp, char *buf) {
  int bytes_recv;
  struct sockaddr remote; 
  socklen_t addr_len;
  bytes_recv = recvfrom(lp->sd, buf, MAXBUF, 0, 
                        (struct sockaddr *)&remote, &addr_len);
  pthread_mutex_lock(lp->mu_set->io_mu);
  printf("[LISTENER] received data\n");
  pthread_mutex_unlock(lp->mu_set->io_mu);
}

// TODO: DRY!
void store_batch(Packet *pack, ListenerPack *lp) {
  float *payload = get_floats(pack);
  size_t count = get_floats_count(pack);
  add_to_batch_stock(lp->batch_stock, payload, count); 
}

char *allocate_buf() {
  char *buf = (char*) malloc(sizeof(char) * MAXBUF);
  return buf;
}
