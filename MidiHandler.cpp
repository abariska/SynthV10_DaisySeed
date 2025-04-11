#include "MidiHandler.h"
#include "Voice.h"

extern DaisySeed hw;

void MidiInit() {
    // MIDI налаштування
    MidiUsbHandler::Config midi_cfg;
    midi_cfg.transport_config.periph = MidiUsbTransport::Config::INTERNAL;
    midi.Init(midi_cfg);
}

// Конвертація MIDI ноти в частоту
float mtof(uint8_t midi_note) {
    return 440.0f * powf(2.0f, (midi_note - 69.0f) / 12.0f);
}

// Обробка MIDI повідомлень
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

// // Обробка Control Change
// void MidiControlChange(uint8_t control, uint8_t value) {
//     // TODO: Реалізувати обробку Control Change
// }

// // Обробка Program Change
// void MidiProgramChange(uint8_t program) {
//     // TODO: Реалізувати обробку Program Change
// } 