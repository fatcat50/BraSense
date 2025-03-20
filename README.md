# ESP32 Measurement System

## Overview
This project uses an **ESP32** to collect data from an **MTi sensor**, logs it to an SD card, and sends it via WebSocket. The system is controlled by a button to start and stop the measurements.

## Features
- Button-controlled start/stop of measurements.
- Data is logged to SD card and transmitted via WebSocket.
- Collects Euler angles (X, Y, Z) from the MTi sensor.

## Requirements
- ESP32 board
- MTi sensor (I2C)
- SD card module
- Button (GPIO 2)

## How to Use
1. Upload the code to your ESP32.
2. Press the button to start/stop measurements.
3. Data will be logged to the SD card and sent via WebSocket.
