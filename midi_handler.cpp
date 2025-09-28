#include "midi_handler.h"
#include "voice.h"

MidiUartHandler midiUart;
MidiUsbHandler midiUsb;
bool midi_note_led = false;
float mod_wheel_value = 0.0f;
float pitch_bend_multiplier = 1.0f;
float aftertouch_value = 0.0f;
void MidiInit()
{
    MidiUsbHandler::Config midi_usb_cfg;
    midi_usb_cfg.transport_config.periph = MidiUsbTransport::Config::INTERNAL;
    midiUsb.Init(midi_usb_cfg);

    MidiUartHandler::Config midi_uart_cfg;
    midi_uart_cfg.transport_config.periph = UartHandler::Config::Peripheral::USART_1;
    midi_uart_cfg.transport_config.rx = {DSY_GPIOB, 15}; // D30 = PB15 = USART1_RX
    midi_uart_cfg.transport_config.tx = {DSY_GPIOB, 14}; // D29 = PB14 = USART1_TX (опціонально)
    midiUart.Init(midi_uart_cfg);

    System::Delay(10);
}

// Handle MIDI messages
void HandleMidiMessage(MidiEvent m)
{
    switch (m.type)
    {
    case NoteOn:
    {
        auto note = m.AsNoteOn();
        HandleNoteOn(note.note, note.velocity);
        midi_note_led = true;
    }
    break;
    case NoteOff:
    {
        auto note = m.AsNoteOff();
        HandleNoteOff(note.note);
        midi_note_led = false;
    }
    break;
    case PitchBend:
    {
        auto pitch_bend = m.AsPitchBend();
        HandlePitchBend(pitch_bend.value);
    }
    break;
    case ControlChange:
    {
        auto control = m.AsControlChange();
        HandleControlChange(control.control_number, control.value);
    }
    break;
    default:
        break;
    }
}

void HandlePitchBend(int16_t pb)
{
    float bend_cents = (float)(pb / 8192.0f) * 200.0f;
    pitch_bend_multiplier = GetPitchBendTableValue(bend_cents);
    
}

// Handle Control Change
void HandleControlChange(uint8_t control, uint8_t value) {
    if (control == 1) {
        mod_wheel_value = value / 127.0f;
    }
}

void HandleAftertouch(uint8_t value) {
    aftertouch_value = value / 127.0f;
}

// // Handle Program Change
// void MidiProgramChange(uint8_t program) {
//     // TODO: Implement Program Change handling
// }