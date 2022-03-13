#include <MiniPID.h>

const int HEATER_PIN = 16;

MiniPID pid = MiniPID(500,300,200);

void setupControl() {
  pinMode(HEATER_PIN, OUTPUT);
  analogWrite(HEATER_PIN, 0);

  pid.setOutputLimits(0, PWMRANGE);
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
        return output * 100 / PWMRANGE;
      }

      return 0;
    }

    void Refresh(float currentTemp) {
      output = pid.getOutput(currentTemp, setPoint);

      if ((setPoint - currentTemp) > 1) { // 100% ON when 1ºC below
        output = PWMRANGE;
      }

      setOutput();
    }

  private:
    void setOutput() {
      analogWrite(HEATER_PIN, enabled ? output : 0);
    }
};


