#include "Voice.h"

// Визначення глобальних змінних
BlOsc lfo;
float lfoValue = 0.0f;
Adsr mainADSR;
VoiceUnit voice[NUM_VOICES];

// Визначення функцій
void InitLfo(float samplerate) {
    lfo.Init(samplerate);
}

void ProcessLfo() {
    // Застосовуємо параметри з шаблону
    lfo.SetFreq(params.lfo.freq);
    lfo.SetWaveform(params.lfo.waveform);
    lfo.SetAmp(params.lfo.amp);
    lfo.SetPw(params.lfo.pw);
    
    // Обробляємо LFO і зберігаємо значення в глобальну змінну
    lfoValue = lfo.Process();
}

// Реалізація методів VoiceUnit
void VoiceUnit::Init(float samplerate, int blocksize) {
    isVoiceActive = false; 
    isGated = false;
    note = -1;
    velocity = 0;
    timestamp = 0;

    // Initialize oscillators
    for (size_t i = 0; i < OSC_NUM; i++) {
        osc[i].Init(samplerate);
    }
    flt.Init(samplerate);
    adsrMain.Init(samplerate, blocksize);

}

void HandleNoteOn(uint8_t midi_note, uint8_t midi_vel) {
    static uint32_t global_time = 0;
    uint32_t current_time = global_time++;
    
    if (params.global.isMono) {
        voice[0].NoteOn(midi_note, midi_vel, current_time);
    } else {
        // Шукаємо вільний голос
        for (size_t i = 0; i < NUM_VOICES; i++) {
            if (!voice[i].isVoiceActive) {
                voice[i].NoteOn(midi_note, midi_vel, current_time);   
                return;
            }
        }
        // Якщо вільних немає, знаходимо найстаріший
        uint8_t oldest_index = 0;
        uint32_t oldest_timestamp = UINT32_MAX;
        for (size_t i = 0; i < NUM_VOICES; i++) {
            if (voice[i].isVoiceActive && voice[i].timestamp < oldest_timestamp) {
                oldest_timestamp = voice[i].timestamp;
                oldest_index = i;
                voice[oldest_index].isGated = false;
            }
        }
        voice[oldest_index].NoteOn(midi_note, midi_vel, current_time);
    }
}

void HandleNoteOff(uint8_t midi_note) {
    if (params.global.isMono) {
        // В моно режимі перевіряємо, чи це та сама нота, що зараз активна
        if (voice[0].isVoiceActive && voice[0].note == midi_note) {
            voice[0].NoteOff();
        }
    } else {
        for (size_t i = 0; i < NUM_VOICES; i++) {
            if (voice[i].isVoiceActive && voice[i].note == midi_note) {
                voice[i].NoteOff();
            }
        }
    }
}
void VoiceUnit::NoteOn(uint8_t midi_note, uint8_t midi_vel, uint32_t time){

            note = midi_note;
        velocity = midi_vel;
        timestamp = time;
        isVoiceActive = true;
        isGated = true;
        // adsrMain.Retrigger(true);
    }

void VoiceUnit::NoteOff(){
    isGated = false; 
    // isVoiceActive = false; 
}

float VoiceUnit::CalculateFrequency(uint8_t midi_note, int pitch, int detune) {
    // MIDI нота 69 (A4) відповідає частоті 440 Гц
    // Додаємо зсув по полутонах (кожен півтон - це 1 MIDI нота)
    float freq = 440.0f * pow(2.0f, ((midi_note + pitch) - 69) / 12.0f);
    freq *= pow(2.0f, (detune * 0.01f));  // detune в центах
    
    return freq;
}

float VoiceUnit::CalculateVelocity(uint8_t velocity){

    return velocity / 127.0f;
}

float VoiceUnit::Process(){
    if (!isVoiceActive && !isGated) return 0.0f;

    float sig = 0.0f;

    // Оновлюємо параметри осциляторів
    for (size_t i = 0; i < OSC_NUM; i++) {
        // Оновлюємо параметри з шаблону
        osc[i].SetWaveform(params.voice.osc[i].waveform);
        osc[i].SetPw(params.voice.osc[i].pw);
        
        // Розраховуємо фінальну частоту
        float final_freq = CalculateFrequency(
            note,
            params.voice.osc[i].pitch,
            params.voice.osc[i].detune
        );
        osc[i].SetFreq(final_freq);
        osc[i].SetAmp(params.voice.osc[i].amp * CalculateVelocity(velocity));
        
        // Обробляємо тільки активні осцилятори
        if (params.voice.osc[i].active) {
            sig += osc[i].Process();
        }
    }
    sig /= OSC_NUM;
    
    flt.SetFreq(params.voice.filter.cutoff);
    flt.SetRes(params.voice.filter.resonance);
    sig = flt.Process(sig);
    
    // Застосовуємо ADSR
    adsrMain.SetAttackTime(params.voice.adsr.attack, 0.1f);
    adsrMain.SetDecayTime(params.voice.adsr.decay);
    adsrMain.SetSustainLevel(params.voice.adsr.sustain);
    adsrMain.SetReleaseTime(params.voice.adsr.release);

    if (isGated && params.voice.adsr.retrigger) {
        adsrMain.Retrigger(true);
    }
    
    float env = adsrMain.Process(isGated);
    sig *= env;

        return sig;
    }



