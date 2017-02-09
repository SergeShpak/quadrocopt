#ifndef CALCULATE_H
#define CALCULATE_H

#include <netinet/in.h> /* INET constants and stuff */

void launch_calculations(int sk, struct sockaddr*, socklen_t, char *recv_buf);

#endif
