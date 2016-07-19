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

#define SEALEVELPRESSURE_HPA (1013.25)

/*------ Constuctors ------*/
meteo::meteo()
: bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK), lcd(LCD_I2C_ADDR,16,2)
{
  _barGraph=new byte*[nCustomChar];
  for (int i = 0; i < nCustomChar; i++) {
    _barGraph[i]=new byte[nrowGraph];
  }
}

/*------ Initialization of Communication ------*/
bool meteo::initialize()
{

  lcd.init(); // initialize the lcd
  lcd.clear();

  // Light up the LCD.
  lcd.backlight();

  if (!bme.begin()) {
    lcd.clear();
    lcd.println("BME280: check wiring");
    while (1);
  }

  /* Set Stbby time and iir filter*/
  bme.set_config(ctrl_conf);

  // Print a message to the LCD in the end of setup to control
  lcd.print("Meteo v0.1");
  delay(2000);

  return true;
}


/*-------- Data Acquisition- --------*/
void meteo::get_data()
{
  /*Set the bme to forced mode for next value*/
  bme.set_control(ctrl_hum,ctrl_tp);

  /*Read data from bme280 and print*/
  curPressure=bme.readPressure()/100.0F;
  dtostrf( curPressure, 6, 1, str_press);
  dtostrf( bme.readTemperature(), 5, 2, str_temp);
  dtostrf( bme.readHumidity(), 5, 2, str_hum);

}

/*-------- Bargraph Creation- --------*/
void meteo::createCustomChar(float Pdata)
{

  //Get the vertical lcd location of this data
  Pdata=(Pdata-PATM_MIN)/(PATM_MAX-PATM_MIN);
  uint8_t P_int= constrain((uint8_t) Pdata,0,7);

  //Build up the corresponding new Char
  for (uint8_t irow=0;irow<(7-P_int);irow++)
    {
      _barGraph[nCustomChar-1][irow] <<= 1 ;
      _barGraph[nCustomChar-1][irow] &= 0b11110;
    }
  for (uint8_t irow=(7-P_int);irow<nrowGraph;irow++)
    {
      _barGraph[nCustomChar-1][irow] <<= 1;
      _barGraph[nCustomChar-1][irow] += 0b1;
    }

  // move all points one step down in time
  for (uint8_t iChar=0;iChar<nCustomChar-1;iChar++)
    {
    for (uint8_t irow=0;irow<nrowGraph;irow++)
      {
	_barGraph[iChar][irow] <<= 1;
	_barGraph[iChar][irow] |= (_barGraph[iChar+1][irow] & 0b10000) >> 4;
      }
    }

  //Create custom Char in the lcd memory
  for (uint8_t iChar=0;iChar<nCustomChar;iChar++)
    {
      lcd.createChar(iChar,_barGraph[iChar]);
    }

}

void meteo::print_pressure_hist()
{
  lcd.setCursor(0,0);
  lcd.print("24h Press Evol:");
  for (uint8_t iChar=0;iChar<nCustomChar;iChar++){
    lcd.setCursor(4+iChar,1);
    lcd.write(iChar);
  }
}

void meteo::print_meteo()
{
  char str1[16];
  sprintf(str1,"%sC - %sH", str_temp, str_hum);
  char str2[16];
  sprintf(str2,"   %s hPa", str_press);
  lcd.setCursor(0,0);
  lcd.print(str1);
  lcd.setCursor(0,1);
  lcd.print(str2);
}
