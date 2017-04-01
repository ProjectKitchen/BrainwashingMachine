
#include <stdio.h>
#include <stdint.h>
#include <malloc.h>

#include "filters.h"


// for digital filters - possibility: dynamic use of call by value
#define PACKETSPERSECOND 256
FidFilter *filt_delta;	// to get filter information
FidFunc *funcp_delta;		// to get function information
FidRun *run_delta;		// to run filtering process
void *fbuf_delta1;
void *fbuf_delta2;		
	
FidFilter *filt_notch;	
FidFunc *funcp_notch;	
FidRun *run_notch;		
void *fbuf_notch1;
void *fbuf_notch2;			

FidFilter *filt_theta;	
FidFunc *funcp_theta;		
FidRun *run_theta;		
void *fbuf_theta1;
void *fbuf_theta2;			
FidFilter *filt_alpha;	
FidFunc *funcp_alpha;		
FidRun *run_alpha;		
void *fbuf_alpha1;
void *fbuf_alpha2;
FidFilter *filt_smr;	
FidFunc *funcp_smr;		
FidRun *run_smr;		
void *fbuf_smr1;
void *fbuf_smr2;
FidFilter *filt_beta;	
FidFunc *funcp_beta;		
FidRun *run_beta;		
void *fbuf_beta1;
void *fbuf_beta2;		


// buffers for averaging

#define buffersize 1000
uint16_t bufpos=0;

double buffer_alpha1[buffersize];
double buffer_beta1[buffersize];
double buffer_theta1[buffersize];					
double buffer_alpha2[buffersize];
double buffer_beta2[buffersize];
double buffer_theta2[buffersize];

double  sum_alpha1 = 0;
double  sum_alpha2 = 0;
double  sum_beta1 = 0;
double	sum_beta2 = 0;
double	sum_theta1 = 0;
double	sum_theta2 = 0;
				

void init_buffers()
{
		sum_alpha1 = 0;
		sum_alpha2 = 0;
		sum_beta1 = 0;
		sum_beta2 = 0;
		sum_theta1 = 0;
		sum_theta2 = 0;
		for(int bufpos=0;bufpos<buffersize;bufpos++)
		{
			buffer_alpha1[bufpos] = 0;
			buffer_alpha2[bufpos] = 0;
			buffer_beta1[bufpos] = 0;
			buffer_beta2[bufpos] = 0;
			buffer_theta1[bufpos] = 0;
			buffer_theta2[bufpos] = 0; 
		}

}


void defineFilters()
{
					
	filt_theta=fid_design("BpBu4", PACKETSPERSECOND, 4, 8, 0, 0);		// theta_filter: 4 - 8 Hz
	run_theta= fid_run_new(filt_theta, &(funcp_theta));
	fbuf_theta1=fid_run_newbuf(run_theta);
	fbuf_theta2=fid_run_newbuf(run_theta);

	filt_alpha=fid_design("BpBu4", PACKETSPERSECOND, 8, 12, 0, 0);		// alpha_filter: 8 - 12 Hz
	run_alpha= fid_run_new(filt_alpha, &(funcp_alpha));
	fbuf_alpha1=fid_run_newbuf(run_alpha);
	fbuf_alpha2=fid_run_newbuf(run_alpha);

	filt_beta=fid_design("BpBu4", PACKETSPERSECOND, 14, 22, 0, 0);		// beta_filter: 14 - 22 Hz
	run_beta= fid_run_new(filt_beta, &(funcp_beta));
	fbuf_beta1=fid_run_newbuf(run_beta);
	fbuf_beta2=fid_run_newbuf(run_beta);

	filt_notch=fid_design("BsBe4", PACKETSPERSECOND, 48, 52, 0, 0);		// beta_filter: 14 - 22 Hz
	run_notch= fid_run_new(filt_notch, &(funcp_notch));
	fbuf_notch1=fid_run_newbuf(run_notch);
	fbuf_notch2=fid_run_newbuf(run_notch);

}


void free_filters()
{
		// Filter Cleanup
	free(fbuf_delta1);
	free(fbuf_theta1);
	free(fbuf_alpha1);
	free(fbuf_smr1);
	free(fbuf_beta1);
	free(fbuf_delta2);
	free(fbuf_theta2);
	free(fbuf_alpha2);
	free(fbuf_smr2);
	free(fbuf_beta2);	
	free(fbuf_notch1);	
	free(fbuf_notch2);	
	fbuf_delta1 = NULL;
	fbuf_theta1 = NULL;
	fbuf_alpha1 = NULL;
	fbuf_smr1 = NULL;
	fbuf_beta1 = NULL;
	fbuf_delta2 = NULL;
	fbuf_theta2 = NULL;
	fbuf_alpha2 = NULL;
	fbuf_smr2 = NULL;
	fbuf_beta2 = NULL;	
	fbuf_notch1 = NULL;	
	fbuf_notch2 = NULL;		
	fid_run_free(run_delta);
	fid_run_free(run_theta);
	fid_run_free(run_alpha);
	fid_run_free(run_smr);
	fid_run_free(run_beta);
	fid_run_free(run_notch);
}




// generating the bands using filters defined in defineFilters
void calc_filteredBands(struct signalData * Signals)
{
	double ch1_theta;
	double ch1_alpha;
	double ch1_beta;
	double ch2_theta;
	double ch2_alpha;
	double ch2_beta;

	double ch1_notch;
	double ch2_notch;


	ch1_notch = (funcp_notch(fbuf_notch1,(double)(Signals->chn1Raw)));	
	ch2_notch = (funcp_notch(fbuf_notch2,(double)(Signals->chn2Raw)));	

	ch1_alpha = (funcp_alpha(fbuf_alpha1,(double)(Signals->chn1Raw)));	
	ch1_beta = (funcp_beta(fbuf_beta1,(double)(Signals->chn1Raw)));
	ch1_theta = (funcp_theta(fbuf_theta1,(double)(Signals->chn1Raw)));	

	ch2_alpha = (funcp_alpha(fbuf_alpha2,(double)(Signals->chn2Raw)));	
	ch2_beta = (funcp_beta(fbuf_beta2,(double)(Signals->chn2Raw)));
	ch2_theta = (funcp_theta(fbuf_theta2,(double)(Signals->chn2Raw)));	

	// abs values of filtered channels
	ch1_alpha = (ch1_alpha<0 ? -ch1_alpha : ch1_alpha);
	ch1_beta = (ch1_beta<0 ? -ch1_beta : ch1_beta);
	ch1_theta = (ch1_theta<0 ? -ch1_theta : ch1_theta);
	ch2_alpha = (ch2_alpha<0 ? -ch2_alpha : ch2_alpha);
	ch2_beta = (ch2_beta<0 ? -ch2_beta : ch2_beta);
	ch2_theta = (ch2_theta<0 ? -ch2_theta : ch2_theta);

	sum_alpha1-= buffer_alpha1[bufpos];
	sum_alpha2-= buffer_alpha2[bufpos];
	sum_beta1-= buffer_beta1[bufpos];
	sum_beta2-= buffer_beta2[bufpos];
	sum_theta1-= buffer_theta1[bufpos];
	sum_theta2-= buffer_theta2[bufpos];

	buffer_alpha1[bufpos] = ch1_alpha;
	buffer_alpha2[bufpos] = ch2_alpha;
	buffer_beta1[bufpos] = ch1_beta;
	buffer_beta2[bufpos] = ch2_beta;
	buffer_theta1[bufpos] = ch1_theta;
	buffer_theta2[bufpos] = ch2_theta; 

	bufpos=(bufpos+1)%buffersize;
	
	sum_alpha1+= ch1_alpha;
	sum_alpha2 += ch2_alpha;
	sum_beta1 += ch1_beta;
	sum_beta2 += ch2_beta;
	sum_theta1 += ch1_theta;
	sum_theta2 += ch2_theta;
							
	Signals->band_alpha1 = sum_alpha1 / buffersize;
	Signals->band_alpha2 = sum_alpha2 / buffersize;
	Signals->band_beta1 = sum_beta1 / buffersize;
	Signals->band_beta2 = sum_beta2 / buffersize;
	Signals->band_theta1 = sum_theta1 / buffersize;
	Signals->band_theta2 = sum_theta2  / buffersize;
	
	Signals->chn1Raw = (int)ch1_notch;
	Signals->chn2Raw = (int)ch2_notch;
}


void calc_midiValues(struct Prog * actprog, struct signalData * Signals)
{
	 double m1,m2,m3;

		// AUDIO CHANNEL 1
		switch(actprog->ch1_c1) {
			case 1:	m1 = Signals->band_alpha1; break;
			case 2: m1 = Signals->band_beta1;  break;
			case 3:	m1 = Signals->band_theta1; break;
			case 4:	m1 = Signals->band_alpha2; break;
			case 5:	m1 = Signals->band_beta2;  break;
			case 6: m1 = Signals->band_theta2; break;
		}
		switch(actprog->ch1_c2) {
			case 1:	m1 /= Signals->band_alpha1;	break;
			case 2:	m1 /= Signals->band_beta1;	break;
			case 3:	m1 /= Signals->band_theta1;	break;
			case 4:	m1 /= Signals->band_alpha2;	break;
			case 5:	m1 /= Signals->band_beta2;	break;
			case 6:	m1 /= Signals->band_theta2;	break;
		}
		
		// AUDIO CHANNEL 2
		switch(actprog->ch2_c1) {
			case 1:	m2 = Signals->band_alpha1;	break;
			case 2:	m2 = Signals->band_beta1;	break;
			case 3:	m2 = Signals->band_theta1;	break;
			case 4:	m2 = Signals->band_alpha2;	break;
			case 5:	m2 = Signals->band_beta2;	break;
			case 6:	m2 = Signals->band_theta2;	break;
		}
		switch(actprog->ch2_c2) {
			case 1:	m2 /= Signals->band_alpha1;	break;
			case 2: m2 /= Signals->band_beta1;	break;
			case 3:	m2 /= Signals->band_theta1;	break;
			case 4:	m2 /= Signals->band_alpha2;	break;
			case 5:	m2 /= Signals->band_beta2;	break;
			case 6:	m2 /= Signals->band_theta2;	break;
		}
			
		// AUDIO CHANNEL 3
		switch(actprog->ch3_c1) {
			case 1:	m3 = Signals->band_alpha1;	break;
			case 2:	m3 = Signals->band_beta1;	break;
			case 3:	m3 = Signals->band_theta1;	break;
			case 4:	m3 = Signals->band_alpha2;	break;
			case 5:	m3 = Signals->band_beta2;	break;
			case 6:	m3 = Signals->band_theta2;	break;
		}
		switch(actprog->ch3_c2) {
			case 1:	m3 /= Signals->band_alpha1;	break;
			case 2:	m3 /= Signals->band_beta1;	break;
			case 3:	m3 /= Signals->band_theta1;	break;
			case 4:	m3 /= Signals->band_alpha2;	break;
			case 5:	m3 /= Signals->band_beta2;	break;
			case 6:	m3 /= Signals->band_theta2;	break;
		}
			
		Signals->midi1 = m1 * actprog->ch1_gain /100; 
		Signals->midi2 = m2 * actprog->ch2_gain /100; 
		Signals->midi3 = m3 * actprog->ch3_gain /100; 
}


