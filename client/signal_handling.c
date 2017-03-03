#include <pthread.h>
#include <signal.h>

#include "include/listener.h"
#include "include/sender.h"
#include "include/printer.h"
#include "include/signal_handling.h"
#include "include/utils.h"

extern WorkersCollection *workers;

void sig_stop_worker();

void sig_handler(int signo) {
  printf("Received some signal!\n");
  switch(signo) {
    case SIGUSR1:
      sig_stop_worker();
      break;
    default:
      return;
  }
  return; 
}

void sig_stop_worker() {
  pthread_t curr_thread = pthread_self();
  if (curr_thread == *(workers->listener)) {
    printf("Stopping listener\n");
    stop_listener();
    return;
  }
  if (curr_thread == *(workers->sender)) {
    printf("Stopping sender\n");
    stop_sender();
    return;
  }
  if (curr_thread == *(workers->printer)) {
    printf("Stopping printer\n");
    stop_printer();
    return;
  }
  printf("Some error...\n");
  return;
}
