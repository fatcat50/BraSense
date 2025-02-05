#include "measurement.h"

#define BUTTON_PIN 2
#define DEBOUNCE_DELAY 50

bool isMeasuring = false;
uint16_t recordCounter = 0;
uint32_t measurementCounter = 0;
bool buttonState = HIGH;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
MTi *MyMTi = NULL;

void initMTi()
{
    pinMode(3, INPUT);

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
    }
    else
    {
        Serial.println("Fehler beim Oeffnen der Datei (START).");
    }
}

void stopMeasurement()
{
    File file = SD.open(currentFileName, FILE_APPEND);
    if (file)
    {
        file.println(">>>> Aufnahme " + String(recordCounter) + " STOPP <<<<");
        file.close();
    }
    else
    {
        Serial.println("Fehler beim Öffnen der Datei (STOPP).");
    }
    isMeasuring = false;
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

void initMeasurement()
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
}

void handleButtonPress()
{
    int reading = digitalRead(BUTTON_PIN);

    if (reading != lastButtonState)
    {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY)
    {
        if (reading != buttonState)
        {
            buttonState = reading;
            if (buttonState == LOW)
            {
                isMeasuring = !isMeasuring;

                if (isMeasuring)
                {
                    startMeasurement();
                }
                else
                {
                    stopMeasurement();
                }

                Serial.print("Messung " + String(recordCounter) +  " : " );
                Serial.println(isMeasuring ? "GESTARTET" : "GESTOPPT");
                digitalWrite(LED_BUILTIN, isMeasuring);
            }
        }
    }
    lastButtonState = reading;
}