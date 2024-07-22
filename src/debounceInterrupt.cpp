#include "DebounceInterrupt.h"

// Inizializza il puntatore statico
DebounceInterrupt* DebounceInterrupt::instances[4] = {nullptr, nullptr, nullptr, nullptr};

DebounceInterrupt::DebounceInterrupt(uint8_t timerIndex, uint16_t prescaler, uint8_t pin, uint32_t timeout)
  : timerIndex(timerIndex), prescaler(prescaler), pin(pin), timeout(timeout), pressed(false) {
    // Inizializza il timer hardware
    timer = timerBegin(timerIndex, prescaler, true);
    timerAttachInterrupt(timer, &DebounceInterrupt::onTimerStatic, true);
    timerAlarmWrite(timer, timeout, false);

    // Imposta il pin come input
    pinMode(pin, INPUT_PULLUP);

    // Imposta l'interrupt sul pin
    attachInterruptArg(digitalPinToInterrupt(pin), DebounceInterrupt::handleInterruptStatic, this, FALLING);

    // Registra l'istanza corrente nella mappa
    instances[timerIndex] = this;
}

bool DebounceInterrupt::isPressed() const {
    return pressed;
}

void IRAM_ATTR DebounceInterrupt::handleInterruptStatic(void* arg) {
    DebounceInterrupt* self = static_cast<DebounceInterrupt*>(arg);
    self->handleInterrupt();
}

void IRAM_ATTR DebounceInterrupt::onTimerStatic() {
    for (int i = 0; i < 4; ++i) {
        if (instances[i]) {
            instances[i]->onTimer();
        }
    }
}

void IRAM_ATTR DebounceInterrupt::handleInterrupt() {
    portENTER_CRITICAL_ISR(&timerMux);  
    if(pulseCount > PUSLSE_COUNT_THRESHOLD)  
        pressed = true;
    pulseCount++;
    timerWrite(timer, 0);
    timerAlarmEnable(timer);
    portEXIT_CRITICAL_ISR(&timerMux);
}

void IRAM_ATTR DebounceInterrupt::onTimer() {
    portENTER_CRITICAL_ISR(&timerMux);
    pulseCount = 0;
    pressed = false;
    timerAlarmDisable(timer);
    portEXIT_CRITICAL_ISR(&timerMux);
}
