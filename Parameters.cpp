#include "Parameters.h"
#include "daisysp.h" // Додаємо для використання констант

using namespace daisysp;

// Визначення глобальної змінної params
SynthParams params;

// Ініціалізація параметрів - стара версія замінена новою у файлі Parameters.h
void InitSynthParams() {
    // Ініціалізація шаблону голосу
    params.voice.osc[0].active = true;
    params.voice.osc[1].active = false;
    params.voice.osc[2].active = false;
    for (int o = 0; o < OSC_NUM; o++) {
        params.voice.osc[o].waveform = 0;
        params.voice.osc[o].freq = 440.0f;
        params.voice.osc[o].pw = 0.5f;
        params.voice.osc[o].amp = 0.1f;
        params.voice.osc[o].pitch = 0;
        params.voice.osc[o].detune = 0;
    }
    
    params.voice.filter.cutoff = 1000.0f;
    params.voice.filter.resonance = 0.0f;
    
    params.voice.adsr.attack = 0.01f;
    params.voice.adsr.decay = 0.1f;
    params.voice.adsr.sustain = 1.0f;
    params.voice.adsr.release = 0.5f;
    params.voice.adsr.retrigger = false;
    
    // Ініціалізація глобального LFO
    params.lfo.freq = 0.5f;
    params.lfo.amp = 0.0f;
    params.lfo.waveform = 0;
    params.lfo.pw = 0.5f;

    params.global.isMono = true;
    params.global.isUnison = false;
    params.global.tuning = 0.0f;
    
    // Ініціалізація ефектів
    InitEffectParams();
}

void InitEffectParams() {
    // Ініціалізуємо параметри для кожного блоку ефектів
    for (int e = 0; e < 2; e++) {
        // Загальні налаштування блоку ефектів
        params.effectUnits[e].isActive = false;
        params.effectUnits[e].activeEffect = EFFECT_NONE;
        
        // Overdrive
        params.effectUnits[e].overdrive.drive = 0.0f;
        params.effectUnits[e].overdrive.isActive = false;
        
        // Chorus
        params.effectUnits[e].chorus.lfoFreq = 0.2f;
        params.effectUnits[e].chorus.lfoDepth = 0.0f;
        params.effectUnits[e].chorus.delay = 0.0f;
        params.effectUnits[e].chorus.feedback = 0.0f;
        params.effectUnits[e].chorus.pan = 0.5f;
        params.effectUnits[e].chorus.isActive = false;
        
        // Compressor
        params.effectUnits[e].compressor.attack = 0.01f;
        params.effectUnits[e].compressor.release = 0.01f;
        params.effectUnits[e].compressor.threshold = 0.0f;
        params.effectUnits[e].compressor.ratio = 1.0f;
        params.effectUnits[e].compressor.makeup = 0.0f;
        params.effectUnits[e].compressor.isActive = false;
        
        // Reverb
        params.effectUnits[e].reverb.feedback = 0.0f;
        params.effectUnits[e].reverb.dryWet = 0.0f;
        params.effectUnits[e].reverb.isActive = false;
    }
}

// Функція для встановлення активного ефекту
void SetActiveEffect(int unitIndex, EffectType effectType) {
    // Переконуємося, що індекс в межах допустимого діапазону
    if (unitIndex < 0 || unitIndex >= 2) return;
    
    // Встановлюємо активний ефект
    params.effectUnits[unitIndex].activeEffect = effectType;
    
    // Оновлюємо стан активності окремих ефектів у структурі params
    // Це забезпечує узгодженість між activeEffect та індивідуальними isActive
    params.effectUnits[unitIndex].overdrive.isActive = (effectType == EFFECT_OVERDRIVE);
    params.effectUnits[unitIndex].chorus.isActive = (effectType == EFFECT_CHORUS);
    params.effectUnits[unitIndex].compressor.isActive = (effectType == EFFECT_COMPRESSOR);
    params.effectUnits[unitIndex].reverb.isActive = (effectType == EFFECT_REVERB);
}

// Оновлення параметрів
// void UpdateParams(Synth& synth, Effects& effects) {
//     // Нічого не робимо, щоб уникнути проблем з неповними типами
//     // Оновлення об'єктів з параметрів відбувається напряму в SynthV10.cpp
// }

// // Заглушки для функцій збереження/завантаження пресетів
// void SavePreset(uint8_t presetNumber) {
//     // Тут буде код для збереження пресету
// }

// void LoadPreset(uint8_t presetNumber) {
//     // Тут буде код для завантаження пресету
// } 