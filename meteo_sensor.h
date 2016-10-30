/*

      This class assembles
               LiquidCrystal 16x2
	       BME280 Adafruit

      to get data and display them

      author : vincent jaunet, vincent [dot] jaunet [at] hotmail [dot] fr
      license: MIT

 */

#ifndef _METEO_SENSOR_
#define _METEO_SENSOR_

#include "Arduino.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BME280.h>
#include <LiquidCrystal_I2C.h>
#include <stdlib.h>

/*--------------------------------------------------------------------------*/
/* parameters to be set */
#define BME_SCK 9
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10
#define LCD_I2C_ADDR 0x20

#define BATT_PIN A4

#define nrowGraph 8
#define nCustomChar 8
#define nRec 40

#define ctrl_humos          B00000001 //Hoversampling=1
#define ctrl_tos_pos_mode   B00100110 //Toversampling=x1, Poversampling=x1, forced mode
#define ctrl_conf           B10100100 //1 second of standby,No Filter,spi 3wire off
/*---------------------------------------------------------------------------*/

class meteo
{
 public :
  meteo();
  bool initialize();
  void print_meteo();
  void print_pressure_hist();
  void print_temp_hist();
  void store_data();
  void turn_off();
  void check_battery();

  float curPressure,curTemp,curHum;

 private :
  Adafruit_BME280   bme;  // software SPI
  LiquidCrystal_I2C lcd;  // LCD address to 0x20; 16 chars-2 lines display

  void get_data();
  float minval(float *);
  float maxval(float *);

  byte** _barGraph;
  float Press_rec[nRec];
  float Temp_rec[nRec];
  void createBarGraph(float *);

};

#endif
