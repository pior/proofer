#include "SH1106Wire.h"
#include <CircularBuffer.h>

inline String tempAsString(float temp) {
  return String(temp, 1) + "ºC";
}

class TempLogger {
  private:
    const unsigned int historyPeriodMs = 30 * 1000;

    float current, minimum, maximum;
    CircularBuffer<float,114> history;
    unsigned long lastHistoryUpdate;

  public:
    void Update(float temp) {
      current = temp;

      unsigned int now = millis();
      if (now - lastHistoryUpdate > historyPeriodMs) {
        history.push(temp);
        lastHistoryUpdate = (now / historyPeriodMs) * historyPeriodMs;
      }

      auto _minimum = history[0];
      auto _maximum = history[0];
      for (int i=0; i< history.size(); i++) {
        _minimum = min(_minimum, history[i]);
        _maximum = max(_maximum, history[i]);
      }
      minimum = _minimum;
      maximum = _maximum;
    }

    int HistorySize() { return history.size(); }
    float History(int index) { return history[index]; }
    float Current() { return current; }
    float Minimum() { return minimum; }
    float Maximum() { return maximum; }
};



class Display {
  private:
    SH1106Wire display;

  public:
    Display() : display(0x3c, SDA, SCL) {}

    void Initialize() {
      display.init();
      // display.flipScreenVertically();
      display.setTextAlignment(TEXT_ALIGN_LEFT);
      display.clear();
      display.display();
    }

    void Update(TempLogger& temp, bool controlEnabled, int controlPower, float setPoint) {
      display.clear();

      display.drawString(0, 0, tempAsString(temp.Current()));
      display.drawString(36, 0, controlEnabled ? "ON" : "OFF");
      display.drawString(62, 0, String(controlPower) + "%");
      display.drawString(92, 0, tempAsString(setPoint));

      drawGraph(temp, setPoint);

      display.display();
    }

  private:
    void drawGraph(TempLogger& temp, float setPoint) {
      const int pixelOriginY = 29;
      const int pixelOriginX = 14;
      const int pixelLengthY = 34;
      const int pixelLengthX = 114;

      int tempCeiling = ceil(max(temp.Maximum(), setPoint));
      int tempFloor = floor(min(temp.Minimum(), setPoint));
      if (tempFloor == tempCeiling) { tempFloor--; };
      float tempDelta = tempCeiling - tempFloor;

      // X axis
      display.drawLine(pixelOriginX - 1, pixelOriginY, pixelOriginX - 1, pixelOriginY + pixelLengthY);

      // X ticks
      display.drawString(0, 27, String(tempCeiling));
      display.drawString(0, 54, String(tempFloor));

      auto yForTemp = [&](float temp) -> int {
        auto relativePosition = (temp - tempFloor) / tempDelta;
        return pixelOriginY + pixelLengthY - (pixelLengthY * relativePosition);
      };

      int pos, lastPos;

      pos = yForTemp(setPoint);
      for (int i=0; i < pixelLengthX; i+=3) {
        display.setPixel(pixelOriginX + i, pos);
      }

      // plot the temp
      for (int i=0; i < min(pixelLengthX, temp.HistorySize()); i++) {
        pos = yForTemp(temp.History(i));
        if (i == 0) { lastPos = pos; };

        display.drawLine(pixelOriginX + i, pos, pixelOriginX + i - 1, lastPos);
        lastPos = pos;
      }
    }

};
