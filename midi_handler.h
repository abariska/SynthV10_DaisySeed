#ifndef MIDI_HANDLER_H
#define MIDI_HANDLER_H

#include "voice.h"

using namespace daisy;
using namespace daisysp;

extern float mod_wheel_value;
extern float pitch_bend_multiplier;
extern float aftertouch_value;
extern bool isMidiData;
extern float GetPitchBendTableValue(int index);

extern bool is_midi_host_usb;
void SetMidiHostUsb(bool value);

void HandleMidiMessage(MidiEvent m);
void HandleAftertouch(uint8_t value);
void HandleControlChange(uint8_t control, uint8_t value);
void MidiInit();
void UartMidiProcess();
void UsbMidiProcess();

#endif
