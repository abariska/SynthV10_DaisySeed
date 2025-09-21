#ifndef MIDI_HANDLER_H
#define MIDI_HANDLER_H

#include "daisy_seed.h"
#include "daisysp.h"
#include "parameters.h"
#include "voice.h"

using namespace daisy;
using namespace daisysp;

extern MidiUartHandler midiUart;
extern MidiUsbHandler midiUsb;
extern bool midi_note_led;

void HandleMidiMessage(MidiEvent m);
void MidiInit();

#endif