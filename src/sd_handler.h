#ifndef SD_HANDLER_H
#define SD_HANDLER_H

#include <Arduino.h>
#include <EEPROM.h>
#include <SD.h>

extern uint16_t fileCounter;
extern String currentFileName;
extern File file;

bool initSDCard();
void loadFileCounter();
void saveFileCounter();
void createNewMeasurementFile();
void openFile();
void closeFile();

#endif  // SD_HANDLER_H