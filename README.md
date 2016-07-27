# meteo_sensor
Requires :
          -- BME280 + Adafruit BME 280 library
          -- lcd 16x2 display + LcdCrystal Library
          -- LowPower Library to run in sleep mode

- Get data from BME280 via SPI and display Pressure, Temperature and Humidity
- Store Pressure and Temperature (40 smaples over 72h) in the flash memory and display them on demand on a bargraph.
- A button premits to switch between the displays of histories and current values. This is done via an ISR.

- BME280 is set on "forced mode", thus sleeps in between acquisitions
- The µC is put on sleep_power_down for in between uses