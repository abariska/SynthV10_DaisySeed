#include "Encoder_mcp.h"

extern DaisySeed hw;

void Enc_mcp::Debounce()
{
    // Оновлення не частіше ніж 1кГц
    uint32_t now = System::GetNow();
    updated_     = false;

    if(now - last_update_ >= 1)
    {
        last_update_ = now;
        updated_     = true;

        // Зсув станів пінів для дебаунсу
        a_ = (a_ << 1) | mcp.GetPin(pin_a_);
        b_ = (b_ << 1) | mcp.GetPin(pin_b_);

        // Визначення напрямку обертання
        inc_ = 0; // скидаємо inc_ спочатку
        if((a_ & 0x03) == 0x02 && (b_ & 0x03) == 0x00)
        {
            inc_ = 1;
        }
        else if((b_ & 0x03) == 0x02 && (a_ & 0x03) == 0x00)
        {
            inc_ = -1;
        }

        if (inc_ != 0) {
            // Визначаємо швидкість обертання
            uint32_t time_diff = now - last_increment_time_;
            
            int    speed_factor_;  
            // Оновлюємо множник швидкості
            if (time_diff < 100) {  // Швидке обертання
                speed_factor_ = 5;
            } else if (time_diff < 70) {  // Дуже швидке обертання
                speed_factor_ = 10;
            } else if (time_diff < 50) {  // Дуже швидке обертання
                speed_factor_ = 100;
            } else {  // Повільне обертання
                speed_factor_ = 1;
            }
            
            // Оновлюємо час останньої зміни
            last_increment_time_ = now;
            
            // Застосовуємо множник швидкості
            inc_ *=  speed_factor_;
        }
        // Оновлюємо стан кнопки
        sw_.Update(now);

        // hw.PrintLine("Encoder: %d", test_int + inc_);
    }
}