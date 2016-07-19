/*

      This class assembles
               LiquidCrystal 16x2
	       BME280
	       Avr µC

      to get data and display them

      author : vincent jaunet, vincent.jaunet@hotmail.fr
      license: MIT

 */

#include "Arduino.h"
#include "meteo_sensor.h"

/*------ Constuctors ------*/
//for software SPI
meteo::meteo(uint8_t lcd_addr, uint8_t bme_cs, uint8_t bme_sck,uint8_t bme_mosi,uint8_t bme_miso)
{
  Adafruit_BME280 bme(bme_cs, bme_mosi, bme_miso, bme_sck); // software SPI
  LiquidCrystal_I2C lcd(0x20,16,2);  // LCD address to 0x20; 16 chars-2 lines display
}
//for hardware SPI
meteo::meteo(uint8_t lcd_addr)
{
}

/*------ Initialization of Communication ------*/
bool meteo::initialize()
{
  //LCD on i2c

  //BME 280 on SPI

}
