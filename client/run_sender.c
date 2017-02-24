#include <string.h>
#include <pthread.h>
#include <stdio.h>

#include "include/sender.h"
#include "include/utils.h"
#include "include/io_stuff.h"
#include "include/utils.h"
#include "include/threading_stuff.h"

void fetch_batches(SenderPack *sp, 
                  float **first_batch, size_t *first_batch_len, 
                  float **second_batch, size_t *second_batch_len);
void send_calcs(SenderPack *sp, float *first_batch, size_t first_batch_len, 
                float *second_batch, size_t second_batch_len);
void signal_sender_ready(SenderPack *sp);

void run_sender(SenderPack *sp) {
  int rounds = 0;
  float *first_batch = NULL;
  size_t first_batch_len = 0;
  float *second_batch = NULL;
  size_t second_batch_len = 0;
  while(1) {
    rounds++;
    pthread_mutex_lock(sp->mu_set->io_mu);
    printf("[SENDER]: round %d\n", rounds);
    pthread_mutex_unlock(sp->mu_set->io_mu);
    safe_print("[SENDER]: waiting for the calculator\n", sp->mu_set->io_mu);
    wait_with_pack(sp->cond_packs->calc_to_sender_signal);
    fetch_batches(sp, &first_batch, &first_batch_len, 
                  &second_batch, &second_batch_len);
    signal_with_pack(sp->cond_packs->sender_signal);
    send_calcs(sp, first_batch, first_batch_len, 
                second_batch, second_batch_len);
    free(first_batch);
    free(second_batch);
    safe_print("[SENDER]: sent results\n", sp->mu_set->io_mu);
  } 
}

void send_calcs(SenderPack *sp, float *first_batch, size_t first_batch_len, 
                float *second_batch, size_t second_batch_len) {
  Packet *first_pack = gen_packet_from_floats(first_batch, first_batch_len);
  char *first_pack_bytes = pack_to_bytes(first_pack);
  size_t pack_size = get_pack_bytes_size(first_pack);
  int bytes_sent = sendto(sp->sd, first_pack_bytes, pack_size, 0, 
                        sp->server_addr->in_first, sp->server_addr->first_len);
  if (bytes_sent < 0) {
    exit_error("Error sending first batch\n"); 
  }
  //rand_sleep(0, 1500 * 1000);
//  pthread_mutex_lock(sp->mu_set->client_addr_mu);
//  struct sockaddr *addr = 
//                  copy_sockaddr((struct sockaddr *)sp->client_addr->addr);
//  
//  size_t addr_len = sp->client_addr->addr_len;
//  pthread_mutex_unlock(sp->mu_set->client_addr_mu);
  Packet *second_pack = gen_packet_from_floats(second_batch, second_batch_len);
  char *second_pack_bytes = pack_to_bytes(second_pack);
  pack_size = get_pack_bytes_size(second_pack);
  bytes_sent = sendto(sp->sd, second_pack_bytes, pack_size, 0, 
                    sp->server_addr->in_second, sp->server_addr->second_len); 
  if (bytes_sent < 0) {
    exit_error("Error sending second batch\n"); 
  } 
  free_pack(first_pack);
  free_pack(second_pack);
  free(first_pack_bytes);
  free(second_pack_bytes);
//  free(addr);

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

void fetch_batches(SenderPack *sp, 
                    float **first_batch, size_t *first_batch_len, 
                    float **second_batch, size_t *second_batch_len) {
  SenderStock *s = sp->stock;
  *first_batch = copy_arr_of_floats(s->first_batch, s->first_batch_len);
  *first_batch_len = s->first_batch_len;
  *second_batch = copy_arr_of_floats(s->second_batch, s->second_batch_len);
  *second_batch_len = s->second_batch_len;
}
