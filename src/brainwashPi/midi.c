#include "midi.h"


// vars for soundoutput
snd_rawmidi_t *midiDevice;	


// #define alloca(x) __builtin_alloca(x)

void find_midi_out(char *cardName)
{
	register int err;
	int cardNum;
	snd_rawmidi_info_t *rawMidiInfo;

	printf("  finding midi out devices ..\n");

	// Assume no output found
	cardName[0] = 0;


	// Start with first card
	cardNum = -1;

	// To get some info about the subdevices of a MIDI device, we need a
	// snd_rawmidi_info_t, so let's allocate one on the stack
	snd_rawmidi_info_alloca(&rawMidiInfo);
	memset(rawMidiInfo, 0, snd_rawmidi_info_sizeof());

	printf("  looping through devices\n");

	do
	{
		// Get next sound card's card number. When "cardNum" == -1, then ALSA
		// fetches the first card
		if ((err = snd_card_next(&cardNum)) < 0) break;

		// Another card? ALSA sets "cardNum" to -1 if no more
		if (cardNum != -1)
		{
			snd_ctl_t			*midiHandle;

			// Open this card. We specify only the card number -- not any device nor sub-device too
			sprintf(cardName, "hw:%i", cardNum);
			printf("  trying to open %s\n",cardName);

			if ((err = snd_ctl_open(&midiHandle, cardName, 0)) >= 0)
			{
				int				devNum;

				// Start with the first device on this card, and look for a MIDI output
				devNum = -1;
				for (;;)
				{
					// Get the number of the next MIDI device on this card
					if ((err = snd_ctl_rawmidi_next_device(midiHandle, &devNum)) < 0 ||

					// No more MIDI devices on this card? ALSA sets "devNum" to -1 if so.
					// NOTE: It's possible that this sound card may have no MIDI devices on it
					// at all, for example if it's only a digital audio card
						devNum < 0)
					{	
						 break;
					}

					// Tell ALSA which device (number) we want info about
					snd_rawmidi_info_set_device(rawMidiInfo, devNum);

					// Get info on the MIDI out portion of this device
					snd_rawmidi_info_set_stream(rawMidiInfo, SND_RAWMIDI_STREAM_OUTPUT);

					// Tell ALSA to fill in our snd_rawmidi_info_t with info on the first
					// MIDI out subdevice
					snd_rawmidi_info_set_subdevice(rawMidiInfo, 0);
					if ((err = snd_ctl_rawmidi_info(midiHandle, rawMidiInfo)) >= 0 && snd_rawmidi_info_get_subdevices_count(rawMidiInfo))
					{
						// We found a MIDI Output device. Format its name in the caller's buffer
						sprintf(cardName, "hw:%i,%i", cardNum, devNum);
						printf("  Found Card: %s\n",cardName);

						// All done
						cardNum = -1;

						break;
					}
				}

				// Close the card after we're done enumerating its subdevices
				snd_ctl_close(midiHandle);
			}
		}

		// Another card?
	} while (cardNum != -1);
	// ALSA allocates some mem to load its config file when we call some of the
	// above functions. Now that we're done getting the info, let's tell ALSA
	// to unload the info and free up that mem
	snd_config_update_free_global();
}

snd_rawmidi_t* openMidiDevice(void)
{
	register int		err;
	snd_rawmidi_t		*midiOutHandle;
	char			cardName[64];

	find_midi_out(&cardName[0]);
	if (!cardName[0])
	{
		printf("Can't find a MIDI Output to play through!\n");
		return (NULL);
	}
	

	// Open output MIDI device
	if ((err = snd_rawmidi_open(NULL, &midiOutHandle, &cardName[0], 0)) < 0)
	{
		printf("Can't open MIDI Output %s: %s\n", &cardName[0], snd_strerror(err));
		return (NULL);
	}

	return (midiOutHandle);
}

void midiProgramChange(snd_rawmidi_t *midiOutHandle, int prog, int chn)
{
	unsigned char midimessage[15];

	if (!midiOutHandle) return;
	// change prog
	midimessage[0] = 0xC0+chn;

	// the prog
	midimessage[1] = prog;

	//printf("Changing channel %d instrument to %d\n",chn,prog);
	snd_rawmidi_write(midiOutHandle, &midimessage[0], 2);
}

void midiVolume(snd_rawmidi_t *midiOutHandle, int vol, int chn)
{
	unsigned char midimessage[15];

	if (!midiOutHandle) return;
	// change the volume
	midimessage[0] = 0xb0+chn;  // **** B0 !!!
	// volume controller
	midimessage[1] = 0x7;
	// volume
	midimessage[2] = vol;

	snd_rawmidi_write(midiOutHandle, &midimessage[0], 3);
}


void midiNoteOn(snd_rawmidi_t *midiOutHandle, int note, int vel, int chn)
{
	unsigned char midimessage[15];

	if (!midiOutHandle) return;
	// We are sending a note-on event
	midimessage[0] = 0x90+chn;

	// Play middle C (on the first MIDI channel) at a velocity of 100
	midimessage[1] = note;
	midimessage[2] = vel;

	snd_rawmidi_write(midiOutHandle, &midimessage[0], 3);
}

void midiNoteOff(snd_rawmidi_t *midiOutHandle, int note, int chn)
{
	unsigned char midimessage[15];

	if (!midiOutHandle) return;
	// We are sending a note-off event (with 0 velocity)
	midimessage[0] = 0x80+chn;

	// Play middle C (on the first MIDI channel) at a velocity of 100
	midimessage[1] = note;
	midimessage[2] = 0;

	snd_rawmidi_write(midiOutHandle, &midimessage[0], 3);
}

void preparePitchBend(snd_rawmidi_t *midiOutHandle, int chn)
{
	unsigned char midimessage[15];
	if (!midiOutHandle) return;

	// Prepare Pitch shifter for greater range
	midimessage[0]=0xb0+chn;
	midimessage[1]=0x65;
	midimessage[2]=0x00;
	snd_rawmidi_write(midiOutHandle, &midimessage[0], 3);
	midimessage[0]=0xb0+chn;
	midimessage[1]=0x64;
	midimessage[2]=0x00;
	snd_rawmidi_write(midiOutHandle, &midimessage[0], 3);
	midimessage[0]=0xb0+chn;
	midimessage[1]=0x06;
	midimessage[2]=0x10;
	snd_rawmidi_write(midiOutHandle, &midimessage[0], 3);
	midimessage[0]=0xb0+chn;
	midimessage[1]=0x26;
	midimessage[2]=0x04;
	snd_rawmidi_write(midiOutHandle, &midimessage[0], 3);
	
}

void pitchBend(snd_rawmidi_t *midiOutHandle,int bend, int chn)
{
	unsigned char midimessage[15];

	if (!midiOutHandle) return;
	midimessage[0]=0xe0+chn;
	midimessage[1]= ((bend)& 0x3fff);
	midimessage[2]= ((bend) >> 7);
	snd_rawmidi_write(midiOutHandle, &midimessage[0], 3);
}


void closeMidiDevice(snd_rawmidi_t *midiOutHandle)
{
	if (!midiOutHandle) return;
	//	snd_rawmidi_drain(midiOutHandle);
	
	// Close the MIDI Output
	snd_rawmidi_close(midiOutHandle);

}

#define DEFAULT_NOTE_VOL 70
#define DEFAULT_NOTE_PITCH 65
#define DEFAULT_NOTE_RATE 400
#define DEFAULT_VOLUME_RATE 10
#define DEFAULT_PITCH_RATE 10

double ch1_note = 0;
double ch2_note = 0;
double ch3_note = 0;

void audiofeedback(int mode, int chn, double val) {

static int oldval1=0;
static int oldval2=0;
static int oldval3=0;
				
	if (midiDevice != NULL) {
		switch (mode) {
			case 1:			// Pitch
			{ 
				int pitch=val*400;
				if(pitch>32767) pitch=32767;
				if(pitch<1) pitch=1;
				if (chn == 1) {
					if (ch1_note != DEFAULT_NOTE_PITCH) {
					midiNoteOff(midiDevice, ch1_note, chn);
					ch1_note = DEFAULT_NOTE_PITCH;
					midiNoteOn(midiDevice, DEFAULT_NOTE_PITCH, 127, chn);
					}
					else {
						if (oldval1 != pitch) {							
							pitchBend(midiDevice, pitch, chn);
							oldval1=pitch;
						}
					}						
				}
				if (chn == 2) {
					if (ch2_note != DEFAULT_NOTE_PITCH) {
					midiNoteOff(midiDevice, ch2_note, chn);
					ch2_note = DEFAULT_NOTE_PITCH;
					midiNoteOn(midiDevice, DEFAULT_NOTE_PITCH, 127, chn);
					}
					else {
						if (oldval2 != pitch) {							
							pitchBend(midiDevice, pitch, chn);
							oldval2=pitch;
						}
					}						
				}	
				if (chn == 3) {
					if (ch3_note != DEFAULT_NOTE_PITCH) {
					midiNoteOff(midiDevice, ch3_note, chn);
					ch3_note = DEFAULT_NOTE_PITCH;
					midiNoteOn(midiDevice, DEFAULT_NOTE_PITCH, 127, chn);
					}
					else {
						if (oldval3 != pitch) {							
							pitchBend(midiDevice, pitch, chn);
							oldval3=pitch;
						}
					}						
				}
			}
			break;
			case 2:			// Volume
			{
				int vol=val*20;
				if (vol>127) vol=127; 
				if (vol<1) vol=1; 
				if (chn == 1) {
					if (ch1_note != DEFAULT_NOTE_VOL) {
					midiNoteOff(midiDevice, ch1_note, chn);
					ch1_note = DEFAULT_NOTE_VOL;
					midiNoteOn(midiDevice, DEFAULT_NOTE_VOL, 127, chn);
					}
					else {
						if (oldval1 != vol) {							
							midiVolume(midiDevice, vol, chn);
							oldval1=vol;
						}
					}											
				}
				if (chn == 2) {
					if (ch2_note != DEFAULT_NOTE_VOL) {
					midiNoteOff(midiDevice, ch2_note, chn);
					ch2_note = DEFAULT_NOTE_VOL;
					midiNoteOn(midiDevice, DEFAULT_NOTE_VOL, 127, chn);
					}
					else {
						if (oldval2 != vol) {							
							midiVolume(midiDevice, vol, chn);
							oldval2=vol;
						}
					}						
				}	
				if (chn == 3) {
					if (ch3_note != DEFAULT_NOTE_VOL) {
					midiNoteOff(midiDevice, ch3_note, chn);
					ch3_note = DEFAULT_NOTE_VOL;
					midiNoteOn(midiDevice, DEFAULT_NOTE_VOL, 127, chn);
					}
					else {
						if (oldval3 != vol) {							
							midiVolume(midiDevice, vol, chn);
							oldval3=vol;
						}
					}						
				}
			}
			break;
			
			case 3:			// Note
			{
				int actnote=val*5;
				if(actnote>100) actnote=100;
				if (actnote<10) actnote=10;
				if (chn == 1) {
					if (ch1_note != actnote) {							
						midiNoteOff(midiDevice, ch1_note, chn);
						midiNoteOn(midiDevice, actnote, 127, chn);
						ch1_note=actnote;
					}
				}						
				
				if (chn == 2) {
					if (ch2_note != actnote) {							
						midiNoteOff(midiDevice, ch2_note, chn);
						midiNoteOn(midiDevice, actnote, 127, chn);
						ch2_note=actnote;
					}
				}						

				if (chn == 3) {
					if (ch3_note != actnote) {							
						midiNoteOff(midiDevice, ch3_note, chn);
						midiNoteOn(midiDevice, actnote, 127, chn);
						ch3_note=actnote;
					}
				}						
			}
				break;
		}
	}
}

int getDefaultRate(int mode) {
	switch (mode) {
		case 1:
			return DEFAULT_PITCH_RATE;
			break;
		case 2:
			return DEFAULT_VOLUME_RATE;
			break;
		case 3:
			return DEFAULT_NOTE_RATE;
			break;
		default:
			return 100;
	} 
}



int soundoff() {
	if (!midiDevice) return(0);
	
	midiNoteOff(midiDevice, ch1_note, 1);
	midiNoteOff(midiDevice, ch2_note, 2);
	midiNoteOff(midiDevice, ch3_note, 3);
	
	ch1_note = 0;
	ch2_note = 0;
	ch3_note = 0;

	return 1;
}


void initMidiInstruments(struct Prog * actprog )
{
	if (midiDevice != NULL) {
		midiProgramChange(midiDevice, actprog->ch1_instrument, 1);
		midiProgramChange(midiDevice, actprog->ch2_instrument, 2);
		midiProgramChange(midiDevice, actprog->ch3_instrument, 3);
		midiVolume(midiDevice, 127,1);
		midiVolume(midiDevice, 127,2);
		midiVolume(midiDevice, 127,3);
	}
}
