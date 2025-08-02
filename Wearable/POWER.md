# Power Usage Information

### Aug 1 2025

#### Current Device List

- ESP32 S3
    - Core 1 idle
    - Core 0, no sleep, 80 MHz
    - ULP core inactive
    - AMS1117 replaced with TLV1117
- TLV1117
    - SOT223 package
    - 3.3v regulator
    - Low Iq
    - LDO
- LP602535
- GPS/IMU
    - CAM-M8
        - 1s/fix
    - LIS3MDL
    - LSM6DSL
    - BME388
- LTR390
    - 16-Bit resolution
    - 25ms measurement rate
- GME128128
    - SH1107


#### TLV1117 3.3V Regulator

Field|Min|Typ|Max|Unit
-----|---|---|---|----
Input Voltage|2||5.5|V
Output Current|||1.1|A
Dropout Voltage 200mA||115||mV
Dropout Voltage 500mA||285||mV
Dropout Voltage 800mA||455||mV
Dropout Voltage 1000mA||570|800|mV
Quiescent Current 0mA||50||μA
Quiescent Current 200mA||150||μA
Quiescent Current 500mA||300||μA
Quiescent Current 800mA||450||μA
Quiescent Current 1000mA||500||μA
Thermal shutdown||165||°C
Thermal reset||145||°C
Operating temperature|-40||125|°C



#### LP602535 500mAh LiPo

Field|Min|Typ|Max|Unit
-----|---|---|---|----
Capacity|500|510||mAh
Energy Capacity||1.85||Wh
Nominal Voltage||3.70||V
Operating Voltage|2.75||4.20|V
Charging Voltage|||4.20±0.050|V
Charging Current|||250|mA
Discharge Current|||500|mA
Discharge Cut-off Voltage||2.75||V
Internal Impedance|||200|mΩ
Cycle Life||500||cycles
Overcharge Detection||4.275||V
Overdischarge Detection||2.75||V
Overcurrent Detection|2||4.5|A
Charge Temperature|0||45|°C
Discharge Temperature|-20||60|°C



#### GPS/IMU

Field|Min|Typ|Max|Unit
-----|---|---|---|----
4.5V on 5V input|39/170|40/170|42/180|mA/mW
3.3V on 5V input|39/130|40/130|44/140|mA/mW
3.3V on 3.3V input|39/130|40/130|42/140|mA/mW 