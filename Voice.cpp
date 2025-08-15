#include "voice.h"
#include "main.h"
#include "daisy_seed.h"

std::array<Osc, OSC_NUM> osc;
Adsr adsrMain;
MoogLadder fltL;
MoogLadder fltR;
Oscillator lfo;


uint8_t noteNum = 60;
float phase = 0;
float frequency = 0;
float amplitude = 0;
float masterPhase = 0.0f; 
float phaseOffsets[OSC_NUM] = {0.0f, 0.0f, 0.0f}; 
float pitch_correction[OSC_NUM] = {0.0f, 0.0f, 0.0f};
float detune_correction[OSC_NUM] = {0.0f, 0.0f, 0.0f};

const int maxNotes = 16;
int activeNotes[maxNotes];
int activeNoteCount = 0;
bool gate = false;
bool hardRetrigger = false;

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
    fltL.Init(samplerate);
    fltR.Init(samplerate);
    adsrMain.Init(samplerate, blocksize);
}

void HandleNoteOn(uint8_t note_in, uint8_t velocity)
{
    bool isNotesPlaying = (activeNoteCount > 0); 
	// Add this note to the list of active notes
	if(activeNoteCount < maxNotes) {
		activeNotes[activeNoteCount] = note_in;
		activeNoteCount++;

		noteNum = note_in;
        frequency = 440.0f * powf(2.0f, (note_in - 69) / 12.0f);

        for (size_t i = 0; i < OSC_NUM; i++) {
            pitch_correction[i] = powf(2.0f, params.osc[i].pitch / 12.0f); 
            detune_correction[i] = powf(2.0f, params.osc[i].detune);     
        }
		
		float velocity_factor = velocity / 127.0f;
		float freq_compensation = powf(2.0f, (note_in - 60.0f) / 48.0f);
		amplitude = velocity_factor * freq_compensation;

        if (!(isNotesPlaying && params.global.isLegato)) {
            adsrMain.Retrigger(false); 
        } 
        gate = true;
	}
}

void HandleNoteOff(uint8_t note_in)
{
	bool activeNoteChanged = false;
	
	// Go through all the active notes and remove any with this number
	for(int i = activeNoteCount - 1; i >= 0; i--) {
		if(activeNotes[i] == note_in) {

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
        gate = false;
	}
	else if(activeNoteChanged) {
		// Update the frequency but don't retrigger
		int mostRecentNote = activeNotes[activeNoteCount - 1];
		
		noteNum = mostRecentNote;
        gate = true;
	}
}

void VoiceProcess(float& sigL, float& sigR){
    sigL = 0.0f;
    sigR = 0.0f;
    
    float phaseInc = frequency / samplerate;

    for (size_t i = 0; i < OSC_NUM; i++)
    {
        if (params.osc[i].active) {

            float final_freq = frequency * pitch_correction[i] * detune_correction[i];
            float oscPhase = fmodf(masterPhase + phaseOffsets[i], 1.0f);
            
            osc[i].SetFreq(final_freq);
            osc[i].SetAmp(params.osc[i].amp * amplitude);
            osc[i].SetWaveform(params.osc[i].waveform);
            osc[i].SetPw(params.osc[i].pw);
            float sig = 0.0f;
            sig += osc[i].Process(oscPhase);

            if (params.osc[i].pan != 0.0f) {
                float pan = params.osc[i].pan;
                float leftGain = (pan >= 0.0f) ? 1.0f : (1.0f + pan);
                float rightGain = (pan <= 0.0f) ? 1.0f : (1.0f - pan);
                
                sigL += sig * leftGain;
                sigR += sig * rightGain;
            } else {
                sigL += sig;
                sigR += sig;
            }

        }  
    }
    
    sigL /= OSC_NUM;
    sigR /= OSC_NUM;

    fltL.SetFreq(params.filter.cutoff);
    fltL.SetRes(params.filter.resonance);
    fltR.SetFreq(params.filter.cutoff);
    fltR.SetRes(params.filter.resonance);
    sigL = fltL.Process(sigL);
    sigR = fltR.Process(sigR);

    adsrMain.SetAttackTime(params.adsr.attack);
    adsrMain.SetDecayTime(params.adsr.decay);
    adsrMain.SetSustainLevel(params.adsr.sustain);
    adsrMain.SetReleaseTime(params.adsr.release);

    float env = adsrMain.Process(gate);
    
    sigL *= env;
    sigR *= env;

    masterPhase += phaseInc;
    if(masterPhase >= 1.0f) masterPhase -= 1.0f;

}

