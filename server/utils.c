#include <stdlib.h>
#include <stdio.h>

void exit_error(char *err_msg) {
  fprintf(stderr, "%s", err_msg);
  exit(1); 
}
