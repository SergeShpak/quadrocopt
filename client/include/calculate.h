#ifndef CALCULATE_H
#define CALCULATE_H

//#include <netinet/in.h> /* INET constants and stuff */
extern const float m, g, Ix, Iy, Iz, LowVal, HighVal, sigma, TRaise;


typedef struct _CalculatorData CalculatorData;
typedef struct _AngleCoordCommand AngleCoordCommand;

struct _CalculatorData {
  float time_start;
  float time_end;
  float step;
  float curr_time;
  
  float Vx;
  float VdotX;
  float Vy;
  float VdotY;
  float Vz;
  float VdotZ;
  float Vtheta;
  float VdotTheta;
  float Vphi;
  float VdotPhi;
  float Vpsy;
  float VdotPsy;
};

CalculatorData *initialize_calc_data(float time_start, float time_end);
void free_calc_data(CalculatorData *calc_data);
void do_next_step(CalculatorData *cald_data, 
                  AngleCoordCommand *angle_coord_command);
float *get_payload_from_calc_data(CalculatorData *calc_data);
float *get_first_batch_from_calc_data(CalculatorData *calc_data);
float *get_second_batch_from_calc_data(CalculatorData *calc_data);
void set_calc_data(CalculatorData *calc_data, float *y);

struct _AngleCoordCommand {
  float *position;
  float *angle;
  float *U;
};

AngleCoordCommand *initialize_angle_coord_command();
void free_angle_command();
void set_angle_position(AngleCoordCommand *a_com, 
                        float *position, float *angle);
void set_angle_command_u(AngleCoordCommand *angle_coord_command, float *u_vec);
void clean_angle_position(AngleCoordCommand *angle_coord_command);

void derivs(float t, float X[], float dXdt[], float *u);
void calc_ref(float tinit, float tend, float step);

#endif
