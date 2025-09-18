#include "midi_handler.h"
#include "voice.h"

MidiUartHandler midiUart;
MidiUsbHandler midiUsb;
bool midi_note_led = false;

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
        HandleNoteOn(m.data[0], m.data[1]);
        midi_note_led = true;
    }
    break;
    case NoteOff:
    {
        HandleNoteOff(m.data[0]);
        midi_note_led = false;
    }
    break;
    default:
        break;
    }
}

// // Handle Control Change
// void MidiControlChange(uint8_t control, uint8_t value) {
//     // TODO: Implement Control Change handling
// }

// // Handle Program Change
// void MidiProgramChange(uint8_t program) {
//     // TODO: Implement Program Change handling
// }