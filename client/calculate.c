#include <netinet/in.h> /* INET constants and stuff */
#include <stdio.h>
#include <stdlib.h>

#include "include/data_struct.h"
#include "include/constants.h"
#include "include/nrutil.h"
#include "include/io_stuff.h"
#include "include/calculate.h"
#include "include/sender.h"

float U1 = 0.0;
float U2 = 0.0;
float U3 = 0.0;
float U4 = 0.0;

FILE *fpTemps;
FILE *fpX;
FILE *fpY;
FILE *fpZ;
FILE *fpPsy;
FILE *fpRef;

//packet *send_and_wait(packet*, int, struct sockaddr*, socklen_t);
void calc_time();
void calc_ref(float tinit, float tend, float stet);
void set_u(float *u_arr);

/*packet *send_and_wait(packet *pack_to_send, int sk, struct sockaddr *server, 
                      socklen_t server_size) {
  print_packet_to_send(pack_to_send);
  char *bytes_to_send = pack_to_bytes(pack_to_send);
  size_t len = get_pack_bytes_size(pack_to_send);
  sendto(sk, bytes_to_send, len, 0, server, server_size);
  recvfrom(sk, recv_buf, MAXBUF, 0, server, &server_size);
  packet *recv_pack = bytes_to_pack(recv_buf);
  free(bytes_to_send);
  print_received_pack(recv_pack);
  return recv_pack;
}*/

void set_u(float *u_arr) {
  U1 = u_arr[0];
  U2 = u_arr[1];
  U3 = u_arr[2];
  U4 = u_arr[3];
}


void calc_ref(float tinit, float tend, float step){
  float t = tinit;
  FILE *fpRef = fopen("ref.txt", "w");
  while(t<=tend){
    float *refTrajSt = tanRh(t,sigma,LowVal,HighVal,TRaise);
    fprintf(fpRef,"%f\n",refTrajSt[1]);
    free_vector(refTrajSt,1,5);
    t+=step;
  }
  fclose(fpRef);
}

void derivs(float t,float X[],float dXdt[], float *u) {
	//float x = X[1];
	float dotX = X[2];
	//float y = X[3];
	float dotY = X[4];
	//float z = X[5];
	float dotZ = X[6];
	float theta = X[7];
	float dotTheta = X[8];
	float phi = X[9];
	float dotPhi = X[10];
	//float psy = X[11];
	float dotPsy = X[12];

  set_u(u);//set the command
  //End of getting command

	float ddotX = theta * U1/m;
	float ddotY = -phi*U1/m;
	float ddotZ = (U1/m) -g;
	float ddotTheta = U2/Ix;
	float ddotPhi = U3/Iy;
	float ddotPsy = U4/Iz;

	dXdt[1] = dotX;
  dXdt[2] = ddotX;
  dXdt[3] = dotY;
  dXdt[4] = ddotY;
  dXdt[5] = dotZ;
  dXdt[6] = ddotZ;
	dXdt[7] = dotTheta;
  dXdt[8] = ddotTheta;
  dXdt[9] = dotPhi;
  dXdt[10] = ddotPhi;
  dXdt[11] = dotPsy;
	dXdt[12] = ddotPsy;
}
