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
// void MidiControlChange(uint8_t control, uint8_t value);
// void MidiProgramChange(uint8_t program);
void MidiInit();

#endif