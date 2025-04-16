#ifndef MIDI_HANDLER_H
#define MIDI_HANDLER_H

#include "daisy_seed.h"
#include "daisysp.h"
#include "Parameters.h"
#include "Voice.h"

using namespace daisy;
using namespace daisysp;

// External declarations
extern MidiUsbHandler midi;

// MIDI functions
void HandleMidiMessage(MidiEvent m);
// void MidiControlChange(uint8_t control, uint8_t value);
// void MidiProgramChange(uint8_t program);
void MidiInit();

#endif 