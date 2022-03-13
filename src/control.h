#include <MiniPID.h>

const int HEATER_PIN = 16;

MiniPID pid = MiniPID(100, 2, 500); // in %

void setupControl() {
  pinMode(HEATER_PIN, OUTPUT);
  analogWrite(HEATER_PIN, 0);

  pid.setOutputLimits(-100, 100); // specify a symmetrical output to avoid resetting errorSum when output is negative.
}

class Control {
  private:
    bool enabled = true;

    float setPoint = 28;
    const int setPointMin = 24;
    const int setPointMax = 35;

    float output = 0;

  public:
    void ToggleEnabled() {
      enabled = !enabled;
      setOutput();

      Serial.print("enabled="); Serial.println(enabled);
    }

    void ChangeTemp(float tempDiff) {
      setPoint += tempDiff;
      if (setPoint > setPointMax) {
        setPoint = setPointMin;
      }

      Serial.print("setPoint="); Serial.println(setPoint);
    }

    float Temp() {
      return setPoint;
    }

    bool Enabled() {
        return enabled;
    }

    int PowerPct() {
      if (enabled) {
        return output;
      }

      return 0;
    }

    void Refresh(float currentTemp) {
      output = pid.getOutput(currentTemp, setPoint);

      if (output < 0) { output = 0; }


      // if ((setPoint - currentTemp) > 1) { // 100% ON when 1ºC below
      //   output = 100;
      // }

      setOutput();
    }

  private:
    void setOutput() {
      analogWrite(HEATER_PIN, enabled ? (output * PWMRANGE / 100) : 0);
    }
};


