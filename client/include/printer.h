#ifndef PRINTER_H
#define PRINTER_H

#include "threading_stuff.h"
#include "data_struct.h"

typedef struct _PrinterPack PrinterPack;
typedef struct _PrinterMutexSet PrinterMutexSet;
typedef struct _PrinterThreadCondPackets PrinterThreadCondPackets;

struct _PrinterPack {
  PrinterMutexSet *mu_set;
  PrinterThreadCondPackets *cond_packs;
  BatchStock *bs;
};

struct _PrinterMutexSet {
  pthread_mutex_t *io_mu;
};

struct _PrinterThreadCondPackets {
  ThreadConditionPack *printer_signal;
  ThreadConditionPack *calc_to_printer_signal;
};

PrinterPack *initialize_printer_pack(PrinterMutexSet *mu_set, 
                        PrinterThreadCondPackets *cond_packs, BatchStock *bs);
PrinterMutexSet *initialize_printer_mu_set(pthread_mutex_t *io_mu);
PrinterThreadCondPackets *initialize_printer_thread_cond_packs(
                                  ThreadConditionPack *printer_signal, 
                                  ThreadConditionPack *calc_to_printer_signal);

void run_printer(PrinterPack *pp);
#endif
