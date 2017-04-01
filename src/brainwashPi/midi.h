
#ifndef _MIDI_H
#define _MIDI_H

#include <alsa/asoundlib.h>
#include "signals.h"

// vars for soundoutput
extern snd_rawmidi_t *midiDevice;	


void closeMidiDevice(snd_rawmidi_t *midiOutHandle);
void pitchBend(snd_rawmidi_t *midiOutHandle,int bend, int chn);
void preparePitchBend(snd_rawmidi_t *midiOutHandle, int chn);
void midiNoteOff(snd_rawmidi_t *midiOutHandle, int note, int chn);
void midiNoteOn(snd_rawmidi_t *midiOutHandle, int note, int vel, int chn);
void midiVolume(snd_rawmidi_t *midiOutHandle, int vol, int chn);
void midiProgramChange(snd_rawmidi_t *midiOutHandle, int prog, int chn);
snd_rawmidi_t* openMidiDevice(void);
void find_midi_out(char *cardName);

void audiofeedback(int mode, int chn, double val);
int getDefaultRate(int mode);
int soundoff();
void initMidiInstruments(struct Prog * actprog );

#endif
