#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

#include "include/listener.h"
#include "include/constants.h"
#include "include/utils.h"
#include "include/threading_stuff.h"

// Static functions

char *allocate_buf();
void listener_error_exit(char *msg);
char *get_listener_msg(char *msg, int listener_number);
void listener_error_exit(char *msg);
void store_batch(Packet *pack, ListenerPack *lp);
void do_first_run(ListenerPack *lp, char *buf);
void receive_packet(ListenerPack *lp, char *buf);
void wait_until_calc_reads(ListenerPack *lp);
void signal_ready(ListenerPack *lp);
Packet *gen_pack(listener_t type);

// End of static functions region

void run_listener(ListenerPack *lp) {
  char *bufin = allocate_buf(); 
  char *msg;
  do_first_run(lp, bufin);
  while(1) {
    receive_packet(lp, bufin); 
    Packet *received_pack = bytes_to_pack(bufin);
    wait_until_calc_reads(lp);
    store_batch(received_pack, lp);
    msg = get_listener_msg("Batch stored\n", lp->type);
    safe_print(msg, lp->mu_set->io_mu);
    free(msg);
    signal_ready(lp);
  }
}

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

ListenerMutexSet *create_listener_mutex_set(pthread_mutex_t *batch_stock_mu,
                    pthread_mutex_t *batch_stock_access_mu, 
                    pthread_mutex_t *client_addr_mu, pthread_mutex_t *io_mu) {
  ListenerMutexSet *lms = (ListenerMutexSet *)malloc(sizeof(ListenerMutexSet));
  lms->batch_stock_mu = batch_stock_mu;
  lms->batch_stock_access_mu = batch_stock_access_mu;
  lms->client_addr_mu = client_addr_mu;
  lms->io_mu = io_mu;
  return lms;
}

void free_listener_mutex_set(ListenerMutexSet *mu_set) {
  free(mu_set);
}

ListenerThreadCondPackets *initialize_cond_packs(
                              ThreadConditionPack *calc_read_cond, 
                              ThreadConditionPack *calc_ready_condition_pack) {
  ListenerThreadCondPackets *cond_packs = 
      (ListenerThreadCondPackets *) malloc(sizeof(ListenerThreadCondPackets));
  cond_packs->calc_read_cond = calc_read_cond;
  cond_packs->calc_ready_condition_pack = calc_ready_condition_pack;
  return cond_packs;
}

char *allocate_buf() {
  char *buf = (char*) malloc(sizeof(char) * MAXBUF);
  return buf;
}

void listener_error_exit(char *msg) {
  size_t msg_len = strlen(msg);
  char msg_local[msg_len + 1];
  strncpy(msg_local, msg, msg_len);
  msg_local[msg_len] = '\0';
  exit_error(msg_local);
}

char *get_listener_msg(char *msg, int listener_number) {
  char *prefix_listener = "Listener ";
  size_t prefix_listener_len = strlen(prefix_listener);
  size_t num_repr_len = 12;
  size_t msg_len = strlen(msg);
  size_t total_len = prefix_listener_len + num_repr_len + msg_len + 1;
  char *result = (char *) malloc (sizeof(char) * total_len); 
  snprintf(result, total_len - 1, "%s%d: %s", 
            prefix_listener, listener_number, msg); 
  return result;
}

// TODO: DRY!
void store_batch(Packet *pack, ListenerPack *lp) {
  float *payload = get_floats(pack);
  size_t count = get_floats_count(pack);
  char *err_msg;
  switch (lp->type) {
    case LISTENER_FIRST:
      pthread_mutex_lock(lp->mu_set->batch_stock_access_mu);
      add_first_stock_to_batch(lp->batch_stock, payload, count);
      pthread_mutex_unlock(lp->mu_set->batch_stock_access_mu);
      break;
    case LISTENER_SECOND:
      pthread_mutex_lock(lp->mu_set->batch_stock_access_mu);
      add_second_stock_to_batch(lp->batch_stock, payload, count);
      pthread_mutex_unlock(lp->mu_set->batch_stock_access_mu);
      break;
    default:
      err_msg = 
        get_listener_msg("Cannot store batch: packet type unknown", lp->type);
      listener_error_exit(err_msg);
  }
  return; 
}

void receive_packet(ListenerPack *lp, char *buf) {
  int bytes_recv;
  struct sockaddr *remote_addr = 
                          (struct sockaddr *) malloc(sizeof(struct sockaddr));
  socklen_t addr_len;
  bytes_recv = recvfrom(lp->sd, buf, MAXBUF, 0, 
                                  (struct sockaddr *) remote_addr, &addr_len);
  if (bytes_recv < 0) {
      char *err_msg = get_listener_msg("Error receiving data", lp->type);
      listener_error_exit(err_msg);
  }
  pthread_mutex_lock(lp->mu_set->client_addr_mu);
  if (!(are_sockaddrs_equal(lp->client_addr->addr, remote_addr))) {
    set_client_address(lp->client_addr, remote_addr, addr_len); 
  }
  pthread_mutex_unlock(lp->mu_set->client_addr_mu);
}

// TODO: DRY!
void do_first_run(ListenerPack *lp, char *buf) {
  receive_packet(lp, buf);
  Packet *received_pack = bytes_to_pack(buf);
  store_batch(received_pack, lp);
  free_pack(received_pack);
  char *msg = get_listener_msg("Batch stored\n", lp->type);
  safe_print(msg, lp->mu_set->io_mu);
  free(msg);
  signal_ready(lp);
}

void wait_until_calc_reads(ListenerPack *lp) {
  while(!(lp->cond_packs->calc_ready_condition_pack->cond_to_verify)) {
    pthread_cond_wait(lp->cond_packs->calc_ready_condition_pack->cond_var,
                      lp->cond_packs->calc_ready_condition_pack->mutex_to_use);
  }
  set_cond_to_verify_to_false(lp->cond_packs->calc_ready_condition_pack);
}

void signal_ready(ListenerPack *lp) {
  pthread_mutex_unlock(lp->mu_set->batch_stock_mu);
  while (!(lp->cond_packs->calc_read_cond->cond_to_verify)) {
    pthread_cond_wait(lp->cond_packs->calc_read_cond->cond_var,
                      lp->cond_packs->calc_read_cond->mutex_to_use);
  }
  set_cond_to_verify_to_false(lp->cond_packs->calc_read_cond);
  pthread_mutex_lock(lp->mu_set->batch_stock_mu);
}

Packet *gen_pack(listener_t type) {
  size_t floats_count = 4;
  float buf[floats_count];
  float base = 1.1111;
  for (int i = 0; i < floats_count; i++) {
    buf[i] = base + (float) i + (float) type;
  }
  Packet *pack = gen_packet_from_floats(buf, floats_count);
  return pack;
}
