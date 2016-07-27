
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
volatile uint8_t menu=MENU_MAX;
meteo meteo_device;

#define Nsleeps 809

volatile bool interFlag=true;
volatile long lasttimeloop;
uint16_t cur_sleep=0;
void BTNpressed()
{

  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 300ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 300)
    {
      interFlag = true;
      return;
    }
  last_interrupt_time = interrupt_time;
}

void print_menu(){

  switch(menu){
  case 0:
    //Renew the data set in the meteo class
    meteo_device.get_data();
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

  meteo_device.initialize();

}


void loop() {

  //There was a button pressed, process it
  if (interFlag){
    menu++;
    if (menu > MENU_MAX) menu=0;
    print_menu();
    interFlag=false;
  }

  //Wait for 6 sec including the processing
  //to display the data
  unsigned long wait_start=millis();
  while (millis()-wait_start<6000){
    if (interFlag) break;
  }

  //If there was no interrupt --------------------
  //-- turn off and set next display to menu 0
  //-- Go to sleep for the expected nuber of 8sec time
  if (!interFlag) {
    meteo_device.turn_off();
    menu=MENU_MAX;

    //account for loop time the time spent in the main loop
    cur_sleep+= (uint16_t) (millis()-lasttimeloop)/8000;

    for (cur_sleep;cur_sleep<Nsleeps;cur_sleep++){
      LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
      // get out of here when button is pressed
      // and restart loop()
      if (interFlag) {
	//get the current time
	lasttimeloop=millis();
	return;
      }
    }
  }

  // Store data after the correct amount of sleeps not when the BTN is pressed
  // Set cur_sleep to restart the whole sleep proccess
  if (cur_sleep==Nsleeps) {
    meteo_device.store_data();
    cur_sleep=0;
  }


}
