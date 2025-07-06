#include "Voice.h"

// Definition of global variables
std::array<BlOsc, OSC_NUM> osc;
Oscillator lfo;
Adsr adsrMain;
MoogLadder flt;

uint8_t noteNum = 60;
float phase = 0;
float frequency = 0;
float amplitude = 0;

const int maxNotes = 16;
int activeNotes[maxNotes];
int activeNoteCount = 0;
bool gate = false;
bool hardRetrigger = false;

// Function definitions
void InitLfo(float samplerate) {
    lfo.Init(samplerate);
}

float ProcessLfo() {
    // Apply parameters from template
    lfo.SetFreq(params.lfo.freq);
    lfo.SetWaveform(params.lfo.waveform);
    lfo.SetAmp(params.lfo.depth);
    return lfo.Process();
}

void VoiceInit(float samplerate, int blocksize) {

    for (size_t i = 0; i < OSC_NUM; i++) {
        osc[i].Init(samplerate);
    }
    flt.Init(samplerate);
    adsrMain.Init(samplerate, blocksize);
}

void HandleNoteOn(uint8_t noteNumber, uint8_t velocity)
{
	// Add this note to the list of active notes
	if(activeNoteCount < maxNotes) {
		activeNotes[activeNoteCount] = noteNumber;
		activeNoteCount++;

		noteNum = noteNumber;
		
		// Map velocity to amplitude
		float velocity_factor = velocity / 127.0f;
		float freq_compensation = powf(2.0f, (noteNumber - 60.0f) / 48.0f);
		amplitude = velocity_factor * freq_compensation;

        if (activeNoteCount == 1){
            adsrMain.Retrigger(hardRetrigger);
            gate = true;
        }
	}
}

void HandleNoteOff(uint8_t noteNumber)
{
	bool activeNoteChanged = false;
	
	// Go through all the active notes and remove any with this number
	for(int i = activeNoteCount - 1; i >= 0; i--) {
		if(activeNotes[i] == noteNumber) {

			// Found a match: is it the most recent note?
			if (i == activeNoteCount - 1) {
				activeNoteChanged = true;
			}

			for (int j = i; j < activeNoteCount - 1; j++) {
				activeNotes[j] = activeNotes[j + 1];
			}
			activeNoteCount--;
		}
	}
    
    if(activeNoteCount == 0) {
		// No notes left
		amplitude = 0;
        gate = false;
	}
	else if(activeNoteChanged) {
		// Update the frequency but don't retrigger
		int mostRecentNote = activeNotes[activeNoteCount - 1];
		
		noteNum = mostRecentNote;
        gate = true;
	}
}

float VoiceProcess(){

    float sig = 0.0f;

    // Update oscillator parameters
    for (size_t i = 0; i < OSC_NUM; i++) {
        // Update parameters from template
        osc[i].SetWaveform(params.osc[i].waveform);
        osc[i].SetPw(params.osc[i].pw);
        
        // Add semitone shift (each semitone is 1 MIDI note)
        frequency = 440.0f * pow(2.0f, ((noteNum + params.osc[i].pitch) - 69) / 12.0f);
        frequency *= pow(2.0f, (params.osc[i].detune));  // detune in cents

        osc[i].SetFreq(frequency);
        osc[i].SetAmp(params.osc[i].amp * amplitude);
        
        // Process only active oscillators
        if (params.osc[i].active) {
            sig += osc[i].Process();
        }
    }
    sig /= OSC_NUM;
    
    flt.SetFreq(params.filter.cutoff);
    flt.SetRes(params.filter.resonance);
    sig = flt.Process(sig);
    
    // Apply ADSR
    adsrMain.SetAttackTime(params.adsr.attack);
    adsrMain.SetDecayTime(params.adsr.decay);
    adsrMain.SetSustainLevel(params.adsr.sustain);
    adsrMain.SetReleaseTime(params.adsr.release);
    
    float env = adsrMain.Process(gate);
    sig *= env;

    return sig;
}

