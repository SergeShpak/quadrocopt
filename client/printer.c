#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "include/printer.h"
#include "include/threading_stuff.h"

void *copy_buf(const void *buf, size_t buf_len);
char *copy_str(const char *src);

PrinterPack *initialize_printer_pack(PrinterMutexSet *mu_set, 
                      PrinterThreadCondPackets *cond_packs, PrinterStock *ps) {
  PrinterPack *pp = (PrinterPack *) malloc(sizeof(PrinterPack));
  pp->mu_set = mu_set;
  pp->cond_packs = cond_packs;
  pp->ps = ps;
  return pp;
}

void free_printer_pack(PrinterPack *pack) {
  free_printer_mutex_set(pack->mu_set);
  free_printer_thread_cond_packs(pack->cond_packs);
  free(pack);
}


PrinterMutexSet *initialize_printer_mu_set(pthread_mutex_t *io_mu) {
  PrinterMutexSet *mu_set = 
                          (PrinterMutexSet *) malloc(sizeof(PrinterMutexSet));
  mu_set->io_mu = io_mu;
  return mu_set;
}

void free_printer_mutex_set(PrinterMutexSet *mu_set) {
  free(mu_set);
}


PrinterThreadCondPackets *initialize_printer_thread_cond_packs(
                                ThreadConditionPack *printer_signal, 
                                ThreadConditionPack *calc_to_printer_signal) {
  PrinterThreadCondPackets *cond_packs =
        (PrinterThreadCondPackets *) malloc(sizeof(PrinterThreadCondPackets));
  cond_packs->printer_signal = printer_signal;
  cond_packs->calc_to_printer_signal = calc_to_printer_signal;
  return cond_packs;
}

void free_printer_thread_cond_packs(PrinterThreadCondPackets *cond_packs) {
  free(cond_packs);
}


PrinterParameters *initialize_printer_params(OutputStream out_stream,
                    ParametersPayloadType payload_type, const char *file_path,
                          char *open_mode, void *payload, int payload_len) {
  PrinterParameters *params = 
                      (PrinterParameters *) malloc(sizeof(PrinterParameters));
  params->out_stream = out_stream;
  params->payload_type = payload_type;
  params->file_path = NULL;
  params->open_mode = NULL;
  params->payload = NULL;
  params->payload_len = -1;
  if (NULL != file_path) {
    params->file_path = copy_str(file_path);
  }
  if (NULL != open_mode) {
    params->open_mode = copy_str(open_mode); 
  }
  if (NULL != payload) {
    params->payload = copy_buf(payload, payload_len);
    params->payload_len = payload_len;
  }
  return params;
}

void free_printer_params(PrinterParameters *params) {
  free(params->file_path);
  free(params->open_mode);
  free(params->payload);
  free(params);
}

PrinterParameters *copy_printer_params(PrinterParameters *params) {
  PrinterParameters *dupl_params = 
                      (PrinterParameters *) malloc(sizeof(PrinterParameters));
  dupl_params->payload_type = params->payload_type;
  dupl_params->out_stream = params->out_stream;
  dupl_params->payload_len = params->payload_len;
  dupl_params->payload = copy_buf(params->payload, params->payload_len);
  dupl_params->file_path = copy_str(params->file_path);
  dupl_params->open_mode = copy_str(params->open_mode);
  return dupl_params; 
}


PrinterStock *initialize_printer_stock() {
  PrinterStock *stock = (PrinterStock *) malloc(sizeof(PrinterStock));
  stock->params = NULL;
  return stock;
}

void set_printer_stock(PrinterStock *stock, PrinterParameters *params) {
  stock->params = initialize_printer_params(params->out_stream, 
                    params->payload_type, params->file_path, params->open_mode, 
                    params->payload, params->payload_len);
}

void free_printer_stock(PrinterStock *stock) {
  free_printer_params(stock->params);
  free(stock);
}


void *copy_buf(const void *buf, size_t buf_len) {
  void *copy = malloc(buf_len);
  memcpy(copy, buf, buf_len);
  return copy; 
}

char *copy_str(const char *src) {
  size_t str_len = strlen(src);
  void *copy = copy_buf(src, sizeof(char) * (str_len + 1));
  return (char *) copy;
}
