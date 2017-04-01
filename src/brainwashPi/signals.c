
#include <stdio.h>
#include <string.h>
#include "signals.h"
#include "midi.h"



void loadprogram(int i, struct Prog * actprog) {
	char filename[80];

	soundoff();

	strcpy(filename, "\0");
	sprintf(filename, "prog%i", i);
	strcat(filename, ".dat");	
	
	printf("  loading program %s\n",filename); 
	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		printf("could not load file !\n"); 
		return;
	}
	
	fread(actprog, sizeof(struct Prog), 1, file);
	actprog->number=i;
	fclose(file);
	
	initMidiInstruments( actprog );	
}

void saveprogram(int i, struct Prog * actprog) {

	char filename[80];
	strcpy(filename, "\0");
	sprintf(filename, "prog%i.dat", i);
	printf("  saving program %s\n",filename); 

	FILE *file = fopen(filename, "wb");
	actprog->number=i;
	if (file != NULL) {
		fwrite(actprog, sizeof(struct Prog), 1, file);
		fclose(file);
	}
	else printf("could not save file !\n"); 

		
}

