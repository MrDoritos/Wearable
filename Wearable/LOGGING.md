# Logging

### Polling Rates

- Each module should synchronize their fields?
- How expensive are SD card flushes?
- Long term data can use an additional buffer, these are for immediate information.
- Screen flush always blocks 36ms, all UI operations block for ~50ms total, but can be split up.
- Sample 2x the frequency of the desired field required to be processed
- I planned on having the polling rate be configured in the UI, without element focus it can't be done
- Time stretching with display on is okay, since with display off it's not an issue

Module|Log Field|Polling Rate|Buffer Size
------|---------|------------|-----------
Battery|Voltage|1000ms (1Hz)|10
MiCS-6814|CO Voltage|1000ms (1Hz)|10
-|NH3 Voltage|-|10
-|NO2 Voltage|-|10
CAM-M8|Latitude|1000ms (1Hz)|100
-|Longitude|-|100
-|Altitude|-|10
-|Bearing|-|10
-|PDOP|-|10
-|Ground Speed|-|10
-|Horizontal Accuracy|-|10
-|Vertical Accuracy|-|10
-|Time Accuracy|-|10
-|Odometer|-|10
IMU|Accelerometer X|10ms (100Hz)|1000
-|Accelerometer Y|-|-
-|Accelerometer Z|-|-
-|Gyroscope X|-|-
-|Gyroscope Y|-|-
-|Gyroscope Z|-|-
BME688|Humidity|1000ms (1Hz)|10
-|Temperature|-|-
-|Pressure|-|-
-|VOC|-|-
-|AQI|-|-
-|CO2|-|-
-|Gas Scan H2S|30000ms|-
-|Gas Scan EtOH|30000ms|-
-|Gas Scan CO|30000ms|-
BMP388|Pressure|1000ms (1Hz)|10
LTR390|ALS|1000ms (1Hz)|10
-|Lux|-|-
-|UVS|-|-
-|UVI|-|-
MAX30102|IR value|50ms|200
-|Red value|50ms|200
-|Heart Rate|1000ms (1Hz)|10
-|SpO2|1000ms (1Hz)|10