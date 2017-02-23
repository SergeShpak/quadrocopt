#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "include/utils.h"
#include "include/listener.h"
#include "include/constants.h"

void do_first_run(ListenerPack *lp, char *buf);
void receive_packet(ListenerPack *lp, char *buf);
void store_batch(Packet *pack, ListenerPack *lp);
void wait_until_calc_reads(ListenerPack *lp);
void signal_listener_ready(ListenerPack *lp);
void listener_error_exit(char *msg);
char *get_listener_msg(char *msg, int listener_number);
char *allocate_buf();

//TODO: delete in release
float *get_random_float_buf(size_t len);

void run_listener(ListenerPack *lp) {
  int rounds = 0;
  char *bufin = allocate_buf(); 
  char *msg;
  //do_first_run(lp, bufin);
  while(1) {
    rounds++;
    pthread_mutex_lock(lp->mu_set->io_mu);
    printf("[LISTENER]: round %d\n", rounds);
    pthread_mutex_unlock(lp->mu_set->io_mu);
    //receive_packet(lp, bufin); 
    //Packet *received_pack = bytes_to_pack(bufin);
    size_t buf_len = 4;
    float *stub_buf = get_random_float_buf(4);
    rand_sleep(0, 1500*1000);
    Packet *received_pack = gen_packet_from_floats(stub_buf, buf_len);
    wait_until_calc_reads(lp);
    store_batch(received_pack, lp);
    msg = get_listener_msg("Batch stored\n", lp->type);
    safe_print(msg, lp->mu_set->io_mu);
    free(msg);
    signal_listener_ready(lp);
  }
}

// TODO: DRY!
void do_first_run(ListenerPack *lp, char *buf) {
  //receive_packet(lp, buf);
  //Packet *received_pack = bytes_to_pack(buf);
  size_t buf_len = 4;
  float *stub_buf = get_random_float_buf(4);
  rand_sleep(0, 1500 * 1000);
  Packet *received_pack = gen_packet_from_floats(stub_buf, buf_len);
  store_batch(received_pack, lp);
  free_pack(received_pack);
  char *msg = get_listener_msg("Batch stored\n", lp->type);
  safe_print(msg, lp->mu_set->io_mu);
  free(msg);
  signal_listener_ready(lp);
}

void receive_packet(ListenerPack *lp, char *buf) {
  int bytes_recv;
  struct sockaddr_in remote; 
  socklen_t addr_len;
  //bytes_recv = recvfrom(lp->sd, buf, MAXBUF, 0, 
  //                      (struct sockaddr *)&remote, &addr_len);
//  if (bytes_recv < 0) {
//      char *err_msg = get_listener_msg("Error receiving data: ", lp->type);
//      safe_print(err_msg, lp->mu_set->io_mu);
//      pthread_mutex_lock(lp->mu_set->io_mu);
//      printf("%d", errno);
//      pthread_mutex_unlock(lp->mu_set->io_mu);
//      listener_error_exit("\n");
//  }
  pthread_mutex_lock(lp->mu_set->client_addr_mu);
  if (!(are_sockaddrs_equal((struct sockaddr *) lp->client_addr->addr, 
                            (struct sockaddr *) &remote))) {
    struct sockaddr *allocated_addr = copy_sockaddr(&remote);
    set_client_address(lp->client_addr, allocated_addr, addr_len); 
  }
  pthread_mutex_unlock(lp->mu_set->client_addr_mu);
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

//  TODO: dry
void wait_until_calc_reads(ListenerPack *lp) {
  ThreadConditionPack *calc_signal = lp->cond_packs->calc_to_listener_signal;
  pthread_mutex_lock(calc_signal->mutex_to_use);
  while(is_cond_false(calc_signal)) {
    pthread_cond_wait(calc_signal->cond_var, calc_signal->mutex_to_use);
  }
  set_cond_to_verify_to_false(calc_signal);
  pthread_mutex_unlock(calc_signal->mutex_to_use);
}

void signal_listener_ready(ListenerPack *lp) {
  ThreadConditionPack *listener_signal = lp->cond_packs->listener_signal;
  pthread_mutex_lock(listener_signal->mutex_to_use);
  set_cond_to_verify_to_true(listener_signal);
  pthread_cond_signal(listener_signal->cond_var);
  pthread_mutex_unlock(listener_signal->mutex_to_use);
}

void listener_error_exit(char *msg) {
  size_t msg_len = strlen(msg);
  char msg_local[msg_len + 1];
  strncpy(msg_local, msg, msg_len);
  msg_local[msg_len] = '\0';
  exit_error(msg_local);
}

char *allocate_buf() {
  char *buf = (char*) malloc(sizeof(char) * MAXBUF);
  return buf;
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
