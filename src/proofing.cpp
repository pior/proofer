#include <Arduino.h>
#include <math.h>
#include <AceButton.h>
using namespace ace_button;

#define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass
#include <TaskScheduler.h>

#include "temp.h"
#include "control.h"
#include "display.h"

// Globals

Scheduler ts;
Control control;
Temp temp;
TempLogger tempLog;
Display display;

// Hardware

const int SW1_PIN = 12;
const int SW2_PIN = 13;
const int SW3_PIN = 14;

AceButton sw1(SW1_PIN);
AceButton sw2(SW2_PIN);
AceButton sw3(SW3_PIN);


// Routines

void buttonHandler(AceButton* button, uint8_t eventType, uint8_t buttonState) {
  if (eventType != AceButton::kEventPressed) {
    return;
  }

  switch (button->getPin()) {
  case SW1_PIN:
    control.ToggleEnabled();
    break;

  case SW2_PIN:
    control.ChangeTemp(1);
    break;

  case SW3_PIN:
    Serial.print("sw3 pressed\n");
    break;
  }
}

// Tasks

bool EMATempInitialized = false;
float EMATemp; // Exponential Moving Average

void updateEMATemp(float current) {
  if (!EMATempInitialized) {
    EMATemp = current;
    EMATempInitialized = true;
  }

  // https://www.investopedia.com/ask/answers/122314/what-exponential-moving-average-ema-formula-and-how-ema-calculated.asp
  float n = 60;
  float k = 2 / (n + 1);
  EMATemp = current * k + EMATemp * (1 - k);
}

void taskMeasureCallback() {
  auto current = temp.Read();

  tempLog.Update(current);

  updateEMATemp(current);
}
Task taskMeasure(500, TASK_FOREVER, &taskMeasureCallback);

void taskPIDCallback() {
  control.Refresh(EMATemp);
}
Task taskPID(5000, TASK_FOREVER, &taskPIDCallback);

void taskDisplayCallback() {
  display.Update(tempLog, control.Enabled(), control.PowerPct(), control.Temp());
}
Task taskDisplay(200, TASK_FOREVER, &taskDisplayCallback);

// SETUP-LOOP

void setup() {
  Serial.begin(115200);
  Serial.println("\nSetup...");

  // Heater
  setupControl();

  // Buttons
  pinMode(SW1_PIN, INPUT_PULLUP);
  pinMode(SW2_PIN, INPUT_PULLUP);
  pinMode(SW3_PIN, INPUT_PULLUP);
  ButtonConfig::getSystemButtonConfig()->setEventHandler(buttonHandler);

  temp.Initialize();
  display.Initialize();

  // Start task scheduler
  ts.init();
  ts.addTask(taskMeasure);
  ts.addTask(taskDisplay);
  ts.addTask(taskPID);
  ts.enableAll();

  Serial.println("Started.");
}

void loop() {
  ts.execute();
  sw1.check();
  sw2.check();
  sw3.check();
}
