
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include <termios.h>
#include "serial.h"


int savetofile = 0;

int openSerialPort(const char * DEVICE_PORT) 
{
	struct termios options;
	printf("### Open Serial Port\n");
	int ComFileHandle = open(DEVICE_PORT, O_RDONLY | O_NOCTTY | O_NDELAY);
	if (ComFileHandle == -1)
	{
		perror("### Unable to open serial port");
		return (-1);
	}
	printf("### Serial port opened! ID %i \n", ComFileHandle);
	tcgetattr(ComFileHandle, &options);
	if (strcmp(DEVICE_PORT, "/dev/ttyACM0")==0) {
		options.c_cflag = B115200 | CS8 | CREAD; // Arduino
	} else {
		options.c_cflag = B57600 | CS8 | CREAD; // OpenEEG
	}
	options.c_iflag = 0;
	options.c_oflag = OPOST | ONLCR;
	options.c_lflag = 0;
	tcsetattr(ComFileHandle, TCSAFLUSH, &options);
	
	return(ComFileHandle);
}

void readport(int fd, struct signalData * signals)
{ 
	if (fd == -1) 
	{
		printf("### Cannot read because serial port is not open \n");
		return;
	} 
	else 
	{
		unsigned char data[256];
		char state = 0;
		unsigned char actbyte;
		int tryread=0;
			
		int readActive = 1;
		uint8_t highbyte1 = 0, lowbyte1 = 0, highbyte2 = 0, lowbyte2 = 0;
		char dummy=0;
		int success=0;
		
		while(readActive)
		{
			success=read(fd, (void*)data, 1); 
			if (success == -1)
			{ 
			   tryread++; if (tryread>=5000) readActive=0;
			   // printf("read returned: -1 - wait for new data");
			   usleep(1000);
		    }
			else if (success==0)
			{
				tryread++; if (tryread>=1000) readActive=0;
 			    printf("read returned 0 (EOF) - restart file read\n");
				lseek(fd,0,SEEK_SET);
			}
			else {
				tryread=0;

				actbyte = data[0];
				if (savetofile) {
					//fprintf(outputfile, "%x", actbyte);  //**** do not overwrite my file !!
				}				
				// printf("read=%x\n", actbyte);  
				switch (state) {
					case 0: if (actbyte == 0xa5) state ++;	// Sync Value
							break;
					case 1: if (actbyte == 0x5a) state ++;  // Sync Value
							else state = 0;
							break;
					case 2: state++;			// Version Number
							break;
					case 3: state++; 				// Framenumber
							break;
					case 4: highbyte1 = actbyte; 	// Channel 1 - Highbyte
							state++;
							break;
					case 5:	lowbyte1 = actbyte; 	// Channel 1 - Lowbyte
							state++;
							break;
					case 6: highbyte2 = actbyte;	// Channel 2 - Highbyte
							state++;
							break;
					case 7:	lowbyte2 = actbyte;		// Channel 2 - Lowbyte
							state++;
							dummy=0;
							break;
					// We do not need more than 2 channels!
					case 8: 
						if (++dummy==8) {
							signals->chn1Raw = highbyte1 * 256 + lowbyte1 - 512;
							signals->chn2Raw = highbyte2 * 256 + lowbyte2 - 512;
							readActive = 0; 
						}
						break;
				}
			}

		}
	}		
}
