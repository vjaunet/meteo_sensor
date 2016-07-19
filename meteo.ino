
/***************************************************************************

         main meteo sensor

 ***************************************************************************/

#include "meteo_sensor.h"

#define BTN 2
#define MENU_MAX 1

volatile uint8_t menu=1;
meteo meteo_device;

volatile bool interFlag=true;
volatile long lasttimeloop;
void BTNpressed()
{
  interFlag = true;
  lasttimeloop=-10000;
  return;
}

void print_menu(){
  delay(50);
  switch(menu){
  case 0:
    meteo_device.print_meteo();
    break;

  case 1:
    meteo_device.print_pressure_hist();
    break;

  default:
    break;
  }

}

void setup() {

  /*define Button pin and set it to 1*/
  pinMode(BTN,INPUT_PULLUP);
  attachInterrupt(0, BTNpressed, LOW);

  meteo_device.initialize();

}


void loop() {

  //Renew the data set in the meteo class
  meteo_device.get_data();

  //There was a button pressed, process it
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
  }

}
