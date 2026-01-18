#include "midi_handler.h"
#include "voice.h"
#include "log_uart.h"
#include "daisy.h"
#include "usbh_midi.h"

MidiUartHandler midiUart;
MidiUsbHandler midiUsb;
USBHostHandle usbHost;

bool is_midi_host_usb = false;
bool midi_note_led = false;
float mod_wheel_value = 0.0f;
float pitch_bend_multiplier = 1.0f;
float aftertouch_value = 0.0f;

void USBH_ClassActive(void* data)
{
    if(usbHost.IsActiveClass(USBH_MIDI_CLASS))
    {
        UartPrint("MIDI device class active");
        MidiUsbHandler::Config midi_config;
        midi_config.transport_config.periph = MidiUsbTransport::Config::Periph::HOST;
        midiUsb.Init(midi_config);
        midiUsb.StartReceive();
    }
}
void USBH_Connect(void* data)
{
    UartPrint("device connected");
}

void USBH_Disconnect(void* data)
{
    UartPrint("device disconnected");
}

void USBH_Error(void* data)
{
    UartPrint("USB device error");
}

void MidiInit()
{
    if (!is_midi_host_usb) {
        MidiUsbHandler::Config midi_usb_cfg;
        midi_usb_cfg.transport_config.periph = MidiUsbTransport::Config::INTERNAL;
        midiUsb.Init(midi_usb_cfg);

        System::Delay(10);
    }
    else {
        USBHostHandle::Config usbhConfig;
        usbhConfig.connect_callback = USBH_Connect,
        usbhConfig.disconnect_callback = USBH_Disconnect,
        usbhConfig.class_active_callback = USBH_ClassActive,
        usbhConfig.error_callback = USBH_Error,
        usbHost.Init(usbhConfig);
    
        usbHost.RegisterClass(USBH_MIDI_CLASS);
    
    }
    MidiUartHandler::Config midi_uart_cfg;
    midi_uart_cfg.transport_config.periph = UartHandler::Config::Peripheral::UART_5;
    midi_uart_cfg.transport_config.rx = Pin(PORTB, 5); 
    midiUart.Init(midi_uart_cfg);

    
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
    case ChannelPressure:
    {
        auto aftertouch = m.AsChannelPressure();
        HandleAftertouch(aftertouch.pressure);
    }
    break;
    default:
        break;
    }
}

void UartMidiProcess()
{
    MidiEvent m;
    while (midiUart.HasEvents())
    {
        m = midiUart.PopEvent();
        HandleMidiMessage(m);
    }
}

void UsbMidiProcess()
{
    if (is_midi_host_usb) {
        usbHost.Process();

            midiUsb.Listen();
            while (midiUsb.HasEvents())
            {
                MidiEvent msg = midiUsb.PopEvent();
                HandleMidiMessage(msg);
            }
    } else {
        midiUsb.Listen();
        while (midiUsb.HasEvents())
        {
            MidiEvent msg = midiUsb.PopEvent();
            HandleMidiMessage(msg);
        }
    }
}

void SetMidiHostUsb(bool value)
{
    is_midi_host_usb = value;
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