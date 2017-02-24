#include <netinet/in.h> /* INET constants and stuff */
#include <stdio.h>
#include <stdlib.h>

#include "include/data_struct.h"
#include "include/constants.h"
#include "include/nrutil.h"
#include "include/io_stuff.h"
#include "include/calculate.h"

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
  while(t<=tend){
    float *refTrajSt = tanRh(t,sigma,LowVal,HighVal,TRaise);
    fprintf(fpRef,"%f\n",refTrajSt[1]);
    free_vector(refTrajSt,1,5);
    t+=step;
  }
}

void derivs(float t, float X[], float dXdt[], AngleCoordCommand *var) {
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

  float *position = 
              (float *) malloc(sizeof(float) * CLIENT_TO_SERVER_PARAMS_COUNT);
  int i;
  for(i=0; i < CLIENT_TO_SERVER_PARAMS_COUNT - 1; i++) {
    position[i] = X[i+1];
  }
  position[CLIENT_TO_SERVER_PARAMS_COUNT - 1] = t;
  float *angle = 
              (float *) malloc(sizeof(float) * CLIENT_TO_SERVER_PARAMS_COUNT);
  for(i=0; i < CLIENT_TO_SERVER_PARAMS_COUNT - 1; i++) {
    angle[i] = X[CLIENT_TO_SERVER_PARAMS_COUNT + i];
  }
  angle[CLIENT_TO_SERVER_PARAMS_COUNT - 1] = t;
  clean_angle_position(var); 
  set_angle_position(var, position, angle);

  free(position);
  free(angle);

  set_u(var->U);

  //End of getting command

	float ddotX = theta * U1 / m;
	float ddotY = -phi * U1 / m;
	float ddotZ = (U1 / m) - g;
	float ddotTheta = U2 / Ix;
	float ddotPhi = U3 / Iy;
	float ddotPsy = U4 / Iz;

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
void getNextStep(AngleCoordCommand *var, 
                  CalculatorData *calc_data){
  float *y,*yout,*dydx;
  y = vector(1,NVAR);
  yout = vector(1,NVAR);
  dydx = vector (1,NVAR);
  y[1] = calc_data->Vx;
  y[2] = calc_data->VdotX;
  y[3] = calc_data->Vy;
  y[4] = calc_data->VdotY;
  y[5] = calc_data->Vz;
  y[6] = calc_data->VdotZ;
  y[7] = calc_data->Vtheta;
  y[8] = calc_data->VdotTheta;
  y[9] = calc_data->Vphi;
  y[10] = calc_data->VdotPhi;
  y[11] = calc_data->Vpsy;
  y[12] = calc_data->VdotPsy;
  float ti = calc_data->curr_time;

  (*derivs)(ti, y, dydx, var);

  int i;
  float xh, hh, h6, *dym, *dyt, *yt;
  float x = ti;
  float h = calc_data->step;

  dym=vector(1,NVAR);
  dyt=vector(1,NVAR);
  yt=vector(1,NVAR);
  hh=h*0.5;
  h6=h/6.0;
  xh=x+hh;
  for (i=1;i<=NVAR;i++) yt[i]=y[i]+hh*dydx[i];
  (*derivs)(xh,yt,dyt,var);
  for (i=1;i<=NVAR;i++) yt[i]=y[i]+hh*dyt[i];
  (*derivs)(xh,yt,dym,var);
  for (i=1;i<=NVAR;i++) {
    yt[i]=y[i]+h*dym[i];
    dym[i] += dyt[i];
  }
  (*derivs)(x+h,yt,dyt,var);
  for (i=1;i<=NVAR;i++)
    yout[i]=y[i]+h6*(dydx[i]+dyt[i]+2.0*dym[i]);

///g	fprintf(fpTemps,"%f\n",ti + calc_data->step);
///g	fprintf(fpX,"%f\n",yout[1]);
///g	fprintf(fpY,"%f\n",yout[3]);
///g	fprintf(fpZ,"%f\n",yout[5]);
///g	fprintf(fpPsy,"%f\n",yout[11]);
///g
	calc_data->Vx = yout[1];
  calc_data->VdotX=yout[2];
  calc_data->Vy=yout[3];
  calc_data->VdotY=yout[4];
	calc_data->Vz =yout[5];
  calc_data->VdotZ =yout[6];
  calc_data->Vtheta =yout[7];
  calc_data->VdotTheta=yout[8];
	calc_data->Vphi =yout[9];
  calc_data->VdotPhi=yout[10];
  calc_data->Vpsy=yout[11];
  calc_data->VdotPsy=yout[12];

  free_vector(yt,1,NVAR);
  free_vector(dyt,1,NVAR);
  free_vector(dym,1,NVAR);
  free_vector(yout,1,NVAR);
  free_vector(y,1,NVAR);
  free_vector(dydx,1,NVAR);
}
