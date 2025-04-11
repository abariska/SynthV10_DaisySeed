#ifndef MIDI_HANDLER_H
#define MIDI_HANDLER_H

#include "daisy_seed.h"
#include "daisysp.h"
#include "Parameters.h"
#include "Voice.h"

using namespace daisy;
using namespace daisysp;

// Зовнішні оголошення
extern Synth synth;
extern MidiUsbHandler midi;

// MIDI функції
void HandleMidiMessage(MidiEvent m);
// void MidiControlChange(uint8_t control, uint8_t value);
// void MidiProgramChange(uint8_t program);
void MidiInit();

#endif 