#include "measurement.h"
<<<<<<< Updated upstream
=======
#include "websocket_handler.h"
#include "sd_handler.h"

#define BUTTON_PIN 2
#define DEBOUNCE_DELAY 50
>>>>>>> Stashed changes

bool isMeasuring = false;
uint16_t recordCounter = 0;
MTi *MyMTi = NULL;

<<<<<<< Updated upstream
void initMTi() {
=======
void print()
{
    Serial.print("Messung " + String(recordCounter) + " : ");
    Serial.println(isMeasuring ? "GESTARTET" : "GESTOPPT");
    //digitalWrite(LED_BUILTIN, isMeasuring);
}

void initMTi()
{
    pinMode(3, INPUT);

>>>>>>> Stashed changes
    MyMTi = new MTi(0x6B, 3);
    if (!MyMTi->detect(1000)) {
        Serial.println("MTi nicht erkannt. Überprüfen Sie die Verbindungen.");
        while (1);
    }
    MyMTi->goToConfig();
    MyMTi->requestDeviceInfo();
    MyMTi->configureOutputs();
    MyMTi->goToMeasurement();
}

void startMeasurement() {
    isMeasuring = true;
    recordCounter++;
<<<<<<< Updated upstream
    Serial.println("Messung gestartet.");
=======

    openFile();

    //if (file)
    //{
        file.println(">>>> Aufnahme " + String(recordCounter) + " START <<<<");
        //file.close();
        print();
        ws.textAll(String(isMeasuring));
    //}
    //else
    //{
      //  Serial.println("Fehler beim Oeffnen der Datei (START).");
    //}
>>>>>>> Stashed changes
}

void stopMeasurement() {
    isMeasuring = false;
<<<<<<< Updated upstream
    Serial.println("Messung gestoppt.");
}

void logMeasurementData() {
    if (MyMTi == NULL) return;
    Serial.println("Messdaten werden geloggt...");
=======
    // File file = SD.open(currentFileName, FILE_APPEND);
    // if (file)
    //{
    file.println(">>>> Aufnahme " + String(recordCounter) + " STOPP <<<<");
    closeFile();
    print();
    ws.textAll(String(isMeasuring));
    //}
    // else
    //{
    // Serial.println("Fehler beim Öffnen der Datei (STOPP).");
    //}
}

void logMeasurementData()
{
    char cnt[32];
    dtostrf(measurementCounter, 8, 2, cnt);

    // File file = SD.open(currentFileName, FILE_APPEND);
    // if (file)
    //{
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
    //file.flush();
    // file.close();
    //}
    // else
    //{
    //  Serial.println("Fehler beim Öffnen der Datei (logData).");
    //}
}

void initMeasurement()
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    //pinMode(LED_BUILTIN, OUTPUT);
    //digitalWrite(LED_BUILTIN, LOW);
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
            }
        }
    }
    lastButtonState = reading;
>>>>>>> Stashed changes
}