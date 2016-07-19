
/***************************************************************************

         main meteo sensor

 ***************************************************************************/

#define BME_SCK 9
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1013.25)

#define ctrl_hum  B00000001 //Hoversampling=1
#define ctrl_tp   B00100110 //Toversampling=x8, Poversampling=x8, forced mode
#define ctrl_conf B10100100  //1 second of standby,No Filter,spi 3wire off

//Adafruit_BME280 bme; // I2C not working.... ???
//Adafruit_BME280 bme(BME_CS); // hardware SPI
Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // Software SPI

LiquidCrystal_I2C lcd(0x20,16,2);  // LCD address to 0x20; 16 chars-2 lines display


#define PATM_MIN 980
#define PATM_MAX 1015
#define nrowGraph 8
#define nCustomChar 8
uint8_t curChar=0;
uint8_t curLine=0;
byte barGraph[nCustomChar][nrowGraph];
void createCustomChar(float Pdata)
{

  //Get the vertical lcd location of this data
  Pdata=(Pdata-PATM_MIN)/(PATM_MAX-PATM_MIN);
  uint8_t P_int= constrain((uint8_t) Pdata,0,7);

  //Build up the corresponding new Char
  for (uint8_t irow=0;irow<(7-P_int);irow++)
    {
      barGraph[nCustomChar-1][irow] <<= 1 ;
      barGraph[nCustomChar-1][irow] &= 0b11110;
    }
  for (uint8_t irow=(7-P_int);irow<nrowGraph;irow++)
    {
      barGraph[nCustomChar-1][irow] <<= 1;
      barGraph[nCustomChar-1][irow] += 0b1;
    }

  // move all points one step down in time
  for (uint8_t iChar=0;iChar<nCustomChar-1;iChar++)
    {
    for (uint8_t irow=0;irow<nrowGraph;irow++)
      {
	barGraph[iChar][irow] <<= 1;
	barGraph[iChar][irow] |= (barGraph[iChar+1][irow] & 0b10000) >> 4;
      }
    }

  //Create custom Char in the lcd memory
  for (uint8_t iChar=0;iChar<nCustomChar;iChar++)
    {
      lcd.createChar(iChar,barGraph[iChar]);
    }
}

#define Npress 360 // For 24 hour reading when acquiring ev 10s
uint8_t iavgP=0;
float avgPressure;
float curPressure;

char str_temp[6];
char str_hum[6];
char str_press[8];

void print_pressure_hist()
{
  lcd.setCursor(0,0);
  lcd.print("24h Press Evol:");
  for (uint8_t iChar=0;iChar<nCustomChar;iChar++){
    lcd.setCursor(4+iChar,1);
    lcd.write(iChar);
  }
}

void print_meteo()
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

#define BTN 2
#define MENU_MAX 1
volatile uint8_t menu=1;
void print_menu(){
  lcd.clear();
  delay(50);
  lcd.backlight();
  switch(menu){
  case 0:
    print_meteo();
    break;

  case 1:
    print_pressure_hist();
    break;

  default:
    break;
  }

}

volatile bool interFlag=true;
volatile long lasttimeloop;
void BTNpressed()
{
  interFlag = true;
  lasttimeloop=-10000;
  return;
}

void setup() {
  // Serial.begin(9600);
  // Serial.println(F("BME280 test"));

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

  /*define Button pin and set it to 1*/
  pinMode(BTN,INPUT_PULLUP);
  attachInterrupt(0, BTNpressed, LOW);

  // Print a message to the LCD in the end of setup to control
  lcd.print("Meteo v0.1");
  delay(2000);

}


void loop() {

  /*Set the bme to forced mode for next value*/
  bme.set_control(ctrl_hum,ctrl_tp);

  /*Read data from bme280 and print*/
  curPressure=bme.readPressure()/100.0F;
  dtostrf( curPressure, 6, 1, str_press);
  dtostrf( bme.readTemperature(), 5, 2, str_temp);
  dtostrf( bme.readHumidity(), 5, 2, str_hum);

  avgPressure+=curPressure;
  iavgP++;
  if (iavgP == Npress) {
    avgPressure /= (float)Npress;
    iavgP=0;
    createCustomChar(avgPressure);
  }

  if (interFlag){
    menu++;
    if (menu > MENU_MAX) menu=0;
    print_menu();
    interFlag=false;
  }

  //for for 10 sec, if no interrupt :
  //turn off and set next display to menu 0
  lasttimeloop=millis();
  while (millis()-lasttimeloop<10000);
  if (!interFlag) {
    menu=MENU_MAX;
    lcd.noBacklight();
  }

}
