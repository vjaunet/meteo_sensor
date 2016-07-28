/*

      This class assembles
               LiquidCrystal 16x2
	       BME280 Adafruit

      to get data and display them

      author : vincent jaunet, vincent [dot] jaunet [at] hotmail [dot] fr
      license: MIT

 */

#include "Arduino.h"
#include "meteo_sensor.h"

#define SEALEVELPRESSURE_HPA (1013.25)

/*------ Constuctors ------*/
meteo::meteo()
: bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK), lcd(LCD_I2C_ADDR,16,2)
{
  // allocate the bargraph table and initialize
  _barGraph=new byte*[nCustomChar];
  for (int i = 0; i < nCustomChar; i++) {
    _barGraph[i]=new byte[nrowGraph];
    for (int j = 0; j < nrowGraph; j++) {
      _barGraph[i][j] &= 0b0;
    }
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

  // initialize pressure and temp tables
  float press= bme.readPressure()/100.0F;
  float temp= bme.readTemperature();
  for (int8_t i=nRec-1;i>-1;--i){
    Press_rec[i] = press;
    Temp_rec[i] = temp;
  }
  delay(300);

  return true;
}

/*-------- Data Acquisition- --------*/
void meteo::get_data()
{
  /*Set the bme to forced mode for next value*/
  bme.set_control(ctrl_hum,ctrl_tp);

  /*Read data from bme280*/
  curPressure=bme.readPressure()/100.0F;
  curTemp = bme.readTemperature()-1.0F;
  curHum = bme.readHumidity();

}

/*-------- Storing Data in the flash mem --------*/
void meteo::store_data()
{
  for (uint8_t i=0;i<nRec-1;i++){
    Press_rec[i]= Press_rec[i+1];
    Temp_rec[i] = Temp_rec[i+1];
  }
  Press_rec[nRec-1]= bme.readPressure()/100.0F;
  Temp_rec[nRec-1] = bme.readTemperature()-1.0F;;
}


/*--------- Display Functions ------------*/
void meteo::print_temp_hist()
{

  createBarGraph(Temp_rec);

  lcd.clear();
  lcd.setCursor(0,0);

  char heading[16];
  char str_temp_min[5];
  char str_temp_max[5];
  dtostrf( minval(Temp_rec), 4, 1, str_temp_min);
  dtostrf( maxval(Temp_rec), 4, 1, str_temp_max);

  sprintf(heading,"T: %s%cC-%s%cC", str_temp_min,(char)223, str_temp_min,(char)223);
  lcd.print(heading);

  for (uint8_t iChar=0;iChar<nCustomChar;iChar++){
    //Create custom Char in the lcd memory and print
    lcd.createChar(iChar,_barGraph[iChar]);
    lcd.setCursor(4+iChar,1);
    lcd.write(iChar);
  }
  lcd.backlight();
}

void meteo::print_pressure_hist()
{

  createBarGraph(Press_rec);

  lcd.clear();
  lcd.setCursor(0,0);

  char heading[16];
  char str_press_min[8];
  char str_press_max[8];
  dtostrf( minval(Press_rec), 6, 1, str_press_min);
  dtostrf( maxval(Press_rec), 6, 1, str_press_max);

  sprintf(heading,"P: %s-%s", str_press_min, str_press_max);
  lcd.print(heading);

  for (uint8_t iChar=0;iChar<nCustomChar;iChar++){
    //Create custom Char in the lcd memory and print
    lcd.createChar(iChar,_barGraph[iChar]);
    lcd.setCursor(4+iChar,1);
    lcd.write(iChar);
  }
  lcd.backlight();

}

void meteo::print_meteo()
{
  //Renew the data set in the meteo class
  get_data();

  char str_temp[6];
  char str_hum[6];
  char str_press[8];
  dtostrf( curPressure, 6, 1, str_press);
  dtostrf( curTemp, 5, 2, str_temp);
  dtostrf( curHum, 5, 2, str_hum);

  char str1[16];
  sprintf(str1,"%sC - %sH", str_temp, str_hum);
  char str2[16];
  sprintf(str2,"   %s hPa", str_press);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(str1);
  lcd.setCursor(0,1);
  lcd.print(str2);
  lcd.backlight();

}

void meteo::turn_off()
{
  lcd.noBacklight();
}


/*-------- Bargraph Creation- --------*/
void meteo::createBarGraph(float *data)
{

  /* loop in the data array and create
   characters based on the values */
  for (int8_t i=0;i<nRec;i++){

    //Get the 8bit equivalent altitiude of this data
    uint8_t data_int=4;
    float minv=minval(data);
    float maxv=maxval(data);
    if (minv != maxv)
      {
    	data_int=
    	  constrain(((data[i]-minv)/(maxv-minv)*7),0,7);
      }


    //Set the lcd column to 1 up to the actual value
    for (uint8_t irow=0;irow<(7-data_int);irow++)
      {
	_barGraph[nCustomChar-1][irow] <<= 1 ;
	_barGraph[nCustomChar-1][irow] &= 0b11110;
      }
    for (uint8_t irow=(7-data_int);irow<nrowGraph;irow++)
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
  }
}


float meteo::maxval(float* array){
  float max=-1e5;
  for (int i=0; i<nRec; i++){
    if (max<array[i]){
      max = array[i];
    }
  }
  return max;
}

float meteo::minval(float* array){
  float min=1e5;
  for (int i=0; i<nRec; i++){
    if (min>array[i]){
      min = array[i];
    }
  }
  return min;
}
