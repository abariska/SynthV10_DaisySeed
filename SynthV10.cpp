#include "SynthV10.h"
#include "daisy.h"
#include "SX1509_extender.h"
#include "Display.h"

using namespace daisy;

DaisySeed hw;
TimerHandle timer1, timer2, timer3;
TimerHandle::Config timer_conf1, timer_conf2, timer_conf3;


int encoderIncs[5] = {0, 0, 0, 0, 0};
int encoderValues[5] = {0, 0, 0, 0, 0};
volatile int testVal = 0;
volatile int encoder_update_flag = 0;
volatile int button_update_flag = 0;
volatile int led_update_flag = 0;

int main(void)
{
    hw.Configure();
    hw.Init();

    hw.StartLog(true);

    InitSX1509Extenders(); 
    // InitDisplayPages();
    // OLED_1in5_Init();

    // Paint_NewImage(background_black, FULL_PAGE_WIDTH, FULL_PAGE_HEIGHT, 0, BLACK);
    // Paint_SetScale(16);
    // Paint_Clear(BLACK);

    hw.DelayMs(100);
    
    TimersInit();
    
    while (1)
    {

            // Paint_Clear(BLACK);

        
        // sx1509_leds.ReadAllPins();
        // RawPinState();
        ProcessEncoders();
        ProcessButtons();
        ProcessLeds();
        // OLED_1in5_Display(background_black);

        
        // if (encoderIncs[0] != 0) {
        //     hw.PrintLine("Encoder 1: %d", encoderIncs[0]);
        // }
    }
}

void RawPinState() {
    char butStr[17];
    char encStr[17]; 
    char ledStr[17]; // 16 біт + '\0'

    for (int i = 0; i < 16; i++) {
        butStr[i] = (sx1509_buttons.current_state >> i & 1) ? '1' : '0';
        encStr[i] = (sx1509_encoders.current_state >> i & 1) ? '1' : '0';
        ledStr[i] = (sx1509_leds.current_state >> i & 1) ? '1' : '0';
    }
    butStr[16] = '\0';
    encStr[16] = '\0';
    ledStr[16] = '\0';
    hw.PrintLine("Buttons raw: %s", butStr);
    Paint_DrawString_EN(10, 60, butStr, &Font12, WHITE, BLACK);
    hw.PrintLine("Encoders raw: %s", encStr);
    Paint_DrawString_EN(10, 80, encStr, &Font12, WHITE, BLACK);
    hw.PrintLine("Leds raw: %s", ledStr);
    Paint_DrawString_EN(10, 100, ledStr, &Font12, WHITE, BLACK);
}

void ProcessButtons() {

    char buttonStr[16][2];
    if (button_update_flag) {
        if (sx1509_buttons.ReadAllPins()) {
            for (int i = 0; i < 16; i++) {
                uint16_t buttonState = sx1509_buttons.current_state >> i & 1;
                sprintf(buttonStr[i], "%d", !(buttonState));
            }
            
        
            hw.PrintLine("But1: %s" "But2: %s" "But3: %s" "But4: %s" "But5: %s" "But6: %s" "But7: %s" "But8: %s" "But9: %s" "But10: %s" "But11: %s" "But12: %s" "But13: %s" "But14: %s" "But15: %s" "But16: %s", 
                buttonStr[0], buttonStr[1], buttonStr[2], buttonStr[3], buttonStr[4], buttonStr[5], buttonStr[6], buttonStr[7], buttonStr[8], buttonStr[9], buttonStr[10], buttonStr[11], buttonStr[12], buttonStr[13], buttonStr[14], buttonStr[15]);
    
        }
        button_update_flag = 0;
    }

        // Paint_DrawChar(0, 0, buttonStr[0][0], &Font12, WHITE, BLACK);
        // Paint_DrawChar(10, 0, buttonStr[1][0], &Font12, WHITE, BLACK);
        // Paint_DrawChar(20, 0, buttonStr[2][0], &Font12, WHITE, BLACK);
        // Paint_DrawChar(30, 0, buttonStr[3][0], &Font12, WHITE, BLACK);
        // Paint_DrawChar(40, 0, buttonStr[4][0], &Font12, WHITE, BLACK);
        // Paint_DrawChar(50, 0, buttonStr[5][0], &Font12, WHITE, BLACK);
        // Paint_DrawChar(60, 0, buttonStr[6][0], &Font12, WHITE, BLACK);
        // Paint_DrawChar(70, 0, buttonStr[7][0], &Font12, WHITE, BLACK);
        // Paint_DrawChar(80, 0, buttonStr[8][0], &Font12, WHITE, BLACK);
        // Paint_DrawChar(90, 0, buttonStr[9][0], &Font12, WHITE, BLACK);
        // Paint_DrawChar(100, 0, buttonStr[10][0], &Font12, WHITE, BLACK);

        // Paint_DrawChar(0, 20, buttonStr[11][0], &Font12, WHITE, BLACK);
        // Paint_DrawChar(20, 20, buttonStr[12][0], &Font12, WHITE, BLACK);
        // Paint_DrawChar(40, 20, buttonStr[13][0], &Font12, WHITE, BLACK);
        // Paint_DrawChar(60, 20, buttonStr[14][0], &Font12, WHITE, BLACK);
        // Paint_DrawChar(80, 20, buttonStr[15][0], &Font12, WHITE, BLACK);
    
    if (sx1509_buttons.IsPressed(8)){
        static uint8_t led_state[16] = {0};
        for (int i = 0; i < 16; i++) {

            if (sx1509_buttons.isFallingEdge(i)) {
                led_state[i] = !led_state[i];
                sx1509_leds.WritePin(i, led_state[i]);
            }
        }
    }
}


void ProcessEncoders(){

    // static char encoderStr[16][2];
    // for (int i = 0; i < 16; i++) {
    //     sprintf(encoderStr[i], "%d", sx1509_encoders.CurrentPinState(i));
    // }

    // Paint_DrawChar(0, 40, encoderStr[0][0], &Font12, WHITE, BLACK);
    // Paint_DrawChar(10, 40, encoderStr[1][0], &Font12, WHITE, BLACK);
    // Paint_DrawChar(20, 40, encoderStr[2][0], &Font12, WHITE, BLACK);
    // Paint_DrawChar(30, 40, encoderStr[3][0], &Font12, WHITE, BLACK);
    // Paint_DrawChar(40, 40, encoderStr[4][0], &Font12, WHITE, BLACK);
    // Paint_DrawChar(50, 40, encoderStr[5][0], &Font12, WHITE, BLACK);
    // Paint_DrawChar(60, 40, encoderStr[6][0], &Font12, WHITE, BLACK);
    // Paint_DrawChar(70, 40, encoderStr[7][0], &Font12, WHITE, BLACK);
    // Paint_DrawChar(80, 40, encoderStr[8][0], &Font12, WHITE, BLACK);
    // Paint_DrawChar(90, 40, encoderStr[9][0], &Font12, WHITE, BLACK);
    // Paint_DrawChar(100, 40, encoderStr[10][0], &Font12, WHITE, BLACK);

    char encoderValStr1[4];
    char encoderValStr2[4];
    char encoderValStr3[4];
    char encoderValStr4[4];
    char encoderValStr5[4];
    
    if(encoder_update_flag){
    if (sx1509_encoders.ReadAllPins())
    {
    encoder_update_flag = 0;

    encoderIncs[0] = EncoderInc(0, ENC_1_A, ENC_1_B);  
    encoderIncs[1] = EncoderInc(1, ENC_2_A, ENC_2_B);  
    encoderIncs[2] = EncoderInc(2, ENC_3_A, ENC_3_B);  
    encoderIncs[3] = EncoderInc(3, ENC_4_A, ENC_4_B);  
    encoderIncs[4] = EncoderInc(4, ENC_DIAL_A, ENC_DIAL_B); 
    
    encoderValues[0] += encoderIncs[0];
    sprintf(encoderValStr1, "%d", encoderValues[0]);
    Paint_DrawString_EN(0, 40, encoderValStr1, &Font12, WHITE, BLACK);
    encoderValues[1] += encoderIncs[1];
    sprintf(encoderValStr2, "%d", encoderValues[1]);
    Paint_DrawString_EN(20, 40, encoderValStr2, &Font12, WHITE, BLACK);
    encoderValues[2] += encoderIncs[2];
    sprintf(encoderValStr3, "%d", encoderValues[2]);
    Paint_DrawString_EN(40, 40, encoderValStr3, &Font12, WHITE, BLACK);
    encoderValues[3] += encoderIncs[3];
    sprintf(encoderValStr4, "%d", encoderValues[3]);
    Paint_DrawString_EN(60, 40, encoderValStr4, &Font12, WHITE, BLACK);
    encoderValues[4] += encoderIncs[4];
    sprintf(encoderValStr5, "%d", encoderValues[4]);
    Paint_DrawString_EN(80, 40, encoderValStr5, &Font12, WHITE, BLACK);

    hw.PrintLine("Enc1: %d" "Enc2: %d" "Enc3: %d" "Enc4: %d" "Enc5: %d" "Test: %d"  , 
        encoderValues[0], encoderValues[1], encoderValues[2], encoderValues[3], encoderValues[4], testVal);
    
    }
}
}

void ProcessLeds() {
    
    char ledStr[16][2];
    if (led_update_flag) {
        if (sx1509_leds.ReadAllPins()) {
            for (int i = 0; i < 16; i++) {
                uint16_t ledState = sx1509_leds.CurrentPinState(i);
                sprintf(ledStr[i], "%d", !(ledState));
            }
            
        
            hw.PrintLine("Led1: %s" "Led2: %s" "Led3: %s" "Led4: %s" "Led5: %s" "Led6: %s" "Led7: %s" "Led8: %s" "Led9: %s" "Led10: %s" "Led11: %s" "Led12: %s" "Led13: %s" "Led14: %s" "Led15: %s" "Led16: %s", 
                ledStr[0], ledStr[1], ledStr[2], ledStr[3], ledStr[4], ledStr[5], ledStr[6], ledStr[7], ledStr[8], ledStr[9], ledStr[10], ledStr[11], ledStr[12], ledStr[13], ledStr[14], ledStr[15]);
    
        }

        led_update_flag = 0;
    }
}

void TimersInit() {
    timer_conf1.periph = TimerHandle::Config::Peripheral::TIM_3;
    timer_conf1.dir = TimerHandle::Config::CounterDir::UP;
    timer_conf1.period = 5000;
    timer_conf1.enable_irq = true;
    timer1.Init(timer_conf1);
    
    timer1.SetCallback([](void* data) {
        encoder_update_flag = 1;
        testVal++;
    });
    timer1.Start();

    timer_conf2.periph = TimerHandle::Config::Peripheral::TIM_4;
    timer_conf2.dir = TimerHandle::Config::CounterDir::UP;
    timer_conf2.period = 100;
    timer_conf2.enable_irq = true;
    timer2.Init(timer_conf2);
    
    timer2.SetCallback([](void* data) {
        button_update_flag = 1;
        led_update_flag = 1;
    });
    timer2.Start();
}