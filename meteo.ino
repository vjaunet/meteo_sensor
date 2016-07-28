
/***************************************************************************

         main meteo sensor

      author : vincent jaunet, vincent [dot] jaunet [at] hotmail [dot] fr
      license: MIT

      Requires :
          -- BME280 + Adafruit BME 280 library
          -- lcd 16x2 display + LcdCrystal Library
          -- LowPower Library to run in sleep mode

 ***************************************************************************/

#include "meteo_sensor.h"
#include "LowPower.h"

#define BTN 2
#define MENU_MAX 2
volatile uint8_t menu=0;
meteo meteo_device;

#define Nsleeps 450 // Get 40 reccords in 40h
//#define Nsleeps 7 // Get a reccord every minute
volatile bool interFlag=true;
void BTNpressed()
{

  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 500ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 500)
    {
      interFlag = true;
      return;
    }
  last_interrupt_time = interrupt_time;
}

void print_menu(){

  switch(menu){
  case 0:
    meteo_device.print_meteo();
    break;

  case 1:
    meteo_device.print_pressure_hist();
    break;

  case 2:
    meteo_device.print_temp_hist();
    break;

  default:
    break;
  }

}

void setup() {

  /*define Button pin and set it to 1*/
  pinMode(BTN,INPUT_PULLUP);
  attachInterrupt(0, BTNpressed, FALLING);

  /* init the devices */
  meteo_device.initialize();

}


void loop() {

  static uint16_t cur_sleep;
  static long lasttimeloop;

  //There was a button pressed, process it
  if (interFlag){
    print_menu();
    menu++;
    if (menu > MENU_MAX) menu=0;
    interFlag=false;
  }

  //Wait for 6 sec including the processing
  //to display the data
  unsigned long wait_start=millis();
  while (millis()-wait_start<6000){
    if (interFlag) return;
  }

  //-- From here, there was no interrupt after the waiting time --

  //-- turn off and set next display to menu 0
  meteo_device.turn_off();
  menu=0;

  //-- account for loop time the time spent in the main loop
  // if too much time was spent, do not sleep
  cur_sleep+= (uint16_t) (millis()-lasttimeloop)/8000;
  if (cur_sleep>Nsleeps) cur_sleep=Nsleeps;

  //-- Go to sleep for the expected nuber of 8sec time
  for (cur_sleep;cur_sleep<Nsleeps;cur_sleep++){
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    // get out of here when button is pressed
    // and restart loop()
    if (interFlag) {
      //get the current time
      lasttimeloop=millis();
      break;
    }
  }

  // Store data after the correct amount of sleeps not when the BTN is pressed
  // Set cur_sleep to restart the whole sleep proccess
  if (cur_sleep==Nsleeps) {
    meteo_device.store_data();
    cur_sleep=0;
  }

}
