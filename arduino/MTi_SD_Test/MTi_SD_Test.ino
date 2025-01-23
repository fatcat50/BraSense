//This library demonstrates basic Xbus communication between an Arduino (Uno) and an Xsens MTi 1-series shield board (MTi-#-DK) using the I2C interface.

//At least the following hardware connections are required:
//MTi_SCL - Arduino_SCL
//MTi_SDA - Arduino_SDA
//MTi_GND - Arduino_GND
//MTi_3.3V - Arduino_3.3V
//MTi_DRDY - Arduino_D3
//Additionally, make sure to:
//  -Enable I2C by switching PSEL0,1 to "1"
//  -Supply the MTi-#-DK with 3.3V (since 5V will force USB mode)
//  -Add 2.7 kOhm pullup resistors to the SCL/SDA lines (only for MTi-#-DK board Rev 2.3 or older - newer revisions come with onboard pullup resistors)

//This example code is merely a starting point for code development. Its functionality can be extended by making use of the Xbus protocol.
//Xbus is the proprietary binary communication protocol that is supported by all Xsens MTi products, and it is fully documented at https://mtidocs.xsens.com/mt-low-level-communication-protocol-documentation

//For further details on this example code, please refer to BASE: https://base.xsens.com/s/article/Interfacing-the-MTi-1-series-DK-with-an-Arduino?language=en_US

#include "Arduino.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "MTi.h"
#include <Wire.h>

#define DRDY 3                      //Arduino Digital IO pin used as input for MTi-DRDY
#define ADDRESS 0x6B                //MTi I2C address 0x6B (default I2C address for MTi 1-series)
MTi *MyMTi = NULL;

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

void testFileIO(fs::FS &fs, const char * path){
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }


    file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++){
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}

void setup() {
  Serial.begin(115200);               //Initialize communication for serial monitor output (Ctrl+Shift+M)
  Wire.begin();                     //Initialize Wire library for I2C communication
  pinMode(DRDY, INPUT);             //Data Ready pin, indicates whether data/notifications are available to be read
  
  if(!SD.begin()){
      Serial.println("Card Mount Failed");
      return;
  }
  
  writeFile(SD, "/hello.txt", "");
  
  MyMTi = new MTi(ADDRESS, DRDY);   //Create our new MTi object

  if (!MyMTi->detect(1000)) {       //Check if MTi is detected before moving on
    Serial.println("Please check your hardware connections.");
    while (1) {
      //Cannot continue because no MTi was detected.
    }
  } else {
    MyMTi->goToConfig();            //Switch device to Config mode
    MyMTi->requestDeviceInfo();     //Request the device's product code and firmware version
    MyMTi->configureOutputs();      //Configure the device's outputs based on its functionality. See MTi::configureOutputs() for more alternative output configurations.
    MyMTi->goToMeasurement();       //Switch device to Measurement mode
  }
}

void loop() {
  if (digitalRead(MyMTi->drdy)) {   //MTi reports that new data/notifications are available
    MyMTi->readMessages();          //Read new data messages
    MyMTi->printData();             //...and print them to the serial monitor (Ctrl+Shift+M)
    String dataString = "";

    if (!isnan(MyMTi->getEulerAngles()[0])) {                                                                        //Only true if this data has been received once before
      appendFile(SD, "/hello.txt", "Euler angles [deg]:");
      for (int i = 0 ; i < 3; ++i) {
        char str[32];
        dtostrf(MyMTi->getEulerAngles()[i], 8, 2, str);
        appendFile(SD, "/hello.txt", str);                                                                    //Print the last read value
        appendFile(SD, "/hello.txt", " ");
        if(i == 2){
          appendFile(SD, "/hello.txt", "\n");
        }
      }
      Serial.println(" ");
    }    
  }
  delay(10);
}
