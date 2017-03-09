#include <stdlib.h>

#include "include/io_stuff.h"
#include "include/printer.h"
#include "include/threading_stuff.h"
#include "include/utils.h"

void *fetch_msg(PrinterStock *stock);
void print_to_file(char *str, char *file_path, char *file_mode);
void print_to_stream(char *str, FILE *stream);

int print(PrinterStock *stock, PrinterPack *pack) {
  void *msg = fetch_msg(stock);
  if (NULL == msg) {
    return -1; 
  }
  PrinterParameters *params = stock->params;
  char *str_to_print;
  FloatArray *float_arr;
  switch(params->payload_type) {
    case STRING:
      str_to_print = (char *)msg;
      break;
    case FLOAT_ARR:
      float_arr = (FloatArray *)msg;
      str_to_print = float_arr_to_string(float_arr->arr, 
                                          float_arr->arr_len / sizeof(float));
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
  return 0;
}

void run_printer(PrinterPack *pp, void (*create_output_files)(void)) {
  create_output_files();
  while(1) {
    int print_status = print(pp->ps, pp);
    if (-1 == print_status) {
      break;
    }
  }
}

void stop_printer(void) {
  pthread_exit(NULL);
}

void *fetch_msg(PrinterStock *stock) {
  return NULL;
}

void print_to_file(char *str, char *file_path, char *file_mode) {
  FILE *fp = fopen(file_path, file_mode);
  print_to_stream(str, fp);
  fclose(fp);
}

void print_to_stream(char *str, FILE *stream) {
  fprintf(stream, "%s\n", str);
}
