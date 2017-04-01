#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <signal.h>
#include <bcm2835.h>

#include "draw.h"
#include "midi.h"
#include "serial.h"
#include "filters.h"
#include "signals.h"


// Set the pin for button that simulates C
#define PIN_WHEELBUTTON RPI_GPIO_P1_11  // pin 11, GPIO 17
#define PIN_STOPBUTTON  27 // RPI_GPIO_P1_13  // pin 13, GPIO 27
#define PIN_STARTBUTTON RPI_GPIO_P1_15  // pin 15, GPIO 22

int USE_GPIO = 1;
int USE_AUDIO = 1;
int USE_GRAPHICS = 1;

#define MODE_ARDUINO 0
#define MODE_OPENEEG 1
#define MODE_OPENEEG_BT 2
#define MODE_ARCHIVE 3
#define MODE_MUSICPLAYER 4
#define MODE_EXIT 5

int modeselect = 2; // 0 -> Arduino, 1 -> openeeg, 2 -> file "neuro_file.txt"   3 -> musicplayer 

// VARS for Program settings and signal data
struct Prog ActProgram = { 1,1,1   , 1,2,3,  0,0,0,  1,2,2,3,3,0, 1,0,0, 0,0,0, 0 };
struct signalData Signals = { 0,0,0,0,0,0,0,0,0,0,0, 0.2 };	


struct termios orig_term_attr;
struct termios new_term_attr;

int fd = -1;
const char *fileurl = "neuro_file.txt";
FILE *outputfile = 0;

int need_screen_init=1;
int configscreen = 0; // for first round go to settings

int program = 0;
int selected_channel = 1; // we have chl 1-3
int changeprogram = 0;
int channel = 1;

int key;
int running = 1;

// set stdin to nonblocking, needed for keyboard input

void setStdinNonblocking() {

    /* set the terminal to raw mode */
    tcgetattr(fileno(stdin), &orig_term_attr);
    memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
    new_term_attr.c_lflag &= ~(ECHO|ICANON);
    new_term_attr.c_cc[VTIME] = 0;
    new_term_attr.c_cc[VMIN] = 0;
    tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);
}

// disable non blocking when ctrl-c is called

void resetStdin()
{
    /* restore the original terminal attributes */
//    tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);
    new_term_attr.c_lflag |= (ECHO|ICANON);
    new_term_attr.c_cc[VTIME] = 20;
    new_term_attr.c_cc[VMIN] = 10;
    tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);
}

void init_audio()
{		
		printf("  Init Sound System...\n");
		system ("qsynth &");
		usleep(4000000);
		system ("aconnect 24 128");

		printf("  Open Midi Device...\n");
		// open midi device
		midiDevice = openMidiDevice();
		if (midiDevice) 
		{ 
			printf("  Midi Device opened.\n");
			preparePitchBend(midiDevice, 1);
			preparePitchBend(midiDevice, 2);
			preparePitchBend(midiDevice, 3);
			midiVolume(midiDevice, 127, 1);
			midiVolume(midiDevice, 127, 2);
			midiVolume(midiDevice, 127, 3);					
		}
		else	printf("  could not open Midi Device.\n");
		//system ("wmctrl -a brainwashPi");	  // change window focus to shell window (for keyboard input)
}

void shutdown_audio()
{
	soundoff();
	closeMidiDevice(midiDevice); // Close the midi device
	system("sudo pkill qsynth");
	usleep(20000);
}

// exit handler
void exithandler() {

    printf("Prepare to exit: freeing all resources !\n");
    if (USE_GRAPHICS) exitDrawingScreen();
	if (USE_GPIO) bcm2835_close(); // Close GPIO Input
	if (USE_AUDIO) shutdown_audio(); 
	resetStdin(); // reset standard input
    free_filters();
	exit(0);
}

int washwheel_changed()
{
	static int first=1;
	static int actlevel=0;
	int ret=0;
	
	if (USE_GPIO ==0) return(0);
	
	if(first) {	
		first=0;
		actlevel=	bcm2835_gpio_lev(PIN_WHEELBUTTON);
		return(0);
	}
	
	if ((actlevel != bcm2835_gpio_lev(PIN_WHEELBUTTON)) && (actlevel==HIGH)) 
		ret=1;
	
	actlevel=	bcm2835_gpio_lev(PIN_WHEELBUTTON);
	return(ret);
}


int startbutton_pressed()
{
	static int first=1;
	static int actlevel=0;
	int ret=0;
	
	if (USE_GPIO ==0) return(0);
	
	if(first) {	
		first=0;
		actlevel=	bcm2835_gpio_lev(PIN_STARTBUTTON);
		return(0);
	}
	
	if ((actlevel != bcm2835_gpio_lev(PIN_STARTBUTTON)) && (actlevel==HIGH)) 
		ret=1;
	
	actlevel=	bcm2835_gpio_lev(PIN_STARTBUTTON);
	return(ret);
}

int stopbutton_pressed()
{
	static int first=1;
	static int actlevel=0;
	int ret=0;
	
	if (USE_GPIO ==0) return(0);
	
	if(first) {	
		first=0;
		actlevel=	bcm2835_gpio_lev(PIN_STOPBUTTON);
		return(0);
	}
	
	if ((actlevel != bcm2835_gpio_lev(PIN_STOPBUTTON)) && (actlevel==HIGH)) 
		ret=1;
	
	actlevel=	bcm2835_gpio_lev(PIN_STOPBUTTON);
	return(ret);
}


void handle_configscreen()
{
	int save_program=-1;
	int load_program=-1;
	
	if (need_screen_init)
	{				
		need_screen_init=0;
		// disable sound
		// if (USE_AUDIO) soundoff();
		
		printf("switched to Configscreen.\n");
		if (USE_GRAPHICS) draw_configsettings(&ActProgram, selected_channel);
	    else {
			printf ("  ch1-mode: %i\n",ActProgram.ch1_mode);
		}		
		return;
	}
	
	// Capture GPIO for program change
	if (washwheel_changed()) {
		if (program < 4) program++;
		else program = 0;
		usleep(10000);
		load_program=program;
	}

	// key = getchar();
	key = fgetc(stdin);

	// check if user decided to leave config screen
	if (key == ' ') 
	{
		configscreen = 0;
		need_screen_init = 1;
		usleep(200000);
		return;
	}
	 
	if ((key!=0) && (key != -1))  // there was a user action on keyboard !
	{ 
		switch(key) {
			// n: next program
			case 'n': 
				if (program == 4) program = 0;
				else program++;
				load_program=program;
				break;
			// p: previous program
			case 'p': 
				if (program == 0) program = 4;
				else program--;
				load_program=program;
				break;
			// up/down: select feedback channel
			case 65:
				selected_channel--; 
				if (selected_channel<=0) selected_channel=3;
				break;
			case 66:
				selected_channel++; 
				if (selected_channel>=4) selected_channel=1;
				break;
			// a: change audio mode
			case 'a': 
				if (selected_channel == 1) {
					if (ActProgram.ch1_mode == 3) {
						ActProgram.ch1_mode = 1; 
					} else {
						ActProgram.ch1_mode++;
					}
					ActProgram.ch1_rate = getDefaultRate(ActProgram.ch1_mode);
				}
				if (selected_channel == 2) {
					if (ActProgram.ch2_mode == 3) {
						ActProgram.ch2_mode = 1; 
					} else {
						ActProgram.ch2_mode++;
					}
					ActProgram.ch2_rate = getDefaultRate(ActProgram.ch2_mode);
				}
				if (selected_channel == 3) {
					if (ActProgram.ch3_mode == 3) {
						ActProgram.ch3_mode = 1; 
					} else {
						ActProgram.ch3_mode++;
					}
					ActProgram.ch3_rate = getDefaultRate(ActProgram.ch3_mode);
				}
				save_program=program;
				break;
			// i: change midi instrument
			case 'i': 
				if (selected_channel == 1) {
					if (ActProgram.ch1_instrument >= 15) {
						ActProgram.ch1_instrument = 1; 
					} else {
						ActProgram.ch1_instrument++;
					}
					midiProgramChange(midiDevice, ActProgram.ch1_instrument, 1);
				}
				if (selected_channel == 2) {
					if (ActProgram.ch2_instrument >= 15) {
						ActProgram.ch2_instrument = 1; 
					} else {
						ActProgram.ch2_instrument++;
					}
					midiProgramChange(midiDevice, ActProgram.ch2_instrument, 2);
				}
				if (selected_channel == 3) {
					if (ActProgram.ch3_instrument >= 15) {
						ActProgram.ch3_instrument = 1; 
					} else {
						ActProgram.ch3_instrument++;
					}
					midiProgramChange(midiDevice, ActProgram.ch3_instrument, 3);
				}
				save_program=program;
				break;
			// m: change mute mode							
			case 'm': 
				if (selected_channel == 1) {
					if (ActProgram.ch1_mute == 0) {
						ActProgram.ch1_mute = 1; 
					} else {
						ActProgram.ch1_mute = 0;
					}
				}
				if (selected_channel == 2) {
					if (ActProgram.ch2_mute == 0) {
						ActProgram.ch2_mute = 1; 
					} else {
						ActProgram.ch2_mute = 0;
					}
				}
				if (selected_channel == 3) {
					if (ActProgram.ch3_mute == 0) {
						ActProgram.ch3_mute = 1; 
					} else {
						ActProgram.ch3_mute = 0;
					}
				}														
				save_program=program;
				break;

			// -: decrease gain
			case '-': 
				if (selected_channel == 1) {
						if (ActProgram.ch1_gain>5) ActProgram.ch1_gain -= 5;
				}
				if (selected_channel == 2) {
						if (ActProgram.ch2_gain>5) ActProgram.ch2_gain -= 5;
				}
				if (selected_channel == 3) {
						if (ActProgram.ch3_gain>5) ActProgram.ch3_gain -= 5;
				}		
				save_program=program;												
				break;
			// +: increase gain
			case '+': 
				if (selected_channel == 1) {
						ActProgram.ch1_gain += 5;
				}
				if (selected_channel == 2) {
						ActProgram.ch2_gain += 5;
				}
				if (selected_channel == 3) {
						ActProgram.ch3_gain += 5;
				}
				save_program=program;									
				break;
			// r: decrease sample rate														
			case 'r': 
				if (selected_channel == 1) {
						if (ActProgram.ch1_rate>5) ActProgram.ch1_rate -= 5;
				}
				if (selected_channel == 2) {
						if (ActProgram.ch2_rate>5) ActProgram.ch2_rate -= 5;
				}
				if (selected_channel == 3) {
						if (ActProgram.ch3_rate>5) ActProgram.ch3_rate -= 5;
				}
				save_program=program;									
				break;
			 // t: increase sample rate
			case 't':
				if (selected_channel == 1) {
						ActProgram.ch1_rate += 5;
				}
				if (selected_channel == 2) {
						ActProgram.ch2_rate += 5;
				}
				if (selected_channel == 3) {
						ActProgram.ch3_rate += 5;
				}
				save_program=program;									
				break;
			 // 1: feedback parameter nominator
			case '1':
				if (selected_channel == 1) {
					if (ActProgram.ch1_c1 == 6) {
						ActProgram.ch1_c1 = 1; 
					} else {
						ActProgram.ch1_c1++;
					}
				}
				if (selected_channel == 2) {
					if (ActProgram.ch2_c1 == 6) {
						ActProgram.ch2_c1 = 1; 
					} else {
						ActProgram.ch2_c1++;
					}
				}
				if (selected_channel == 3) {
					if (ActProgram.ch3_c1 == 6) {
						ActProgram.ch3_c1 = 1; 
					} else {
						ActProgram.ch3_c1++;
					}
				}
				save_program=program;									
				break;
			// 2: feedback parameter denominator
			case '2': 
				if (selected_channel == 1) {
					if (ActProgram.ch1_c2 == 6) {
						ActProgram.ch1_c2 = 0; 
					} else {
						ActProgram.ch1_c2++;
					}
				}
				if (selected_channel == 2) {
					if (ActProgram.ch2_c2 == 6) {
						ActProgram.ch2_c2 = 0; 
					} else {
						ActProgram.ch2_c2++;
					}
				}
				if (selected_channel == 3) {
					if (ActProgram.ch3_c2 == 6) {
						ActProgram.ch3_c2 = 0; 
					} else {
						ActProgram.ch3_c2++;
					}
				}
				save_program=program;
				break;
			// exit
			case 'x':
				running = 0;
				break;	
				
			// default: printf("unknown key = %i\n",key);						
		}
		
		
		if (save_program>-1) {
			saveprogram(save_program,&ActProgram);
			loadprogram(save_program,&ActProgram);
			save_program=0;
		}
		if (load_program>-1) {
			loadprogram(load_program,&ActProgram);
			load_program=0;
		}

		if (USE_GRAPHICS) draw_configsettings(&ActProgram, selected_channel);

	} 
}


void handle_displayscreen()
{	
	key = fgetc(stdin);

	if (key == '+') {
		// increase amplifier
		Signals.amplifier += 0.02;
	}else if (key == '-') {
		// decrease amplifier
		Signals.amplifier -= 0.02;
	}else if (key == 'c') {
		// switch to config screen
		configscreen = 1;
		need_screen_init=1;
		return;   // in this case, continue in configscreen handler !
	}else if (key == '-') {
		// previous program
		if (program == 0) program = 4;
		else program--;
		changeprogram = 1;
	}
	else if ((key == 'n') || washwheel_changed()) {   // check for next program (same as key 'n')
		if (program < 4) program++;
		else program = 0;
		changeprogram = 1;
	}
	else if ((key =='x') || stopbutton_pressed())  {   // check for stop (same as key 'x')
		running = 0;
		soundoff();
	}
	
	if (changeprogram == 1){
		  loadprogram(program,&ActProgram);
		  changeprogram = 0;
		  need_screen_init=1;
		  usleep (10000); 
	}
			
	if (need_screen_init)
	{
		printf("Screeninit: reset buffers and background \n");
		need_screen_init=0;
		init_buffers();
		if (USE_GRAPHICS) init_displayScreen(&ActProgram);	
		printf("Screnninit: done \n");
	} 
	else
	{
		if (USE_GRAPHICS) draw_displayscreen(&ActProgram, &Signals);
		else printf ("ch1: %i \tch2: %i\tch3: %i\n",(int)Signals.midi1,(int)Signals.midi2,(int)Signals.midi3);
	}
} 



int main(int argc, char**argv) {
	
	// catch CTRL-C
	signal(SIGINT, exithandler);
	

	printf("######################################################\n");
	printf("          Mind-O-Matic BrainWashing Machine \n");
	printf("                (c) SHIFZ 1997-2016 \n");
	printf("######################################################\n");
	printf("\n  Let's get started...\n");


	for (int i=1; i<argc; i++) {
		if (!strcmp(argv[i],"-nogpio")) { printf("\n  Option -nogpio found - switching off GPIO input !\n"); USE_GPIO=0; }
		else if (!strcmp(argv[i],"-noaudio")) { printf("\n  Option -noaudio found - switching off audio output !\n"); USE_AUDIO=0; }
		else if (!strcmp(argv[i],"-nographics")) { printf("\n  Option -nographics found - switching off graphics output !\n"); USE_GRAPHICS=0; }
	}
	
	// Init GPIO
	if (USE_GPIO) {
		printf("  Init GPIO device...\n");
		bcm2835_init();
	}

	if (USE_AUDIO) {
		printf("  Init audio / midi device...\n");
		init_audio();
	}

	// Init Filters
	printf("  Init filters...\n");
	defineFilters();

	// set non blocking
	printf("  Set stdin to nonblocking.\n");
	setStdinNonblocking();


	if (USE_GRAPHICS) {
		// Initalisation of graphic output
		printf("  Init Graphics System...\n");
		initDrawingScreen();
	}

	// Select Device and mode of operation
	while (modeselect < MODE_EXIT)
	{
		printf("  Keyboard loop for input device selection....\n");
		printf("  Select device (n:next / p:previous): (0) Arduino | (1) OpenEEG | (2) File <neurofile.txt>  | (3) MusicPlayer  | (4) Exit\n");
		if (USE_GRAPHICS) draw_startScreen(modeselect, "use washwheel or 'n' to select mode, press washbutton or space to start" );
		
		
		do {
			key = fgetc(stdin);
			if ((key == 'n') || washwheel_changed()) { 
			    modeselect++; if (modeselect > MODE_EXIT) modeselect=0; 
				printf("current selection: %d\n",modeselect);
				if (USE_GRAPHICS) draw_startScreen(modeselect, "use washwheel or 'n' to select mode, press play or 'space' to start" );
			}
			usleep(5000);
		} while ((key != 32) && (!startbutton_pressed()));

		printf("Mode %d selected !\n",modeselect);

		switch(modeselect) {
			case MODE_ARDUINO:
				printf("  /dev/ttyACM0 was selected as input device, trying to open device\n");
				fd=openSerialPort("/dev/ttyACM0");
				break;
			case MODE_OPENEEG:
				printf("  /dev/ttyUSB0 was selected as input device, trying to open device\n");
				fd=openSerialPort("/dev/ttyUSB0");
				break;
			case MODE_OPENEEG_BT:
				printf("  /dev/rfcomm0 was selected as input device, trying to open device\n");
				fd=openSerialPort("/dev/rfcomm0");
				break;
			case MODE_ARCHIVE:
				printf("  Archive File was selected as input device.\n");
				// Set file descriptor to file
				fd = open("neuro_file.txt", O_RDONLY);
				break;
			case MODE_MUSICPLAYER:
				printf("  Music Player running !\n");
				if (USE_GRAPHICS) draw_startScreen(modeselect, "MUSIC PLAYER RUNNING!" );
				if (USE_AUDIO) shutdown_audio();


				// remove playlist file (created from python music player)
				system ("rm /home/pi/playing.txt");

				system ("python /home/pi/MousePlayer/src/main.py /home/pi/Music > /dev/null 2>&1 &");
				
				usleep(1000000);  // wait for player to start
				int playfile =-1;
				int player_active=1;
				int success,len=0;
				char acttune[256],c;
				playfile = open("/home/pi/playing.txt", O_RDONLY);
				if (playfile>0) {
					while (player_active) {
						success=read(playfile, (void*) &c, 1); 
						if (success>0) { 
							if (c=='\n') { 
								acttune[len]=0; len=0;
								if (!strcmp(acttune,"quit")) player_active=0;
								else {
									char displaytext[300];
									strcpy(displaytext,"now playing: "); strcat(displaytext,acttune);
									draw_startScreen(modeselect, displaytext );
								}
							} else { if (len < 255) acttune[len]=c; len++; }
						} else usleep (1000);
					}
					close (playfile);
					system ("rm /home/pi/playing.txt");
				}
				
				printf("  Shutdown Music Player !\n");
				if (USE_GRAPHICS) draw_startScreen(modeselect, "Shutdown MUSIC PLAYER!" );
				if (USE_AUDIO) init_audio();
				break;
			case MODE_EXIT:
				printf("  Exit program\n");
				break;
		}
					
		
		if (modeselect < MODE_MUSICPLAYER)
		{
			
			if (fd < 0) { 
				printf("\nCould not open file descriptor...\n");
				if (USE_GRAPHICS) draw_startScreen(modeselect, "Could not open device - select another mode !" );
				usleep(2000000);

			} 
			else
			{
				printf("  Starting washloop with valid file descriptor: %d\n",fd);

				// open file if user wants to save output
				//if (savetofile) {
				//	outputfile = fopen(fileurl, "w");
				//}
				// printf("  Press Space to start...\n");
				// while ( fgetc(stdin) != 32) usleep(1000);   // wait for start key 

				printf("  Changing to default program \n");
				loadprogram(0,&ActProgram);

				printf("  Entering wash program loop \n");

				running =1;
				need_screen_init=1;
				configscreen = 0; 
				
				long counter=0;
				
				while(running) {
					// read raw input signals from device or archive file
					readport(fd, &Signals);
					if (modeselect==2) usleep (2500);  // delay a bit if read from file
					
					// calculate band activity and audio feedback values
					calc_filteredBands(&Signals);
					calc_midiValues(&ActProgram, &Signals);

					// perform audio feedback
					if (USE_AUDIO) {
						if (counter++ % 30 == 0) {
							if (ActProgram.ch1_mute == 1) 
								audiofeedback(ActProgram.ch1_mode, 1, Signals.midi1);
							if (ActProgram.ch2_mute == 1) 
							  audiofeedback(ActProgram.ch2_mode, 2, Signals.midi2);
							if (ActProgram.ch3_mute == 1) 
								audiofeedback(ActProgram.ch3_mode, 3, Signals.midi3);
						}
						// printf("after audiofeedback \n");  
					}

						
					if (configscreen)
						handle_configscreen();
					else 
						handle_displayscreen();
				}
				printf("  Wash program loop finished\n");
			} 
		}
	}
	exithandler();
} 


