#include <FastPID.h>

const int HEATER_PIN = 16;

float Kp=10, Ki=2, Kd=3, Hz=2;
int output_bits = 8;
bool output_signed = false;
FastPID myPID(Kp, Ki, Kd, Hz, output_bits, output_signed);

void setupControl() {
  pinMode(HEATER_PIN, OUTPUT);
  analogWrite(HEATER_PIN, 0);

  myPID.setOutputRange(0, PWMRANGE);
}

class Control {
  private:
    bool enabled = false;

    float setPoint = 25;
    const int setPointMin = 24;
    const int setPointMax = 30;

    uint16_t output = 0;

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
      output = myPID.step(setPoint*10, currentTemp*10);
      setOutput();
    }

  private:
    void setOutput() {
      analogWrite(HEATER_PIN, enabled ? output : 0);
    }
};


