#include <string.h>
#include <pthread.h>

#include "include/sender.h"
#include "include/utils.h"
#include "include/io_stuff.h"
#include "include/utils.h"

void wait_for_calculator(SenderPack *sp);
void send_calcs(SenderPack *sp);
void signal_sent(SenderPack *sp);

void run_sender(SenderPack *sp) {
  safe_print("Sender here\n", sp->mu_set->io_mu);
  while(1) {
    wait_for_calculator(sp);
    send_calcs(sp);
    signal_sent(sp);
  } 
}

void wait_for_calculator(SenderPack *sp) {
  pthread_mutex_lock(
      sp->cond_packs->reader_calculated_cond_pack->mutex_to_use);
  while(!(sp->cond_packs->reader_calculated_cond_pack->cond_to_verify)) {
    pthread_cond_wait(sp->cond_packs->reader_calculated_cond_pack->cond_var,
                    sp->cond_packs->reader_calculated_cond_pack->mutex_to_use);
  }
  set_cond_to_verify_to_false(sp->cond_packs->reader_calculated_cond_pack);
  pthread_mutex_unlock(
      sp->cond_packs->reader_calculated_cond_pack->mutex_to_use);
}

void send_calcs(SenderPack *sp) {
  size_t calc_len = sp->cs->calculations_len;
  float *calcs = sp->cs->calculations;
  Packet *pack = gen_packet_from_floats(calcs, calc_len);
  char *pack_bytes = pack_to_bytes(pack);
  size_t pack_size = get_pack_bytes_size(pack);
  pthread_mutex_lock(sp->mu_set->client_addr_mu);
  struct sockaddr *addr = 
                  copy_sockaddr((struct sockaddr *)sp->client_addr->addr);
  size_t addr_len = sp->client_addr->addr_len;
  pthread_mutex_unlock(sp->mu_set->client_addr_mu);
  int bytes_sent = sendto(sp->sd, pack_bytes, pack_size, 0, addr, addr_len); 
  if (bytes_sent < 0) {
    exit_error("Error sending data\n"); 
  }
  free(addr);
//  char *calcs_string = float_arr_to_string(calcs, calc_len);
//  size_t calcs_str_len = strlen(calcs_string);
//  char *prefix = "Sender will send: ";
//  size_t prefix_len = strlen(prefix);
//  size_t total_len = calcs_str_len + prefix_len;
//  char *result_str = (char *) malloc(sizeof(char) * 
//                      (total_len + 2));
//  memcpy(result_str, prefix, prefix_len);
//  memcpy(result_str + prefix_len, calcs_string, calcs_str_len);
//  result_str[total_len] = '\n';
//  result_str[total_len + 1] = '\n';
//  free(calcs_string);
//  safe_print(result_str, sp->mu_set->io_mu);
}

void signal_sent(SenderPack *sp) {
  pthread_mutex_lock(sp->cond_packs->sender_sent_cond_pack->mutex_to_use);
  set_cond_to_verify_to_false(sp->cond_packs->reader_calculated_cond_pack);
  set_cond_to_verify_to_true(sp->cond_packs->sender_sent_cond_pack);
  pthread_cond_signal(
      sp->cond_packs->sender_sent_cond_pack->cond_var);
  pthread_mutex_unlock(sp->cond_packs->sender_sent_cond_pack->mutex_to_use);
}
