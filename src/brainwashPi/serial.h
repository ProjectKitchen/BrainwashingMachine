
#ifndef _SERIAL_H
#define _SERIAL_H


#include "signals.h"


int openSerialPort(const char * DEVICE_PORT);
void readport(int fd, struct signalData * signals);

#endif
