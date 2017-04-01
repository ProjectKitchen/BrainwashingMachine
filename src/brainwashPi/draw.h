
#ifndef _DRAW_H
#define _DRAW_H


#include "VG/openvg.h"
#include "VG/vgu.h"
#include "fontinfo.h"
#include "shapes.h"
#include "signals.h"


void init_displayScreen(struct Prog * );
void initDrawingScreen();
void exitDrawingScreen();
void draw_startScreen(int i, char * text);
void draw_displayscreen(struct Prog *, struct signalData *);
void draw_startup();
void draw_configscreen();
void draw_configsettings(struct Prog *,int selected_channel );

#endif
