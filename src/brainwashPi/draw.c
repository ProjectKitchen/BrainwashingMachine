
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "draw.h"
#include "signal.h"



int width;
int height;

int counter = 0;

int xoffset = 110;  // offset of channel line graphs 
int xoffset2 = 110; // offset of feature line graphs
int xcoord, xcoord2;


// last y coordinate of signals
double ly_plain = 0;
double ly_plain2 = 0;
double ly_alpha = 0;
double ly_beta = 0;
double ly_theta = 0;

VGfloat color[4];


#define DISPLAY_GAIN 3
#define BAR_GAIN 15
#define NEURO_SPEED 15
#define PLAIN_SPEED 1


char programtext[5][20]={"Vorwasche","Spuelen","Waschen","Schleudern","Staerken"};
char bandNames [7][20] = { "   Off   ", "Ch1-Alpha", "Ch1-Beta", "Ch1-Theta", "Ch2-Alpha", "Ch2-Beta", "Ch2-Theta"} ;
char audioMode [4][20] = { " none ", "Pitch", "Volume", "Notes" };
char muteMode [3][20] = { "muted", "active" };

VGPath rectPath;
VGPath linePath;

const VGubyte lineCmds[] = {VG_MOVE_TO_ABS, VG_LINE_TO_ABS, VG_CLOSE_PATH} ;
VGfloat lineCoords[] = { 0,0,0,0 };

const VGubyte rectCmds[] = {VG_MOVE_TO_ABS, VG_LINE_TO_ABS, VG_LINE_TO_ABS, VG_LINE_TO_ABS, VG_LINE_TO_ABS, VG_CLOSE_PATH} ;
VGfloat rectCoords[] = { 0,0,0,0,0,0,0,0,0,0 };


void initPaths()
{
	rectPath =vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1.0f, 0.0f, 6, 10,  VG_PATH_CAPABILITY_ALL);
	vgAppendPathData(rectPath, 6, (const VGubyte *) rectCmds, rectCoords );   
	//vguRect(rectPath, (VGfloat)0, (VGfloat)0, (VGfloat)10, (VGfloat)10);
	
	linePath=vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1.0f, 0.0f, 3, 4,  VG_PATH_CAPABILITY_ALL);
	vgAppendPathData(linePath, 3, (const VGubyte *) lineCmds, lineCoords );   

	printf("paths initialized !\n");
}

void freePaths()
{
	vgDestroyPath(rectPath);
	vgDestroyPath(linePath);
}


// Rect makes a rectangle at the specified location and dimensions
void myRect(VGPath rectPath, VGfloat x, VGfloat y, VGfloat w, VGfloat h) {
	rectCoords[0]=x;
	rectCoords[1]=y;
	rectCoords[2]=x+w;
	rectCoords[3]=y;
	rectCoords[4]=x+w;
	rectCoords[5]=y+h;
	rectCoords[6]=x;
	rectCoords[7]=y+h;
	rectCoords[8]=x;
	rectCoords[9]=y;
	
	vgModifyPathCoords(rectPath,0,5,rectCoords);
	vgDrawPath(rectPath, VG_FILL_PATH | VG_STROKE_PATH);
}

// Line makes a line from (x1,y1) to (x2,y2)
void myLine(VGPath linePath, VGfloat x1, VGfloat y1, VGfloat x2, VGfloat y2) {
	lineCoords[0]=x1;
	lineCoords[1]=y1;
	lineCoords[2]=x2;
	lineCoords[3]=y2;

	vgModifyPathCoords(linePath,0,2,lineCoords);
	vgDrawPath(linePath, VG_STROKE_PATH);
}


void initDrawingScreen()
{
	init(&width, &height);              // Graphics initialization
	initPaths();
	Start(width, height);               // Start the picture with height/weight of screen
}

void exitDrawingScreen()
{
    End();
	finish();
	freePaths();	
}

void draw_startScreen(int i, char * text)
{
	// Show Credits
	Background(255, 255, 255);          // White background
	Fill(0, 153, 255, 1);               // cyan  text color
	Text(width/2-290, height/2 + 120, "Mind-O-Matic 2000", SerifTypeface, 50);  // Title
	Fill(255, 0, 153, 1);               // purple text
	Text(width/2-210, height/2 + 70, "BrainWashing Machine | SHIFZ", SerifTypeface, 20);  // subtitle 
	Image(width/2-391/2, height/2-220, 391, 255, "intro.jpg");
	
	Fill(0, 123, 225, 1);               // cyan  text color
    Text(40, 210, text, SerifTypeface, 14);

	Fill(150, 0, 80, 1);
	switch (i) {
		case 0:
  	       Text(40, 240, "Selected Mode: Read live EEG-Signal from Arduino (ttyACM0)", SerifTypeface, 16);
  	       break;
		case 1:
  	       Text(40, 240, "Selected Mode: Read live EEG-Signal from OpenEEG-USB (ttyUSB0)", SerifTypeface, 16);
  	       break;
		case 2:
  	       Text(40, 240, "Selected Mode: Read live EEG-Signal from OpenEEG-BT (rfcomm0)", SerifTypeface, 16);
  	       break;
		case 3:
  	       Text(40, 240, "Selected Mode: Read recorded EEG-Signal from Archive-File", SerifTypeface, 16);
  	       break;
		case 4:
  	       Text(40, 240, "Selected Mode: Music Player", SerifTypeface, 16);
  	       break;
		case 5:
  	       Text(40, 240, "Selected Mode: Exit", SerifTypeface, 16);
  	       break;
	   }

	End();
}


// format the textoutput
// gain
char * txt_gain(int i) {
	static char buffer[10];
	sprintf(buffer, "%d", i);
	return (buffer);
}

// Instrument
char * txt_instrument(int i) {
	static char buffer[10];
	sprintf(buffer, "%d", i);
	return (buffer);
}

void draw_bars(struct Prog* actprog, struct signalData * signals) {

	double temp;	
	int leftoffset = width/2 + 240;


	if (actprog->ch1_mute == 1) {							
		RGB(30, 0, 195, color);
		vgSetfv(VG_CLEAR_COLOR, 4, color);
		temp = signals->midi1;
		temp *= BAR_GAIN;
		if (temp > 319)  temp = 319; 
		if (temp < 1)  temp = 1; 
		vgClear (leftoffset, 50, 70, temp);
	}

	if (actprog->ch2_mute == 1) {							
		RGB(255, 0, 0, color);
		vgSetfv(VG_CLEAR_COLOR, 4, color);
		temp = signals->midi2;
		temp *= BAR_GAIN;
		if (temp > 319)  temp = 319; 
		if (temp < 1) temp = 1; 
		vgClear (leftoffset+200 , 50, 70, temp);
	}

	if (actprog->ch3_mute == 1) {							
		RGB(30, 127, 0, color);
		vgSetfv(VG_CLEAR_COLOR, 4, color);
		temp = signals->midi3;
		temp *= BAR_GAIN;
		if (temp > 319) temp = 319; 
		if (temp < 1) temp = 1; 
		vgClear (leftoffset +400 , 50, 70, temp);
	}

	RGB(255, 255, 255, color);
	vgSetfv(VG_CLEAR_COLOR, 4, color);
}


void draw_configsettings(struct Prog * actprog, int selected_channel)
{					
		// Show with selected settings

		Background(255, 255, 255);          // White background

		Fill(0, 103, 205, 1);               // #09f text
		Text(40, height - 80, "Config Settings", SerifTypeface, 50);  // Title
		
		Fill(255, 0, 153, 1);               // #f09 text
		Text(40, height - 940, "Press Space to return to live screen.", SerifTypeface, 30); 

		Fill(0, 50, 180, 1); // set to blue
		Text(40, 20, " up/down=channel selection     1/2=feedback parameter     +/-=gain    i=instrument    m=mute    a=audiomode    s=save program     n/p=change program", SerifTypeface, 15); 

		// Display: Program					 					
		Fill(0, 153, 255, 1); // set to blue
		Text(850, height - 80, "for washing stage ", SerifTypeface, 25); 
		Text(1200, height - 80, programtext[actprog->number], SerifTypeface, 40); // Program	
		
		// Title
		Fill(60, 60, 255, 1);  // set to white
		Text(100, height - 220, "Parameter", SerifTypeface, 25); 
		Text(400, height - 220, "Mute", SerifTypeface, 25); 
		Text(640, height - 220, "Audiomode", SerifTypeface, 25); 					
		Text(930, height - 220, "Gain", SerifTypeface, 25);
		Text(1150, height - 220, "Speed", SerifTypeface, 25); 
		Text(1330, height - 220, "Instrument", SerifTypeface, 25); 
		
		// Border
		StrokeWidth(3);
		Fill(255, 255, 255, 1);  // set to white
		Stroke(255, 0, 255, 1);	// draw rect with with border
		if (selected_channel == 1) {
			myRect(rectPath, 50, height - 400, width-100, 150); 						
		}
		if (selected_channel == 2) {
			myRect(rectPath, 50, height - 600, width-100, 150); 					
		}
		if (selected_channel == 3) {
			myRect(rectPath, 50, height - 800, width-100, 150); 						
		}
		StrokeWidth(1);



		// Display: Feedback Paramters
		int margin =0;
		
		Fill(20, 20, 200, 1);  // blue
		if (actprog->ch1_c2) {
			Text(100, height - 330, "divided by", SerifTypeface, 15);  
			Text(100, height - 370, bandNames[actprog->ch1_c2], SerifTypeface, 25); 
			margin=0;
		} else margin=50;
		Text(100, height - 300 -margin, bandNames[actprog->ch1_c1], SerifTypeface, 25); 
		Text(400, height - 350, muteMode[actprog->ch1_mute], SerifTypeface, 25); 
		Text(680, height - 350, audioMode[actprog->ch1_mode], SerifTypeface, 25); 
		Text(950, height - 350, txt_gain(actprog->ch1_gain), SerifTypeface, 25); 
		Text(1170, height - 350, txt_gain(actprog->ch1_rate), SerifTypeface, 25); 
		Text(1350, height - 350, txt_instrument(actprog->ch1_instrument), SerifTypeface, 25); 

		Fill(255, 20, 20, 1);  // red
		if (actprog->ch2_c2) {
			Text(100, height - 530, "divided by", SerifTypeface, 15);  
			Text(100, height - 570, bandNames[actprog->ch2_c2], SerifTypeface, 25); 
			margin=0;
		} else margin=50;
		Text(100, height - 500-margin, bandNames[actprog->ch2_c1], SerifTypeface, 25);					
		Text(400, height - 550, muteMode[actprog->ch2_mute], SerifTypeface, 25); 
		Text(680, height - 550, audioMode[actprog->ch2_mode], SerifTypeface, 25); 
		Text(950, height - 550, txt_gain(actprog->ch2_gain), SerifTypeface, 25); 
		Text(1170, height - 550, txt_gain(actprog->ch2_rate), SerifTypeface, 25); 
		Text(1350, height - 550, txt_instrument(actprog->ch2_instrument), SerifTypeface, 25); 

		Fill(0, 150, 0, 1);  // green
		if (actprog->ch3_c2) {
			Text(100, height - 730, "divided by", SerifTypeface, 15);  
			Text(100, height - 770, bandNames[actprog->ch3_c2], SerifTypeface, 25);
			margin=0;
		} else margin=50;
		Text(100, height - 700-margin, bandNames[actprog->ch3_c1], SerifTypeface, 25); 
		Text(400, height - 750, muteMode[actprog->ch3_mute], SerifTypeface, 25); 
		Text(680, height - 750, audioMode[actprog->ch3_mode], SerifTypeface, 25);
		Text(950, height - 750, txt_gain(actprog->ch3_gain), SerifTypeface, 25); 
		Text(1170, height - 750, txt_gain(actprog->ch3_rate), SerifTypeface, 25); 
		Text(1350, height - 750, txt_instrument(actprog->ch3_instrument), SerifTypeface, 25); 
							
		End();
}


void init_displayScreen(struct Prog * actprogram)
{
		// Reset Background
		Background(255, 255, 255);

		xcoord=xoffset;
		xcoord2=xoffset2;
	
		// Clear everything and set titles

		Fill(0, 153, 255, 1);               // #09f text
		Text(50, height - 90, "Mind-O-Matic 2000", SerifTypeface, 50);  // Title
		Fill(255, 0, 153, 1);               // #f09 text
		Text(50, height - 140, "BrainWashing Machine by SHIFZ", SerifTypeface, 20);  // subtitle 

		Fill(90, 0, 153, 1);               // #f09 text
		Text(900, height - 80, "current washing stage:", SerifTypeface, 25);  // output of selected program
		Text(1300, height - 80, programtext[actprogram->number], SerifTypeface, 30);  // output of selected program
		
		Fill(130, 130, 130, 1);               // #f09 text
		Text(880, height - 130, "[C] Configscreen   [+/-] In-/Decrease Amplifier   [N] Next Program    [X] exit", SerifTypeface, 12);  // key map 


		Fill(70, 70, 110, 1);               // #f09 text
		Text(50, height/2 -5, "Chn2", SerifTypeface, 10); 
		Text(50, height/2 + 250 -5 , "Chn1", SerifTypeface, 10); 

		StrokeWidth(1);
		Stroke(70, 70, 110, 1);
		myLine(linePath, xoffset-2, height/2-100, xoffset-2, height/2+100);
		myLine(linePath, xoffset-2, height/2+250-100, xoffset-2, height/2+250+100);
		myLine(linePath, xoffset2-2, 18, xoffset2-2, 320);
		myLine(linePath, xoffset2-2, 18, width/2, 18);


		Stroke(150, 150, 150, 1);
		myLine(linePath, xoffset, height/2, width, height/2);
		myLine(linePath, xoffset, height/2+250, width, height/2+250);



		Text(width/2+350, 370, "Active Neurofeedback Parameters", SerifTypeface, 11); 
		Text(150, 335, "EEG Frequency Band Activity", SerifTypeface, 11); 

		Text(50, 150, "Alpha", SerifTypeface, 10); 
		Text(50, 120, "Beta", SerifTypeface, 10); 
		Text(50, 90,  "Theta", SerifTypeface, 10); 

		RGB(30, 0, 195, color);
		vgSetfv(VG_CLEAR_COLOR, 4, color);
		vgClear (20, 150, 15, 15);

		RGB(255, 0, 0, color);
		vgSetfv(VG_CLEAR_COLOR, 4, color);
		vgClear (20, 120, 15, 15);

		RGB(30, 127, 0, color);
		vgSetfv(VG_CLEAR_COLOR, 4, color);
		vgClear (20, 90, 15, 15);



        // display program labels
		Fill(50, 20, 200, 1);
		char bartitle[50]; int margin;

		strcpy(bartitle, bandNames[actprogram->ch1_c1]); 
		if (actprogram->ch1_c2 != 0) { strcat (bartitle,"/"); strcat(bartitle,bandNames[actprogram->ch1_c2]); margin=0; } else margin=25;
		Text(1050+margin, 20, bartitle, SerifTypeface, 11); 

		strcpy(bartitle, bandNames[actprogram->ch2_c1]); 
		if (actprogram->ch2_c2 != 0) {strcat (bartitle,"/"); strcat(bartitle,bandNames[actprogram->ch2_c2]); margin=0; } else margin=25;
		Text(1240+margin, 20, bartitle, SerifTypeface, 11);  
		
		strcpy(bartitle, bandNames[actprogram->ch3_c1]); 
		if (actprogram->ch3_c2 != 0) {strcat (bartitle,"/"); strcat(bartitle,bandNames[actprogram->ch3_c2]); margin=0; } else margin=25;
		Text(1430+margin, 20, bartitle, SerifTypeface, 11);  

		End();
}

void draw_displayscreen(struct Prog* actprog, struct signalData * signals)
{
	double y;
		
	counter++; 
	if (counter % PLAIN_SPEED == 0) {
		// OUTPUT: of plain signal 1 and 2
		y = signals->chn1Raw * signals->amplifier + height / 2 + 250;
        if (y<400) y=400;
        if (y>850) y=850;

		StrokeWidth(1);
		Stroke(150, 0, 0, 1);
		if (ly_plain)
			myLine(linePath, xcoord, ly_plain, xcoord+1, y);
		ly_plain = y;
		
		y = signals->chn2Raw * signals->amplifier + height / 2 + 0;
        if (y<400) y=400;
        if (y>850) y=850;
		Stroke(200, 0, 0, 1);				
		if (ly_plain2)
			myLine(linePath,xcoord, ly_plain2, xcoord+1, y);
		ly_plain2 = y;

		xcoord++; 
		if (xcoord == width) {
			xcoord = xoffset;
			Fill(255, 255, 255, 1);  // set to white
			vgClear (xoffset-1, 398, width, 455);
			StrokeWidth(1);
			Stroke(150, 150, 150, 1);
			myLine(linePath, xoffset, height/2, width, height/2);
			myLine(linePath, xoffset, height/2+250, width, height/2+250);
			
		}
	}
	
	if (counter % NEURO_SPEED == 0) {	
		
		StrokeWidth(2);
		y = (signals->band_alpha1 + signals->band_alpha2) * DISPLAY_GAIN +20;
		if (y > 319) y = 319;	
		Stroke(30, 0, 195, 1);			
		if (ly_alpha)
			myLine(linePath, xcoord2, ly_alpha, xcoord2+1, y);
		ly_alpha = y;

		y = (signals->band_beta1 + signals->band_beta2) * DISPLAY_GAIN +20 ;
		if (y > 319) y = 319;	
		Stroke(255, 0, 0, 1);	
		if (ly_beta)
			myLine(linePath,xcoord2, ly_beta, xcoord2+1, y);
		ly_beta = y;			

		y = (signals->band_theta1 + signals->band_theta2) * DISPLAY_GAIN +20 ;
		if (y > 319) y = 319;	
		Stroke(0, 127, 0, 1);	
		if (ly_theta)
			myLine(linePath, xcoord2, ly_theta, xcoord2+1, y);
		ly_theta = y;
		StrokeWidth(1);

		xcoord2++;
		if (xcoord2 == width/2) {
			xcoord2 = xoffset2;
			RGB(255, 255, 255, color);
			vgSetfv(VG_CLEAR_COLOR, 4, color);
			vgClear (xoffset2, 20, width/2+1, 300);
		}		
	}
	
	if (counter%25 == 0) { 
		// update Bar Chart
		RGB(255, 255, 255, color);
		vgSetfv(VG_CLEAR_COLOR, 4, color);
		vgClear (width/2, 50, width/2, 320);
		draw_bars(actprog,signals);		
		
		// Output of visualization
		End();
		// printf("@");

	}		

}

	

