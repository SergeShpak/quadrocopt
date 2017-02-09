#include <stdlib.h>

#include "include/data_struct.h"
#include "include/constants.h"
#include "include/nrutil.h"

const float m = 0.53;
const float g = 9.81;
const float Ix = 0.00623;
const float Iy = 0.00623;
const float Iz = 0.012;

const float LowVal = 0;
const float HighVal = 10;
const float sigma = 1;
const float TRaise = 5 ;

const float L0 = 810000;
const float L1 = 108000;
const float L2 = 5400;
const float L3 = 120;

const float L0z = 30;
const float L1z = 30;

const float L0psy = 125;
const float L1psy = 125;


float *calculate_ordinary(packet *pack);
float *server_calculations(float *params);

float *calculate(packet *pack) {
  float *result;
  switch (pack->type) {
    case PACK_OF_FLOATS:
      result = calculate_ordinary(pack);
      break;
    default:
      exit(1);
  }
  return result;
}

float *calculate_ordinary(packet *pack) {
  float *params = get_floats(pack);
  float *output = server_calculations(params);
  free(params);
  return output;  
}

float *server_calculations(float *params) {
  	// Received parameters 
  	float x = params[0];
	float dotX = params[1];
	float y = params[2];
	float dotY = params[3];
	float z = params[4];
	float dotZ = params[5];
	float theta = params[6];
	float dotTheta = params[7];
	float phi = params[8];
	float dotPhi = params[9];
	float psy = params[10];
	float dotPsy = params[11];
	float t = params[12];

  // Récupérer les valeurs de référence

	float *refTrajSt = tanRh(t,sigma,LowVal,HighVal,TRaise);
	float ref = refTrajSt[1];
	float dotref = refTrajSt[2];
	float ddotref = refTrajSt[3];
	float d3dotref = refTrajSt[4];
	float d4dotref = refTrajSt[5];

	//fprintf(fpRef,"%f\n",ref);
	//printf("%f;%f;%f;%f;%f",ref,dotref,ddotref,d3dotref,d4dotref);

	// Expression de U1:

	float ez = z - ref;
	float dotEz = dotZ - dotref;
	float V3 = ddotref - L0z*ez - L1z*dotEz;
	float U1 = m*(g + V3);

	// Calcul dotV3, dotU1 et ddotV3 : (nécessaire pour le calcul)

	float dotV3 = d3dotref - L0z*dotEz - L1z*(-g + U1/m - ddotref); 
	float dotU1 = m*dotV3;
	float ddotV3 = d4dotref - L0z*(-g + U1/m - ddotref) - L1z*(dotU1/m - d3dotref);

	// Expression de U2:

	float ddotX = (theta*U1)/m;
	float d3dotX = (dotTheta*U1 + theta*dotU1)/m;

	float ex = x - ref;
	float dotEx = dotX - dotref;
	float ddotEx = ddotX - ddotref;
	float d3dotEx = d3dotX - d3dotref; 

	float V1 = d4dotref - L0*ex - L1*dotEx - L2*ddotEx - L3*d3dotEx;
	float U2 = (Ix/(g + V3))*(V1 - 2*dotTheta*dotV3 - theta*ddotV3);

	// Expression de U3 :

	float ddotY = - (phi*U1)/m;
	float d3dotY = - (dotPhi*U1 + phi*dotU1)/m;

	float ey = y - ref;
	float dotEy = dotY - dotref;
	float ddotEy = ddotY - ddotref;
	float d3dotEy = d3dotY - d3dotref;

	float V2 = d4dotref - L0*ey - L1*dotEy - L2*ddotEy - L3*d3dotEy;
	float U3 = -(Iy/(g+V3))*(V2 + 2*dotPhi*dotV3 + phi*ddotV3);

	// Expression de U4 :
	float epsy = psy - ref;
	float dotEpsy = dotPsy - dotref;

	float V4 = ddotref - L0psy*epsy - L1psy*dotEpsy;
	float U4 = Iz*V4;

	float *res = (float *) malloc(sizeof(float) * 4);
	res[0]= U1;
	res[1] = U2;
	res[2] = U3;
	res[3] = U4;

	free_vector(refTrajSt,1,5);
	return res;
}
