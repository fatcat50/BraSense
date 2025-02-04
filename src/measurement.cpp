#include "measurement.h"

bool isMeasuring = false;
uint16_t recordCounter = 0;
uint32_t measurementCounter = 0;
MTi *MyMTi = NULL;

void initMTi()
{
    MyMTi = new MTi(0x6B, 3);
    if (!MyMTi->detect(1000))
    {
        Serial.println("MTi nicht erkannt. Überprüfen Sie die Verbindungen.");
        while (1)
            ;
    }
    MyMTi->goToConfig();
    MyMTi->requestDeviceInfo();
    MyMTi->configureOutputs();
    MyMTi->goToMeasurement();
}

void startMeasurement()
{
    isMeasuring = true;
    recordCounter++;

    // Ins File schreiben, dass eine neue Aufnahme gestartet wurde
    File file = SD.open(currentFileName, FILE_APPEND);
    if (file)
    {
        file.println(">>>> Aufnahme " + String(recordCounter) + " START <<<<");
        file.close();
        Serial.println("Aufnahme " + String(recordCounter) + " gestartet.");
    }
    else
    {
        Serial.println("Fehler beim Oeffnen der Datei (START).");
    }
    digitalWrite(LED_BUILTIN, HIGH);
}

void stopMeasurement()
{
    if (isMeasuring)
    {
        File file = SD.open(currentFileName, FILE_APPEND);
        if (file)
        {
            file.println(">>>> Aufnahme " + String(recordCounter) + " STOPP <<<<");
            file.close();
            Serial.println("Aufnahme gestoppt.");
        }
        else
        {
            Serial.println("Fehler beim Öffnen der Datei (STOPP).");
        }
        isMeasuring = false;
    }
    digitalWrite(LED_BUILTIN, LOW);
}

void logMeasurementData()
{
    char cnt[32];
    dtostrf(measurementCounter, 8, 2, cnt);

    File file = SD.open(currentFileName, FILE_APPEND);
    if (file)
    {
        file.print(cnt);
        file.print(" ; Acceleration [m/s^2]: ");

        for (int i = 0; i < 3; ++i)
        {
            char str[32];
            dtostrf(MyMTi->getAcceleration()[i], 8, 2, str);
            file.print(str);
            if (i < 2)
                file.print(" ");
        }
        file.println();
        file.close();
    }
    else
    {
        Serial.println("Fehler beim Öffnen der Datei (logData).");
    }
}