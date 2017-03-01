#include <stdlib.h>

#include "include/printer.h"
#include "include/threading_stuff.h"
#include "include/io_stuff.h"

void print_to_file(char *str, char *file_path, char *file_mode);
void print_to_stream(char *str, FILE *stream);

void print(PrinterStock *stock, PrinterPack *pack) {
  wait_with_pack(pack->cond_packs->calc_to_printer_signal);
  PrinterParameters *params = copy_printer_params(stock->params);
  signal_with_pack(pack->cond_packs->printer_signal);
  char *str_to_print;
  switch(params->payload_type) {
    case STRING:
      str_to_print = (char *)params->payload;
      break;
    case FLOAT_ARR:
      str_to_print = float_arr_to_string((float *)params->payload, 
                                          params->payload_len / sizeof(float));
      break;
    default:
      //TODO: exit with error
      exit(1);
  }
  switch(params->out_stream) {
    case STDOUT:
      print_to_stream(str_to_print, stdout);
      break;
    case STDERR:
      print_to_stream(str_to_print, stderr);
      break;
    case FOUT:
      print_to_file(str_to_print, params->file_path, params->open_mode);
      break;
  }
  free_printer_params(params);
  return;
}

void run_printer(PrinterPack *pp, void (*create_output_files)(void)) {
  create_output_files();
  while(1) {
    print(pp->ps, pp);
  }
}

void print_to_file(char *str, char *file_path, char *file_mode) {
  FILE *fp = fopen(file_path, file_mode);
  print_to_stream(str, fp);
  fclose(fp);
}

void print_to_stream(char *str, FILE *stream) {
  fprintf(stream, "%s\n", str);
}
