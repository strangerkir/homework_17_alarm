#include <Arduino.h>
#include "driver/adc.h"

#define ADC_PIN 4
#define LED_PIN 18
#define BUTTON_PIN 16
#define ALARM_TRIGGER_VOLTAGE 2.0
#define ALARM_ARM_MIN_VOLTAGE 2.2

struct alarmState {
  bool armed = true;
  bool triggered = false;
};

struct alarmState state;

void setup(void)
{
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetPinAttenuation(ADC_PIN, ADC_11db);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void updateState(void) {
    int raw = analogRead(ADC_PIN);
    float volt = raw * 3.3 / 4096.0;
    Serial.printf("raw: %d, voltage %.3f \n", raw, volt);

    int buttonState = digitalRead(BUTTON_PIN);

    if (buttonState == LOW) {
      if(state.armed) {
        state.armed = false;
        state.triggered = false;
        Serial.println("Alarm disabled");
      } else if(volt >= ALARM_ARM_MIN_VOLTAGE) {
        state.armed = true;
        Serial.println("Arming the system");
      } else {
        Serial.println("Too dark to arm the system");
      }
    }

    if(state.armed && volt < ALARM_TRIGGER_VOLTAGE) {
      state.triggered = true;
      Serial.println("Alarm triggered!");
    }
  }

void duty() {
    updateState();
    while(state.armed && state.triggered) {
       digitalWrite(LED_PIN, HIGH);
       delay(100);
       digitalWrite(LED_PIN, LOW);  
       delay(100);
       updateState();
    }

  }

void loop(void)
{
  duty();
  delay(300);
}

