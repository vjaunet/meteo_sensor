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

#define nrowGraph 8
#define nCustomChar 8
#define nRec 40

#define ctrl_hum  B00000001 //Hoversampling=1
#define ctrl_tp   B00100110 //Toversampling=x8, Poversampling=x8, forced mode
#define ctrl_conf B10100100  //1 second of standby,No Filter,spi 3wire off
/*---------------------------------------------------------------------------*/

class meteo
{
 public :
  meteo();
  bool initialize();
  void print_meteo();
  void print_pressure_hist();
  void print_temp_hist();
  void get_data();
  void store_data();
  void turn_off();

 private :
  Adafruit_BME280   bme; // software SPI
  LiquidCrystal_I2C lcd;  // LCD address to 0x20; 16 chars-2 lines display

  float minval(float *);
  float maxval(float *);

  byte** _barGraph;
  float curPressure,curTemp,curHum;
  char str_temp[6];
  char str_hum[6];
  char str_press[8];
  float Press_rec[nRec];
  float Temp_rec[nRec];
  void createBarGraph(float *);

};

#endif
