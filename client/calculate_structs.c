#include <stdlib.h>

#include "include/calculate.h"
#include "include/constants.h"
#include "include/utils.h"

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
const float TRaise = 5;

CalculatorData *initialize_calc_data(float time_start, float time_end) {
  CalculatorData *calc_data = 
                            (CalculatorData *) malloc(sizeof(CalculatorData));
  calc_data->time_start = time_start;
  calc_data->time_end = time_end;
  calc_data->curr_time = time_start;
  calc_data->step = (time_end - time_start) / NSTEP;
  
  calc_data->Vx = 0.0;
  calc_data->VdotX = 0.0;
  calc_data->Vy = 0.0;
  calc_data->VdotY = 0.0;
  calc_data->Vz = 0.0;
  calc_data->VdotZ = 0.0;
  calc_data->Vtheta = 0.0;
  calc_data->VdotTheta = 0.0;
  calc_data->Vphi = 0.0;
  calc_data->VdotPhi = 0.0;
  calc_data->Vpsy = 0.0;
  calc_data->VdotPsy = 0.0;
  return calc_data;
}

void free_calc_data(CalculatorData *calc_data) {
  free(calc_data);
}

void do_next_step(CalculatorData *calc_data, 
                  AngleCoordCommand *angle_coord_command) {
  getNextStep(angle_coord_command, calc_data);
  calc_data->curr_time += calc_data->step;
}

float *get_first_batch_from_calc_data(CalculatorData *calc_data) {
  float *batch = (float *) malloc(sizeof(float) * CLIENT_FIRST_BATCH_LEN);
  batch[0] = calc_data->Vx;
  batch[1] = calc_data->VdotX;
  batch[2] = calc_data->Vy;
  batch[3] = calc_data->VdotY;
  batch[4] = calc_data->Vz;
  batch[5] = calc_data->VdotZ;
  return batch;
}

float *get_second_batch_from_calc_data(CalculatorData *calc_data) {
  float *batch = (float *) malloc(sizeof(float) * CLIENT_SECOND_BATCH_LEN);
  batch[0] = calc_data->Vtheta;
  batch[1] = calc_data->VdotTheta;
  batch[2] = calc_data->Vphi;
  batch[3] = calc_data->VdotPhi;
  batch[4] = calc_data->Vpsy;
  batch[5] = calc_data->VdotPsy;
  batch[6] = calc_data->curr_time;
  return batch;
}


AngleCoordCommand *initialize_angle_coord_command() {
  AngleCoordCommand *angle_coord_command 
                    = (AngleCoordCommand *) malloc(sizeof(AngleCoordCommand));
  angle_coord_command->U = NULL;
  angle_coord_command->position = NULL;
  angle_coord_command->angle = NULL;
  return angle_coord_command;
}

void free_angle_command(AngleCoordCommand *angle_coord_command) {
  free(angle_coord_command);
}

void set_angle_position(AngleCoordCommand *angle_coord_command,
                        float *position, float *angle) {
  angle_coord_command->position = copy_arr_of_floats(position, 
                                            CLIENT_TO_SERVER_PARAMS_COUNT);
  angle_coord_command->angle = copy_arr_of_floats(angle, 
                                            CLIENT_TO_SERVER_PARAMS_COUNT);
}

void set_angle_command_u(AngleCoordCommand *angle_coord_command, 
                          float *u_vec) {
  angle_coord_command->U = copy_arr_of_floats(u_vec, 
                                              SERVER_TO_CLIENT_PARAMS_COUNT);
}

void clean_angle_position(AngleCoordCommand *angle_coord_command) {
  free(angle_coord_command->position);
  free(angle_coord_command->angle);
}
