#include "voice.h"
#include "main.h"
 
std::array<Osc, OSC_NUM> osc;
PhaseGenerator phaseGenerator;
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

    phaseGenerator.Init(samplerate);
    for (size_t i = 0; i < OSC_NUM; i++) {
        osc[i].Init(samplerate);
    }
    flt.Init(samplerate);
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
		
		// Map velocity to amplitude
		float velocity_factor = velocity / 127.0f;
		float freq_compensation = powf(2.0f, (note_in - 60.0f) / 48.0f);
		amplitude = velocity_factor * freq_compensation;

        if (!(isNotesPlaying && params.global.isLegato)) {
            // phaseGenerator.Reset();

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

float VoiceProcess(){

    float sig = 0.0f;

    // float base_frequency = 440.0f * powf(2.0f, (noteNum - 69) / 12.0f);
    // phaseGenerator.SetFreq(base_frequency);
    
    // Get the master phase ONCE before the loop
    // float master_phase = phaseGenerator.Process();

    for (size_t i = 0; i < OSC_NUM; i++)
    {
        if (params.osc[i].active) {

            float final_freq = 440.0f * powf(2.0f, ((noteNum + params.osc[i].pitch) - 69) / 12.0f);
            final_freq *= powf(2.0f, (params.osc[i].detune));
            
            osc[i].SetFreq(final_freq);
            osc[i].SetAmp(params.osc[i].amp * amplitude);
            osc[i].SetWaveform(params.osc[i].waveform);
            osc[i].SetPw(params.osc[i].pw);
            
            sig += osc[i].Process();
        }

           
    }
    sig /= OSC_NUM;

        flt.SetFreq(params.filter.cutoff);
        flt.SetRes(params.filter.resonance);
        sig = flt.Process(sig);

        adsrMain.SetAttackTime(params.adsr.attack);
        adsrMain.SetDecayTime(params.adsr.decay);
        adsrMain.SetSustainLevel(params.adsr.sustain);
        adsrMain.SetReleaseTime(params.adsr.release);
    
        float env = adsrMain.Process(gate);
        sig *= env;     

    return sig;
}

void VoiceProcessTest(float& sigL, float& sigR){

    // float base_frequency = 440.0f * powf(2.0f, (noteNum - 69) / 12.0f);
    // phaseGenerator.SetFreq(base_frequency);
    
    // Get the master phase ONCE before the loop
    // float master_phase = phaseGenerator.Process();

    for (size_t i = 0; i < OSC_NUM; i++)
    {
        if (params.osc[i].active) {

            float final_freq = 440.0f * powf(2.0f, ((noteNum + params.osc[i].pitch) - 69) / 12.0f);
            final_freq *= powf(2.0f, (params.osc[i].detune));
            
            osc[i].SetFreq(final_freq);
            osc[i].SetAmp(params.osc[i].amp * amplitude);
            osc[i].SetWaveform(params.osc[i].waveform);
            osc[i].SetPw(params.osc[i].pw);
            
            
        }
        
           
    }
    adsrMain.SetAttackTime(params.adsr.attack);
    adsrMain.SetDecayTime(params.adsr.decay);
    adsrMain.SetSustainLevel(params.adsr.sustain);
    adsrMain.SetReleaseTime(params.adsr.release);
    float env = adsrMain.Process(gate);
    sigL = osc[0].Process() * env;  
    sigR = osc[1].Process() * env;
}
