#ifndef SD_HANDLER_H
#define SD_HANDLER_H

#include <Arduino.h>
#include <SD.h>
#include <EEPROM.h>

extern uint16_t fileCounter;
extern String currentFileName;

bool initSDCard();
void loadFileCounter();
void saveFileCounter();
void createNewMeasurementFile();
void appendFile(fs::FS &fs, const char *path, const char *message);

#endif // SD_HANDLER_H