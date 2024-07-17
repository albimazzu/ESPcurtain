#ifndef DEBOUNCEINTERRUPT_H
#define DEBOUNCEINTERRUPT_H

#include <Arduino.h>

class DebounceInterrupt {
public:
    DebounceInterrupt(uint8_t timerIndex, uint16_t prescaler, uint8_t pin, uint32_t timeout);
    bool isPressed() const;

private:
    static void IRAM_ATTR handleInterruptStatic(void* arg);
    static void IRAM_ATTR onTimerStatic();
    void IRAM_ATTR handleInterrupt();
    void IRAM_ATTR onTimer();

    uint8_t timerIndex;
    uint16_t prescaler;
    uint8_t pin;
    uint32_t timeout;
    volatile bool pinPressed;
    hw_timer_t* timer;
    portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

    static DebounceInterrupt* instances[4]; // Massimo 4 timer hardware
};

#endif // DEBOUNCEINTERRUPT_H
