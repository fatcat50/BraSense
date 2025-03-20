# ESP32 Measurement System

## Overview
The **ESP32 Measurement System** is a project designed to collect, store, and transmit measurement data using various sensors. The system uses **WiFi**, **SD Card**, and **WebSocket** to provide real-time measurement monitoring and logging.

## Features
- **WiFi Connectivity**: Connects to a WiFi network for data transmission.
- **Web Interface**: Supports real-time communication using WebSockets.
- **SD Card Storage**: Data is logged and stored on an SD card for later retrieval.
- **Measurement Logging**: Collects measurement data and logs it continuously.
- **Task Scheduling**: Uses task management to run various operations concurrently.

## Hardware Requirements
- **ESP32 Development Board**
- **MTi Sensor** (or similar measurement sensor)
- **SD Card Module**
- **Power Source (USB or Battery)**

## Software Requirements
- **Arduino IDE** or **PlatformIO** for compiling and uploading the code.
- **AsyncTCP Library** for asynchronous communication.
- **ESPAsyncWebServer Library** for handling the web server and WebSockets.
- **SD Library** for reading/writing data to the SD card.

## Installation & Setup

1. **WiFi Configuration**  
   Update the `initWiFi()` function in the code with your **WiFi credentials**.

2. **SD Card Setup**  
   Ensure the SD card is connected to the ESP32 correctly, and the code initializes the SD card for data logging.

3. **MTi Sensor Setup**  
   Set up the MTi sensor and ensure the sensor readings are properly processed by the `MyMTi->readMessages()` function.

4. **Upload the Code**  
   Use the **Arduino IDE** or **PlatformIO** to upload the code to the ESP32.

5. **Web Interface**  
   Access the ESP32’s web interface for real-time data display via WebSocket and a built-in HTML interface.

## Usage
- The ESP32 connects to WiFi on startup, begins logging measurements, and stores data on the SD card.
- Measurement data is processed when the MTi sensor provides new data.
- The web interface can be accessed by navigating to the ESP32’s IP address in a browser to monitor data in real time.
