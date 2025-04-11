#ifndef PARAMETERS_H
#define PARAMETERS_H

#include <array>
#include <cstdint>
#include "daisysp.h"

// Потрібно для структур масивів
#define OSC_NUM 3

using namespace daisysp;

// Перелік типів ефектів
enum EffectType {
    EFFECT_NONE,
    EFFECT_OVERDRIVE,
    EFFECT_CHORUS,
    EFFECT_COMPRESSOR,
    EFFECT_REVERB
};

// Структура параметрів ефекту Overdrive
struct OverdriveParams {
    float drive;          // Рівень драйву
    bool isActive;        // Чи активний ефект
};

// Структура параметрів ефекту Chorus
struct ChorusParams {
    float lfoFreq;        // Частота LFO
    float lfoDepth;       // Глибина LFO
    float delay;          // Затримка
    float feedback;       // Зворотній зв'язок
    float pan;            // Панорама
    bool isActive;        // Чи активний ефект
};

// Структура параметрів ефекту Compressor
struct CompressorParams {
    float attack;         // Час атаки
    float release;        // Час релізу
    float threshold;      // Поріг
    float ratio;          // Співвідношення
    float makeup;         // Підсилення
    bool isActive;        // Чи активний ефект
};

// Структура параметрів ефекту Reverb
struct ReverbParams {
    float dryWet;         // Баланс сухого/мокрого сигналу
    float feedback;       // Зворотній зв'язок
    float lpFreq;         // Частота фільтра низьких частот
    bool isActive;        // Чи активний ефект
};

// Структура параметрів блоку ефектів
struct EffectUnitParams {
    OverdriveParams overdrive;     // Параметри Overdrive
    ChorusParams chorus;           // Параметри Chorus
    CompressorParams compressor;   // Параметри Compressor
    ReverbParams reverb;           // Параметри Reverb
    bool isActive;                 // Чи активний блок ефектів
    EffectType activeEffect;       // Який ефект активний зараз
};

// Структура для збереження параметрів синтезатора
struct SynthParams {
    // Один голос-шаблон із усіма налаштуваннями
    struct {
        struct {
            int waveform;
            float pw;
            float amp;
            int pitch;
            float detune;
            bool active;
            float freq; 
            float pan; 
        } osc[OSC_NUM];
        
        struct {
            float cutoff;
            float resonance;
        } filter;
        
        struct {
            float attack;
            float decay;
            float sustain;
            float release;
            bool retrigger;
        } adsr;
    } voice;  // Один голос-шаблон
    
    // Глобальний LFO
    struct {
        float freq;
        float amp;
        int waveform;
        float pw;
    } lfo;

    struct {
        float tuning;
        bool isMono;
        bool isUnison;
    } global;

    EffectUnitParams effectUnits[2];
};

// Глобальна змінна для доступу до параметрів
extern SynthParams params;

// Forward declarations
struct Synth;
struct Effects;

// Функції для ініціалізації параметрів
void InitSynthParams();
void InitEffectParams();

// Функції для оновлення параметрів
void UpdateParams(Synth& synth, Effects& effects);

// Функція для встановлення активного ефекту
void SetActiveEffect(int unitIndex, EffectType effectType);

// Функція для отримання назви ефекту у вигляді рядка
inline const char* GetEffectName(EffectType effect) {
    switch (effect) {
        case EFFECT_NONE:        return " - ";
        case EFFECT_OVERDRIVE:   return "Overdrive";
        case EFFECT_CHORUS:      return "Chorus";
        case EFFECT_COMPRESSOR:  return "Compressor";
        case EFFECT_REVERB:      return "Reverb";
        default:          return "Unknown";
    }
}

// Функція для перемикання активності блоку ефектів
inline void ToggleEffectUnitActive(int unitIndex) {
    if (unitIndex >= 0 && unitIndex < 2) {
        params.effectUnits[unitIndex].isActive = !params.effectUnits[unitIndex].isActive;
    }
}

// Функції для збереження/завантаження пресетів
void SavePreset(uint8_t presetNumber);
void LoadPreset(uint8_t presetNumber);

#endif // PARAMETERS_H
