#include "midi_handler.h"
#include "voice.h"

extern DaisySeed hw;

void MidiInit() {
    // MIDI configuration
    MidiUsbHandler::Config midi_cfg;
    midi_cfg.transport_config.periph = MidiUsbTransport::Config::INTERNAL;
    midi.Init(midi_cfg);
}

// Handle MIDI messages
void HandleMidiMessage(MidiEvent m) {
    switch(m.type) {
        case NoteOn:
            {
                HandleNoteOn(m.data[0], m.data[1]);
            }
            break;
        case NoteOff:
            {
                HandleNoteOff(m.data[0]);
            }
            break;
        default:
            break;
    }
    // hw.PrintLine("Midi message: %d\n", m.type);
}

// // Handle Control Change
// void MidiControlChange(uint8_t control, uint8_t value) {
//     // TODO: Implement Control Change handling
// }

// // Handle Program Change
// void MidiProgramChange(uint8_t program) {
//     // TODO: Implement Program Change handling
// } 