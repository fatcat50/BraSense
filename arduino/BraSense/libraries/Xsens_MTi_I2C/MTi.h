#ifndef MTi_h
#define MTi_h

#include "Xbus.h"

class MTi {
  public:
    MTi(uint8_t x, uint8_t y) : xbus() {
      address = x;
      drdy = y;
    }
    uint8_t drdy;
    uint8_t address;
    bool detect(uint16_t timeout);
    void requestDeviceInfo();
    void configureOutputs();
    void goToConfig();
    void goToMeasurement();
    void readMessages();
    void printData();

  public:
    Xbus xbus;
    void sendMessage(uint8_t *message, uint8_t numBytes);

    float* getEulerAngles() {
      return xbus.euler;
    }
    float* getAcceleration() {
      return xbus.acc;
    }
    float* getRateOfTurn() {
      return xbus.rot;
    }
    float* getLatLon() {
      return xbus.latlon;
    }
};

#endif
