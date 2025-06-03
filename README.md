# SynthV10

![Version](https://img.shields.io/badge/version-0.1.0-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)

## Author

Borys Kuzmenko

## Project Description

SynthV10 is a digital synthesizer project based on the Daisy Seed microcontroller with an OLED display. The project is in development stage and gradually implements planned sound synthesis and user interface features.

## Current Functionality

### Sound Capabilities
- Polyphony with multiple voices
- 3 oscillators per voice with waveform selection
- Filter with cutoff and resonance control
- Amplitude envelope (ADSR)
- LFO for parameter modulation
- Effects: Overdrive, Chorus, Compressor, Reverb

### Interface
- OLED display for parameter visualization
- Menu system with different pages for settings
- Control via encoders and buttons
- LED indication of active modules

### MIDI
- MIDI support via USB
- Basic MIDI message processing

## Hardware Components

- **Daisy Seed** - STM32H7-based microcontroller
- **OLED display** - SSD130x 128x64
- **MCP23017** - I2C port expanders
- **Encoders and buttons** - for control
- **LEDs** - for status indication

## Libraries

- **DaisySP** - main audio processing library
- **libDaisy** - base library for working with Daisy Seed

## Project Structure

```
SynthV10_DaisySeed/
├── SynthV10.cpp       # Main program file
├── SynthV10.h         # Core definitions and interfaces
├── Button.cpp/h       # Button handling
├── DisplayOLED.cpp/h  # OLED display control
├── Effects.h          # Audio effects
├── Encoder_mcp.cpp/h  # Encoder control via MCP23017
├── Makefile           # Build configuration
├── Mcp23017.cpp/h     # MCP23017 driver
├── Menu.h             # Menu system
├── MidiHandler.cpp/h  # MIDI processing
├── Parameters.cpp/h   # Synthesizer parameters
├── Smallreverb.h      # Reverb effect implementation
└── Voice.cpp/h        # Sound generation
```

## External Library Paths

For successful compilation, you need to specify the correct paths to external libraries. In the `Makefile`, specify the following paths:

```makefile
# Path to libDaisy library
LIBDAISY_DIR = $(HOME)/dev/Daisy/libDaisy

# Path to DaisySP library
DAISYSP_DIR = $(HOME)/dev/Daisy/DaisySP
```

Libraries should be installed in these directories or paths should be modified according to your file system.

### Installing Libraries

```bash
# Clone and build libDaisy
git clone https://github.com/electro-smith/libDaisy.git
cd libDaisy
make

# Clone and build DaisySP
git clone https://github.com/electro-smith/DaisySP.git
cd DaisySP
make
```

## Known Issues

## License

This project is distributed under the MIT license.
