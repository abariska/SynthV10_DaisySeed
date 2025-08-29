#include "voice.h"
#include "main.h"
#include "daisy_seed.h"

std::array<Osc, OSC_NUM> osc;
Adsr adsrMain;
MoogLadder fltL;
MoogLadder fltR;
Oscillator lfo;
SlewLimiter freqSlew[OSC_NUM];

using P = ParamUnitName;

uint8_t noteNum = 60;
float phase = 0;
float frequency = 0;
float amplitude = 0;
float masterPhase = 0.0f; 
float oscPhase[OSC_NUM] = {0.0f, 0.0f, 0.0f};
float oscPhaseInc[OSC_NUM] = {0.0f, 0.0f, 0.0f};
float phaseOffsets[OSC_NUM] = {0.0f, 0.0f, 0.0f}; 
float pitch_correction[OSC_NUM] = {0.0f, 0.0f, 0.0f};
float detune_correction[OSC_NUM] = {0.0f, 0.0f, 0.0f};

float smoothedFreq[OSC_NUM] = {0.0f};

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
    lfo.SetFreq(paramManager.GetValue(P::LFO_FREQ));
    lfo.SetWaveform(paramManager.GetValue(P::LFO_WAVEFORM));
    lfo.SetAmp(paramManager.GetNormalised(P::LFO_DEPTH));
    return lfo.Process();
}

void VoiceInit(float samplerate, int blocksize) {

    for (size_t i = 0; i < OSC_NUM; i++) {
        osc[i].Init(samplerate);
    }
    fltL.Init(samplerate);
    fltR.Init(samplerate);
    adsrMain.Init(samplerate, blocksize);
    // SlewLimiter for frequency
    for (size_t i = 0; i < OSC_NUM; i++) {
        freqSlew[i].Init(0.00001f, samplerate);  // 1 мс згладжування
        freqSlew[i].SetCurrent(frequency);     // початкове значення
    }
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
            pitch_correction[i] = powf(2.0f, paramManager.GetInt(OSC_PITCH[i]) / 12.0f);  
            detune_correction[i] = powf(2.0f, paramManager.GetInt(OSC_DETUNE[i]) / 1200.0f);     

            oscPhase[i] = masterPhase * frequency;
            // oscPhase[i] -= floorf(oscPhase[i]);
            freqSlew[i].SetCurrent(frequency); 
        }
		
		float velocity_factor = velocity / 127.0f;
		float freq_compensation = powf(2.0f, (note_in - 60.0f) / 48.0f);
		amplitude = velocity_factor * freq_compensation;

        if (!(isNotesPlaying && paramManager.GetBool(P::GLOBAL_LEGATO))) {
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
    masterPhase += phaseInc;
    if(masterPhase >= 1.0f) masterPhase -= 1.0f;

    for (size_t i = 0; i < OSC_NUM; i++)
    {
        if (paramManager.GetValue(OSC_ACTIVE[i])) {

            float final_freq = frequency * pitch_correction[i] * detune_correction[i];
            smoothedFreq[i] = freqSlew[i].Process(final_freq); 

            oscPhase[i] *= (final_freq / smoothedFreq[i]);
            oscPhase[i] -= floorf(oscPhase[i]);
            
            osc[i].SetFreq(smoothedFreq[i]);
            osc[i].SetAmp(paramManager.GetNormalised(OSC_AMP[i]) * amplitude);
            osc[i].SetWaveform(paramManager.GetValue(OSC_WAVEFORM[i]));
            osc[i].SetPw(paramManager.GetNormalised(OSC_PWM[i]));
            float sig = osc[i].Process(oscPhase[i]);

            if (paramManager.GetValue(OSC_PAN[i]) != 0.0f) {
                float pan = paramManager.GetValue(OSC_PAN[i]);
                float leftGain = (pan <= 0.0f) ? 1.0f : (1.0f - pan / 100.0f);
                float rightGain = (pan >= 0.0f) ? 1.0f : (1.0f + pan / 100.0f);
                
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

    fltL.SetFreq(paramManager.GetValue(P::FILTER_CUTOFF));
    fltL.SetRes(paramManager.GetNormalised(P::FILTER_RESONANCE));
    fltR.SetFreq(paramManager.GetValue(P::FILTER_CUTOFF));
    fltR.SetRes(paramManager.GetNormalised(P::FILTER_RESONANCE));
    sigL = fltL.Process(sigL);
    sigR = fltR.Process(sigR);

    // TODO: fix the real curve. it sounds shorter than it should be.
    adsrMain.SetAttackTime(paramManager.GetValue(P::ADSR_ATTACK), 1.0f);
    adsrMain.SetDecayTime(paramManager.GetValue(P::ADSR_DECAY));
    adsrMain.SetSustainLevel(paramManager.GetNormalised(P::ADSR_SUSTAIN));
    adsrMain.SetReleaseTime(paramManager.GetValue(P::ADSR_RELEASE));

    float env = adsrMain.Process(gate);
    
    sigL *= env;
    sigR *= env;

}

