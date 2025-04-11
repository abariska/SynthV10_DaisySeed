#ifndef LED_H
#define LED_H

#include "Mcp23017.h"

class Led_mcp {
private:
    Mcp23017& mcp;       // Посилання на контролер MCP23017
    int pin;             // Номер піну на MCP23017
    bool state;          // Поточний стан LED (увімкнено/вимкнено)
    bool inverted;       // Інверсія логіки LED
    
public:
    // Конструктор з призначенням піну MCP23017
    Led_mcp(Mcp23017& mcp, int pin, bool inverted = false)
        : mcp(mcp), pin(pin), state(false), inverted(inverted) {}
    
    // Увімкнути LED
    void On() {
        mcp.WritePin(pin, inverted ? 0 : 1);
        state = true;
    }
    
    // Вимкнути LED
    void Off() {
        mcp.WritePin(pin, inverted ? 1 : 0);
        state = false;
    }
    
    // Переключити стан LED
    void Toggle() {
        state = !state;
        mcp.WritePin(pin, inverted ? !state : state);
    }
    
    // Встановити стан LED
    void SetState(bool newState) {
        state = newState;
        mcp.WritePin(pin, inverted ? !state : state);
    }
    
    // Отримати поточний стан LED
    bool GetState() const {
        return state;
    }
};


#endif // LED_H 