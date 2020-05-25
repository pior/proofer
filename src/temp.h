#include "SPI.h"
#include "Adafruit_MCP9808.h"

class Temp {
  private:
    Adafruit_MCP9808 sensor;

  public:
    void Initialize() {
      if (!sensor.begin(0x18)) {
        Serial.println("Couldn't find MCP9808! Check your connections and verify the address is correct.");
        while (1);
      }
      sensor.wake();
      // sensor.shutdown_wake(1);
      Serial.println("MCP9808 initialized");
    }

    float Read() {
      return sensor.readTempC();
    }
};
