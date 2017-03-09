#ifndef PRINTER_H
#define PRINTER_H

#include <stdio.h>

#include "threading_stuff.h"
#include "utils.h"

typedef enum _OutputStream OutputStream;
typedef enum _ParametersPayloadType ParametersPayloadType;

typedef struct _PrinterPack PrinterPack;
typedef struct _PrinterMutexSet PrinterMutexSet;
typedef struct _PrinterThreadCondPackets PrinterThreadCondPackets;
typedef struct _PrinterParameters PrinterParameters;
typedef struct _PrinterStock PrinterStock;

/******************************************************************************
 * Enums section  *************************************************************
 *****************************************************************************/

enum _OutputStream {
  STDOUT,
  STDERR,
  FOUT
};

enum _ParametersPayloadType {
  STRING,
  FLOAT_ARR,
};

/******************************************************************************
 * End of enums section *******************************************************
 *****************************************************************************/


struct _PrinterPack {
  PrinterMutexSet *mu_set;
  PrinterThreadCondPackets *cond_packs;
  PrinterStock *ps;
};

PrinterPack *initialize_printer_pack(PrinterMutexSet *mu_set, 
                      PrinterThreadCondPackets *cond_packs, PrinterStock *ps);
void free_printer_pack(PrinterPack *pack);


struct _PrinterMutexSet {
  pthread_mutex_t *io_mu;
};

PrinterMutexSet *initialize_printer_mu_set(pthread_mutex_t *io_mu);
void free_printer_mutex_set(PrinterMutexSet *mu_set);


struct _PrinterThreadCondPackets {
  ThreadConditionPack *printer_signal;
  ThreadConditionPack *calc_to_printer_signal;
};

PrinterThreadCondPackets *initialize_printer_thread_cond_packs(
                                  ThreadConditionPack *printer_signal, 
                                  ThreadConditionPack *calc_to_printer_signal);
void free_printer_thread_cond_packs(PrinterThreadCondPackets *cond_packs);


struct _PrinterParameters {
  OutputStream out_stream;
  ParametersPayloadType payload_type;
  char *file_path;
  char *open_mode;
};

PrinterParameters *initialize_printer_params(OutputStream out_stream, 
                    ParametersPayloadType payload_type, const char *file_path, 
                    char *open_mode);
void free_printer_params(PrinterParameters *printer_params);
PrinterParameters *copy_printer_params(PrinterParameters *params);


struct _PrinterStock {
  PrinterParameters *params;
  DoubleLinkedList *message_queue; 
};

PrinterStock *initialize_printer_stock();
void set_printer_stock(PrinterStock *stock, PrinterParameters *params);
void add_message_to_printer_queue(PrinterStock *stock, void *msg);
void free_printer_stock(PrinterStock *stock, void (*free_func)(void*));


int print(PrinterStock *stock, PrinterPack *pack);
void run_printer(PrinterPack *pp, void (*create_output_files)(void));
void stop_printer(void);

#endif
