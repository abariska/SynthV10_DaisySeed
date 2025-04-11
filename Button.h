#ifndef BUTTON_H
#define BUTTON_H

#include "Mcp23017.h"

class Button_mcp {
private:
    Mcp23017& mcp;        // Посилання на контролер MCP23017
    int pin;              // Номер піну на MCP23017
    bool inverted;        // Прапорець інверсії логіки
    
    uint32_t last_update_;
    bool updated_;
    uint8_t state_;
    float rising_edge_time_;
    
public:
    // Конструктор з призначенням піну MCP23017 та можливістю інверсії
    Button_mcp(Mcp23017& mcp_ref, int pinValue, bool invert = false);
    
    // Ініціалізація кнопки
    void Init();
    
    // Оновлення параметрів кнопки
    void UpdateParams(int pinValue, bool invert);
    
    // Метод для дебаунсингу кнопки (окремий, як в libDaisy)
    void Debounce();
    
    // Оновлення стану кнопки - викликає Debounce() для сумісності
    void Update(unsigned long currentTime);
    
    // Перевірка, чи була кнопка щойно натиснута (фронт наростання)
    bool RisingEdge() const;
    
    // Перевірка, чи була кнопка щойно відпущена (фронт спадання)
    bool FallingEdge() const;
    
    // Перевірка, чи натиснута кнопка
    bool IsPressed() const;
    
    // Перевірка, чи була кнопка щойно натиснута (для сумісності)
    bool WasPressed() const;
    
    // Перевірка, чи була кнопка щойно відпущена (для сумісності)
    bool WasReleased() const;
    
    // Перевірка, чи відбулося довге натискання
    bool IsLongPressed() const;
    
    // Отримати час натискання кнопки в мілісекундах
    float TimeHeldMs() const;
    
    // Отримати поточний необроблений стан кнопки
    bool RawState();
    
    // Отримати поточне значення state_ для діагностики
    uint8_t GetState() const { return state_; }
};

#endif // BUTTON_H 