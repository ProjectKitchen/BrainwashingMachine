
#ifndef _FILTERS_H
#define _FILTERS_H

#include "fidlib.h"
#include "signals.h"

void defineFilters();
void free_filters();
int getDefaultRate(int mode);
void do_averaging();
void init_buffers();


void calc_filteredBands(struct signalData * );
void calc_midiValues(struct Prog *, struct signalData * );


#endif
