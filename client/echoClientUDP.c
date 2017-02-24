/* echoClientUDP.c */

/* Simple UDP echo client - tries to send everything read from stdin
   as a single datagram (MAX 1MB)*/

#include <stdio.h>      /* standard C i/o facilities */
#include <stdlib.h>     /* needed for atoi() */
#include <unistd.h>     /* defines STDIN_FILENO, system calls,etc */
#include <sys/types.h>  /* system data type definitions */
#include <sys/socket.h> /* socket specific definitions */
#include <netinet/in.h> /* INET constants and stuff */
#include <arpa/inet.h>  /* IP address conversion stuff */
#include <netdb.h>      /* gethostbyname */
#include <string.h>
#include <pthread.h>

#include "include/io_stuff.h"
#include "include/data_struct.h"
#include "include/calculate.h"
#include "include/constants.h"

udp_com *com_first_port;
udp_com *com_out_port;
udp_com *com_second_port;

int is_running = 1;

void* send_position_angle(void *buffer);
void* receive_command(void *buffer);
void init_connexion();

/* client program:

   The following must passed in on the command line:
      hostname of the server (argv[1])
      port number of the server (argv[2])
*/

int main( int argc, char **argv ) {

  if (argc!=2) {
    printf("Usage: %s <server name> \n",argv[0]);
    exit(0);
  }

  com_first_port = init_udp_com(argv[1],SERVER_IN_FIRST_PORT,MAXBUF);
  com_second_port = init_udp_com(argv[1],SERVER_IN_SECOND_PORT,MAXBUF);
  com_out_port = init_listener_udp_com(argv[1]);

  angle_coord_command *var = init_angle_coord_command(CLIENT_TO_SERVER_PARAMS_COUNT,
                CLIENT_TO_SERVER_PARAMS_COUNT,SERVER_TO_CLIENT_PARAMS_COUNT);

  init_connexion();

  pthread_t monThreadEnvoyeur;
  pthread_t monThreadRecepteur;

  pthread_create(&monThreadEnvoyeur,NULL,send_position_angle, (void *) var);
  pthread_create(&monThreadRecepteur,NULL,receive_command,(void* ) var);

  launch_calculations(var);

  is_running = 0;
  close(com_first_port->sk);
  close(com_second_port->sk);
  close(com_out_port->sk);

  pthread_join(monThreadEnvoyeur,NULL);
  pthread_join(monThreadRecepteur,NULL);

  free_udp_com(com_first_port);
  free_udp_com(com_second_port);
  free_udp_com(com_out_port);
  free_angle_coord_command(var);

  return(0);
}

void* send_position_angle(void *buffer){
  angle_coord_command *var = (angle_coord_command *) buffer;
  while(is_running){
    send_data(com_first_port,com_second_port,var);
  }
  return NULL;
}

void* receive_command (void* buffer){
  angle_coord_command *var =(angle_coord_command *) buffer;
  while(is_running){
    recv_data(com_out_port,var);
  }
  return NULL;
}

void init_connexion(){
  float *msg = (float *) (malloc(sizeof(float) * CLIENT_TO_SERVER_PARAMS_COUNT));
  for(int i=0;i<CLIENT_TO_SERVER_PARAMS_COUNT;i++){
    msg[i] = 5.0;
  }
  packet *sent = gen_packet_from_floats(msg,CLIENT_TO_SERVER_PARAMS_COUNT);
  send_unit(sent,com_first_port);
  send_unit(sent,com_second_port);
  send_unit(sent,com_out_port);
  printf("%s", "Init packet Sent: ");
  print_pack(sent);
  printf("%s", "\n");
  packet *recv1 = recv_unit(com_first_port);
  packet *recv2 = recv_unit(com_second_port);
  packet *recv3 = recv_unit(com_out_port);
  printf("%s", "Init packet Received: ");
  print_pack(recv1);
  print_pack(recv2);
  print_pack(recv3);
  printf("%s", "\n");
  free_pack(sent);
  free_pack(recv1);
  free_pack(recv2);
  free_pack(recv3);
  free(msg);
}


