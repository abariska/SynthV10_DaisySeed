#include "Button.h"

// Використання глобальної змінної hw з daisy_seed.h
// using namespace daisy;
// extern DaisySeed hw;

// Конструктор з можливістю інверсії
Button_mcp::Button_mcp(Mcp23017& mcp_ref, int pinValue, bool invert) 
    : mcp(mcp_ref), pin(pinValue), inverted(invert) {
    state_ = 0;
    updated_ = false;
    last_update_ = 0;
    rising_edge_time_ = 0.0f;
}

// Ініціалізація кнопки
void Button_mcp::Init() {
    // Ініціалізація кнопки
    state_ = 0;
    updated_ = false;
    last_update_ = 0;
    rising_edge_time_ = 0.0f;
}

// Метод для дебаунсингу кнопки (окремий, як в libDaisy)
void Button_mcp::Debounce() {
    bool new_val = mcp.ReadPin(pin);
    
    // Інвертуємо значення якщо потрібно
    if (inverted) {
        new_val = !new_val;
    }
    
    // Зсуваємо стан вліво і додаємо новий біт
    state_ = (state_ << 1) | (new_val ? 1 : 0);
}

// Оновлення стану кнопки - викликає Debounce() для сумісності
void Button_mcp::Update(unsigned long currentTime) {
    if (currentTime - last_update_ >= 1) { // 1ms інтервал
        Debounce();
        last_update_ = currentTime;
        updated_ = true;
    }
}

// Перевірка, чи була кнопка щойно натиснута (фронт наростання)
bool Button_mcp::RisingEdge() const {
    return (state_ & 0x03) == 0x02; // Перевіряємо тільки останні 2 біти
}

// Перевірка, чи була кнопка щойно відпущена (фронт спадання)
bool Button_mcp::FallingEdge() const {
    return (state_ & 0x03) == 0x01; // Перевіряємо тільки останні 2 біти
}

// Перевірка, чи натиснута кнопка
bool Button_mcp::IsPressed() const {
    return (state_ & 0x03) == 0x03; // Перевіряємо тільки останні 2 біти
}

// Перевірка, чи була кнопка щойно натиснута
bool Button_mcp::WasPressed() const {
    return RisingEdge();
}

// Перевірка, чи була кнопка щойно відпущена
bool Button_mcp::WasReleased() const {
    return FallingEdge();
}

// Перевірка, чи відбулося довге натискання
bool Button_mcp::IsLongPressed() const {
    return IsPressed() && TimeHeldMs() > 1000.0f;
}

// Отримати час натискання кнопки в мілісекундах
float Button_mcp::TimeHeldMs() const {
    return rising_edge_time_;
}

// Отримати поточний необроблений стан кнопки
bool Button_mcp::RawState() {
    bool raw = mcp.ReadPin(pin);
    return inverted ? !raw : raw;
}

void Button_mcp::UpdateParams(int pinValue, bool invert) {
    pin = pinValue;
    inverted = invert;
    Init();  // Переініціалізуємо кнопку з новими параметрами
}


