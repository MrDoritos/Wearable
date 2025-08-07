# Wearable

Open source activity and environment tracking and logging device. Works fully offline.

## Features

- GPS location and time synchronization
- Humidity, temperature, and pressure monitoring
- Monitor the level of various gases, such as CO, CO2, VOCs, NH3, NO2, and other hydrocarbons
- UV exposure tracking
- 128x128 OLED display, for high contrast and low power draw
- Piezo and haptic user feedback
- Accelerometer, gyroscope, magnetometer information
- Logging to SD card
- Screen timeout and input lock
- Automatic screen brightness from the ambient light sensor
- Heart rate, blood oxygen (SpO2), and breathing rate
- Minimal loading time

## User interface

- GPS map (only plotted, no map)
- Travel distance and pedometer (step counter)
    - Also available in the stopwatch, which offers splits and history
- Log view
    - Short term, about 10s of information
    - Long term, any amount of information at any specified interval
- Delay free, low latency interface
- Full screen clock interface
- Battery level and time and date visible
- Dynamic interface layout, the size of elements can change

## What's missing

A lot, only a month of programming really went into it. It wasn't sloppy though, and it is quite easy to add and remove features. Some power saving features are missing entirely.

## How much

$200 in parts, but sewing and 3D printing will take some time.

## Parts

ESP32 S3
TLV1117 3.3V LDO SOT-223 regulator
JST 500 or 1200 mAh battery
GME128128 I2C SH1107 1.5" Monochrome OLED
BME688
BerryGPS-IMU v4
10mm Piezo, 10mm Vibration motor
5x 4.25mm SMD buttons
30 AWG silicone wire
34 AWG enameled wire
Stretchy fabric, stretchy thread, non-strechy thread
MAX30102
LTR390-UV

Resin printed button caps.
3D printed fabric frames and button holder.

It is best to remove unnecessary connectors.

## In the future

Not waterproof. Most sensors can move to a single board. MAX30102 can be minified. Buttons could be silicone. No frame for the LTR390-UV prevents consistent light measurements. No ammeter to track battery usage.

## Why

Internship project

## Credits

DFRobot, MAX30102, BMP388
RocketManRC, u-blox-m8
Adafruit, LTR390, LSM6DSL, LIS3MDL, inspiration for display driver
Mozilla, CSS documentation
Espressif, ESP IDF, HAL for ESP32 S3
