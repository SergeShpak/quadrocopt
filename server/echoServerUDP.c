/* UDP echo server program -- echoServerUDP.c */

#include <stdio.h>      /* standard C i/o facilities */
#include <stdlib.h>     /* needed for atoi() */
#include <unistd.h>     /* defines STDIN_FILENO, system calls,etc */
#include <sys/types.h>  /* system data type definitions */
#include <sys/socket.h> /* socket specific definitions */
#include <netinet/in.h> /* INET constants and stuff */
#include <arpa/inet.h>  /* IP address conversion stuff */
#include <netdb.h>      /* gethostbyname */

#include "include/io_stuff.h"
#include "include/data_struct.h"
#include "include/calculate.h"
#include "include/constants.h"

/* this routine echos any messages (UDP datagrams) received */

#define MAXBUF 1024*1024

void print_received_pack(packet *pack);
void print_packet_to_send(packet *pack);

void send_calculation_result(float *calc_result, int sd, 
                            struct sockaddr *client, socklen_t client_len) {
  packet *pack_to_send = gen_packet_from_floats(calc_result, 
                                  SERVER_TO_CLIENT_PARAMS_COUNT);
  size_t pack_len = get_pack_bytes_size(pack_to_send);
  char *bytes_to_send = pack_to_bytes(pack_to_send);
  print_packet_to_send(pack_to_send);
  sleep(0);
  sendto(sd, bytes_to_send, pack_len, 0, client, client_len);
  free_pack(pack_to_send);
  free(bytes_to_send);
}

void run_server(int sd) {
    unsigned int len,                    // source address size
        n;                      // number of bytes received
    char bufin[MAXBUF];         // here the message is stored
    struct sockaddr_in remote;  // source address

    /* need to know how big address struct is, len must be set before the
       call to recvfrom!!! */

    len = sizeof(remote);

    while (1) {
      
      /* read a datagram from the socket (put result in bufin) */
      n = recvfrom(sd, bufin, MAXBUF, 0, (struct sockaddr *)&remote, &len);
      if (n < 0) {
        perror("Error receiving data");
        exit(1);
      }

      packet *received_pack = bytes_to_pack(bufin);
      print_received_pack(received_pack);

      /* print out the address of the sender */
      printf("Got a datagram from %s port %d\n",
             inet_ntoa(remote.sin_addr), ntohs(remote.sin_port));

      float *calc_result = calculate(received_pack);
      send_calculation_result(calc_result, sd, 
                                (struct sockaddr*) &remote, len);
      free_pack(received_pack);
      free(calc_result);
    }
}

/* server main routine */

int main() {
  int ld;
  struct sockaddr_in skaddr;
  unsigned int length;

  /* create a socket
     IP protocol family (PF_INET)
     UDP protocol (SOCK_DGRAM)
  */

  if ((ld = socket( PF_INET, SOCK_DGRAM, 0 )) < 0) {
    printf("Problem creating socket\n");
    exit(1);
  }

  /* establish our address
     address family is AF_INET
     our IP address is INADDR_ANY (any of our IP addresses)
     the port number is assigned by the kernel
  */

  skaddr.sin_family = AF_INET;
  skaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  skaddr.sin_port = htons(0);

  if (bind(ld, (struct sockaddr *) &skaddr, sizeof(skaddr))<0) {
    printf("Problem binding\n");
    exit(0);
  }

  /* find out what port we were assigned and print it out */

  length = sizeof( skaddr );
  if (getsockname(ld, (struct sockaddr *) &skaddr, &length)<0) {
    printf("Error getsockname\n");
    exit(1);
  }

  /* port number's are network byte order, we have to convert to
     host byte order before printing !
  */
  printf("The server UDP port number is %d\n", ntohs(skaddr.sin_port));
   
  /* Go echo every datagram we get */
  run_server(ld);
  
  return(0);
}

void print_packet_to_send(packet *pack) {
  printf("%s", "Packet to send: ");
  print_pack(pack);
  printf("%s", "\n");
}

void print_received_pack(packet *pack) {
  printf("%s", "Received packet: ");
  print_pack(pack);
  printf("%s", "\n");
}
