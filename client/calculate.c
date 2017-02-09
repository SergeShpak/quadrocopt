#include <netinet/in.h> /* INET constants and stuff */
#include <stdio.h>
#include <stdlib.h>

#include "include/data_struct.h"
#include "include/constants.h"
#include "include/nrutil.h"
#include "include/io_stuff.h"
#include "include/calculate.h"

const float m = 0.53;
const float g = 9.81;

const float tinit = 0.0;
const float tend = 10.0;

const float Ix = 0.00623;
const float Iy = 0.00623;
const float Iz = 0.012;

const float LowVal = 0;
const float HighVal = 10;
const float sigma = 1;
const float TRaise = 5 ;

// Intial state values
float Vx = 0.0;
float VdotX = 0;
float Vy = 0;
float VdotY = 0;
float Vz = 0.0;
float VdotZ = 0;
float Vtheta = 0;
float VdotTheta = 0;
float Vphi = 0;
float VdotPhi = 0;
float Vpsy = 0;
float VdotPsy = 0;

float step;
float curr_time;

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

char *recv_buf;

packet *send_and_wait(packet*, int, struct sockaddr*, socklen_t);
void create_output_files();
void calc_time();
packet *get_packet_from_var(float t , float X[]);
void getNextStep(float time,int sk, struct sockaddr *server, size_t server_size);
void calc_ref();
void set_u(float *u_arr);
void print_packet_to_send(packet *pack);
void print_received_pack(packet *pack);

void launch_calculations(int sk, struct sockaddr *server, 
                          socklen_t server_size, char *buf) {
  step = (tend - tinit) / NSTEP;
  curr_time = tinit;
  recv_buf = buf;
  create_output_files();
  calc_ref();
  while(curr_time <= tend) {
    getNextStep(curr_time,sk,server,server_size);
    calc_time();
  } 
}

packet *send_and_wait(packet *pack_to_send, int sk, struct sockaddr *server, 
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
}

void set_u(float *u_arr) {
  U1 = u_arr[0];
  U2 = u_arr[1];
  U3 = u_arr[2];
  U4 = u_arr[3];
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

void create_output_files() {
  fpTemps = fopen("temps.txt", "w");
  fpX = fopen("xcoord.txt", "w");
  fpY = fopen("ycoord.txt", "w");
  fpZ = fopen("zcoord.txt", "w");
  fpPsy = fopen("psy.txt", "w");
  fpRef = fopen("ref.txt", "w");
}

void calc_time() {
  curr_time += step;
}

packet *get_packet_from_var(float t , float X[]){
  float *params_init = (float *) malloc(
                              sizeof(float) * CLIENT_TO_SERVER_PARAMS_COUNT);
  for(int i=0;i<NVAR;i++){
    params_init[i] = X[i+1];
  }
  params_init[NVAR] = t;
  packet *pack = gen_packet_from_floats(params_init,CLIENT_TO_SERVER_PARAMS_COUNT);
  free(params_init);
  return pack;
}

void calc_ref(){
  float t = tinit;
  while(t<=tend){
    float *refTrajSt = tanRh(t,sigma,LowVal,HighVal,TRaise);
    fprintf(fpRef,"%f\n",refTrajSt[1]);
    free_vector(refTrajSt,1,5);
    t+=step;
  }
}

void derivs(float t,float X[],float dXdt[],int sk, struct sockaddr *server, 
                          socklen_t server_size) {
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

  //We get via UDP the commands U1,U2,U3,U4
  packet *pack = get_packet_from_var(t,X);
  packet *recv = send_and_wait(pack, sk, server, server_size);
  float *u = get_floats(recv);
  set_u(u);//set the command
  free(u);
  free_pack(pack);
  free_pack(recv);
  //End of getting command

	float ddotX = theta*U1/m;
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

//extern float **y,*xx;   /* referencing declaration */
void getNextStep(float ti,int sk, struct sockaddr *server, size_t server_size){
  float *y,*yout,*dydx;
  y = vector(1,NVAR);
  yout = vector(1,NVAR);
  dydx = vector (1,NVAR);
  y[1] = Vx;
  y[2] = VdotX;
  y[3] = Vy;
  y[4] = VdotY;
  y[5] = Vz;
  y[6] = VdotZ;
  y[7] = Vtheta;
  y[8] = VdotTheta;
  y[9] = Vphi;
  y[10] = VdotPhi;
  y[11] = Vpsy;
  y[12] = VdotPsy;

  (*derivs)(ti,y,dydx,sk,server,server_size);

  int i;
  float xh,hh,h6,*dym,*dyt,*yt;
  float x = ti;
  float h = step;

  dym=vector(1,NVAR);
  dyt=vector(1,NVAR);
  yt=vector(1,NVAR);
  hh=h*0.5;
  h6=h/6.0;
  xh=x+hh;
  for (i=1;i<=NVAR;i++) yt[i]=y[i]+hh*dydx[i];
  (*derivs)(xh,yt,dyt,sk,server,server_size);
  for (i=1;i<=NVAR;i++) yt[i]=y[i]+hh*dyt[i];
  (*derivs)(xh,yt,dym,sk,server,server_size);
  for (i=1;i<=NVAR;i++) {
    yt[i]=y[i]+h*dym[i];
    dym[i] += dyt[i];
  }
  (*derivs)(x+h,yt,dyt,sk,server,server_size);
  for (i=1;i<=NVAR;i++)
    yout[i]=y[i]+h6*(dydx[i]+dyt[i]+2.0*dym[i]);

	fprintf(fpTemps,"%f\n",ti+step);
	fprintf(fpX,"%f\n",yout[1]);
	fprintf(fpY,"%f\n",yout[3]);
	fprintf(fpZ,"%f\n",yout[5]);
	fprintf(fpPsy,"%f\n",yout[11]);

	Vx = yout[1];VdotX=yout[2];Vy=yout[3];VdotY=yout[4];
	Vz =yout[5],VdotZ =yout[6];Vtheta =yout[7];VdotTheta=yout[8];
	Vphi =yout[9];VdotPhi=yout[10];Vpsy=yout[11];VdotPsy=yout[12];

  free_vector(yt,1,NVAR);
  free_vector(dyt,1,NVAR);
  free_vector(dym,1,NVAR);
  free_vector(yout,1,NVAR);
  free_vector(y,1,NVAR);
  free_vector(dydx,1,NVAR);
}
