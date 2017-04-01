
#ifndef _SIGNALS_H
#define _SIGNALS_H


struct signalData {
	int chn1Raw;
	double band_alpha1;
	double band_beta1;
	double band_theta1;					
	
	int chn2Raw;
	double band_alpha2;
	double band_beta2;
	double band_theta2;
	
	double midi1;
	double midi2;
	double midi3;
	
	double amplifier;

} ;


struct Prog {
	// Instrument: 1-30
	int ch1_instrument ;
	int ch2_instrument ;
	int ch3_instrument ;
	// Mode: 1 = pitch / 2 = volume / 3 = note
	int ch1_mode ;
	int ch2_mode ;
	int ch3_mode ;	
	// Amplifier
	int ch1_gain ;
	int ch2_gain ;
	int ch3_gain ;
	// neurochannels 0 = Off / 1 = Alpha / 2 = Beta / 3 = Theta
	int ch1_c1 ;
	int ch1_c2 ;					
	int ch2_c1 ;
	int ch2_c2 ;
	int ch3_c1 ;
	int ch3_c2 ;
	// Mute
	int ch1_mute ;
	int ch2_mute ;
	int ch3_mute ;
	// Sample Rate
	int ch1_rate;
	int ch2_rate;
	int ch3_rate;		
	
	int number;  // number of the actual program
};	
	
	
void loadprogram(int i, struct Prog * actprog);
void saveprogram(int i, struct Prog * actprog);

	
#endif
