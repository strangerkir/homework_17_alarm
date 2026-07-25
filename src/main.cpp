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

struct buttonState {
  bool pressed = false;
  bool prevPressedState = false;
  bool newPressDetected = false;
};

struct alarmState alrm;
struct buttonState button;

void setup(void)
{
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetPinAttenuation(ADC_PIN, ADC_11db);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void updateButtonState(void) {
    int buttonValue = digitalRead(BUTTON_PIN);
    bool buttonPressed = (buttonValue == LOW);

    if(buttonPressed == button.prevPressedState){
      if(buttonPressed) {
        button.newPressDetected = false;
        Serial.println("Button still pressed, ignoring");
      }
      return;
    }

    button.prevPressedState = button.pressed;
    button.pressed = buttonPressed;
    button.newPressDetected = (button.pressed && !button.prevPressedState);
  }

void updateAlarmState(void) {
  int raw = analogRead(ADC_PIN);
  float volt = raw * 3.3 / 4096.0;
  Serial.printf("raw: %d, voltage %.3f \n", raw, volt);

  if (button.newPressDetected) {
    if(alrm.armed) {
      alrm.armed = false;
      alrm.triggered = false;
      Serial.println("Alarm disabled");
    } else if(volt >= ALARM_ARM_MIN_VOLTAGE) {
      alrm.armed = true;
      Serial.println("Arming the system");
    } else {
      Serial.println("Too dark to arm the system");
    }
  }

  if(alrm.armed && volt < ALARM_TRIGGER_VOLTAGE) {
    alrm.triggered = true;
    Serial.println("Alarm triggered!");
  }
}

void updateStates(void) {
    updateButtonState();
    updateAlarmState();

  }

void duty() {
  updateStates();
  while(alrm.armed && alrm.triggered) {
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);  
      delay(100);
      updateStates();
  }

  }

void loop(void)
{
  duty();
  delay(300);
}

