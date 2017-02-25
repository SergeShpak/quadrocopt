#include <stdlib.h>
#include <stdio.h>

#include "include/printer.h"
#include "include/threading_stuff.h"

void create_output_files();
void write_to_files(PrinterPack *pp);

FILE *fpTemps;
FILE *fpX;
FILE *fpY;
FILE *fpZ;
FILE *fpPsy;
FILE *fpRef;


PrinterPack *initialize_printer_pack(PrinterMutexSet *mu_set, 
                        PrinterThreadCondPackets *cond_packs, BatchStock *bs) {
  PrinterPack *pp = (PrinterPack *) malloc(sizeof(PrinterPack));
  pp->mu_set = mu_set;
  pp->cond_packs = cond_packs;
  pp->bs = bs;
  return pp;
}

PrinterMutexSet *initialize_printer_mu_set(pthread_mutex_t *io_mu) {
  PrinterMutexSet *mu_set = 
                          (PrinterMutexSet *) malloc(sizeof(PrinterMutexSet));
  mu_set->io_mu = io_mu;
  return mu_set;
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

void run_printer(PrinterPack *pp) {
  create_output_files();
  while(1) {
    wait_with_pack(pp->cond_packs->calc_to_printer_signal);
    write_to_files(pp);
    signal_with_pack(pp->cond_packs->printer_signal); 
  }
}

void create_output_files() {
  fpTemps = fopen("temps.txt", "w");
  fpX = fopen("xcoord.txt", "w");
  fpY = fopen("ycoord.txt", "w");
  fpZ = fopen("zcoord.txt", "w");
  fpPsy = fopen("psy.txt", "w");
  fclose(fpTemps);
  fclose(fpX);
  fclose(fpZ);
  fclose(fpPsy);
}

void write_to_files(PrinterPack *pp) {
  fpTemps = fopen("temps.txt", "a+");
  fpX = fopen("xcoord.txt", "a+");
  fpY = fopen("ycoord.txt", "a+");
  fpZ = fopen("zcoord.txt", "a+");
  fpPsy = fopen("psy.txt", "a+");
  fprintf(fpTemps, "%f\n", pp->bs->batch[0]);
  fprintf(fpX, "%f\n", pp->bs->batch[1]);
  fprintf(fpY, "%f\n", pp->bs->batch[2]);
  fprintf(fpZ, "%f\n", pp->bs->batch[3]);
  fprintf(fpPsy, "%f\n", pp->bs->batch[4]);
  fclose(fpTemps);
  fclose(fpX);
  fclose(fpY);
  fclose(fpZ);
  fclose(fpPsy);
}
