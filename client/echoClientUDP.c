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

#include "include/io_stuff.h"
#include "include/data_struct.h"
#include "include/calculate.h"
#include "include/constants.h"

/* get_stdin reads from standard input until EOF is found,
   or the maximum bytes have been read.
*/

//int get_stdin( char *buf, int maxlen ) {
//  int i=0;
//  int n;
//
//  while ( (n=read(STDIN_FILENO,buf+i,maxlen-i)) > 0 ) {
//    i+=n;
//    if (i==maxlen) break;
//  }
//
//  if (n!=0) {
//    perror("Error reading stdin");
//    exit(1);
//  }
//
//  /* return the number of bytes read including the last read */
//  return(i);
//}


/* client program:

   The following must passed in on the command line:
      hostname of the server (argv[1])
      port number of the server (argv[2])
*/


int main( int argc, char **argv ) {
  int sk;
  struct sockaddr_in server;
  struct hostent *hp;
  char *buf = (char*) malloc(sizeof(char) * MAXBUF);

  /* Make sure we have the right number of command line args */

  if (argc!=3) {
    printf("Usage: %s <server name> <port number>\n",argv[0]);
    exit(0);
  }

  /* create a socket
     IP protocol family (PF_INET)
     UDP (SOCK_DGRAM)
  */

  if ((sk = socket( PF_INET, SOCK_DGRAM, 0 )) < 0)
    {
      printf("Problem creating socket\n");
      exit(1);
    }

  /* Using UDP we don't need to call bind unless we care what our
     port number is - most clients don't care */

  /* now create a sockaddr that will be used to contact the server

     fill in an address structure that will be used to specify
     the address of the server we want to connect to

     address family is IP  (AF_INET)

     server IP address is found by calling gethostbyname with the
     name of the server (entered on the command line)

     server port number is argv[2] (entered on the command line)
  */

  server.sin_family = AF_INET;
  if ((hp = gethostbyname(argv[1]))==0) {
    printf("Invalid or unknown host\n");
    exit(1);
  }

  /* copy the IP address into the sockaddr
     It is already in network byte order
  */

  memcpy( &server.sin_addr.s_addr, hp->h_addr, hp->h_length);

  /* establish the server port number - we must use network byte order! */
  server.sin_port = htons(atoi(argv[2]));

  launch_calculations(sk, (struct sockaddr*) &server, sizeof(server), buf);

  printf("%s\n", "Calculations finished");

  free(buf);
  return(0);
}

