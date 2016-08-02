# --- Chinese Nano V3.0 --------------

BOARD_TAG      = nano
F_CPU          = 16000000L
MCU            = atmega328p

ARDUINO_LIBS = SPI Wire Adafruit_BME280 LiquidCrystal_I2C LowPower
#	ESP8266_AT_Arduino_Library SoftwareSerial

PORT=/dev/cu.wchusbserial1420
AVRDUDE_ARD_BAUDRATE = 57600
ISP_PROG     = avrisp
AVRDUDE_OPTS = -v

MONITOR_BAUDRATE=9600

#include the Arduino Makefile
include /usr/local/opt/arduino-mk/Arduino.mk


# MONITOR_PORT = /dev/cu.wchusbserial1420
# Plain AVR-C exemple
# NO_CORE = Yes
# BOARD_TAG    = atmega16
# MCU = atmega16
# F_CPU = 8000000L
# ISP_PROG   = stk500v1
