
/***************************************************************************

         main meteo sensor

      author : vincent jaunet, vincent [dot] jaunet [at] hotmail [dot] fr
      license: MIT

      Requires :
          -- BME280 + Adafruit BME 280 library
          -- lcd 16x2 display + LcdCrystal Library
          -- LowPower Library to run in sleep mode
          -- esp8266 for wifi connectivity

 ***************************************************************************/

#include "meteo_sensor.h"
#include <LowPower.h>
#include <SoftwareSerial.h>
#include <SparkFunESP8266WiFi.h>

//Define the web server and the AccessPoint
#define SSID "ESP8266_01"
#define PASSWORD "1234567890"
const String htmlHeader = "HTTP/1.1 200 OK\r\n"
  "Content-Type: text/html\r\n"
  "Connection: close\r\n\r\n"
  "<!DOCTYPE HTML>\r\n"
  "<html>\r\n";
ESP8266Server server = ESP8266Server(80);
void serve_webpage()
{
  // available() is an ESP8266Server function which will
  // return an ESP8266Client object for printing and reading.
  // available() has one parameter -- a timeout value. This
  // is the number of milliseconds the function waits,
  // checking for a connection.
  ESP8266Client client = server.available(500);

  if (client)
    {
      Serial.println(F("Client Connected!"));
      // an http request ends with a blank line
      boolean currentLineIsBlank = true;
      while (client.connected())
	{
	  if (client.available())
	    {
	      char c = client.read();
	      // if you've gotten to the end of the line (received a newline
	      // character) and the line is blank, the http request has ended,
	      // so you can send a reply
	      if (c == '\n' && currentLineIsBlank)
		{
		  Serial.println(F("Sending HTML page"));
		  // send a standard http response header:
		  client.print(htmlHeader);
		  String htmlBody;
		  // output the value of each analog input pin
		  for (int a = 0; a < 6; a++)
		    {
		      htmlBody += "A";
		      htmlBody += String(a);
		      htmlBody += ": ";
		      htmlBody += String(analogRead(a));
		      htmlBody += "<br>\n";
		    }
		  htmlBody += "</html>\n";
		  client.print(htmlBody);
		  break;
		}
	      if (c == '\n')
		{
		  // you're starting a new line
		  currentLineIsBlank = true;
		}
	      else if (c != '\r')
		{
		  // you've gotten a character on the current line
		  currentLineIsBlank = false;
		}
	    }
	}
      // give the web browser time to receive the data
      delay(1);

      // close the connection:
      client.stop();
      Serial.println(F("Client disconnected"));
    }

}

#define BTN 2
#define MENU_MAX 2
uint8_t menu=0;
meteo meteo_device;

#define Nsleeps 450 // Get 40 reccords in 40h
//#define Nsleeps 7 // Get a reccord every minute
volatile bool interFlag=false;
void BTNpressed()
{

  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200)
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

  Serial.begin(9600);

  /*define Button pin and set it to 1*/
  pinMode(BTN,INPUT_PULLUP);
  attachInterrupt(0, BTNpressed, FALLING);

  /* init the devices */
  meteo_device.initialize();

  /* init the esp8266 */
  // esp8266.begin() verifies that the ESP8266 is operational
  // and sets it up for the rest of the sketch.
  // It returns either true or false -- indicating whether
  // communication was successul or not.
  if ( !esp8266.begin() )
    {
      Serial.println(F("Error talking to ESP8266."));
    }

  //set the esp8266 to AP mode
  //  The ESP8266 can be set to one of three modes:
  //  1 - ESP8266_MODE_STA - Station only
  //  2 - ESP8266_MODE_AP - Access point only
  //  3 - ESP8266_MODE_STAAP - Station/AP combo
  // Use esp8266.getMode() to check which mode it's in:
  if (esp8266.setMode(ESP8266_MODE_AP) < 0)
    {
      Serial.println(F("Error setting mode."));
    }

  //Beware that one may need to specify IP address range and mask
   //before attempting to get an adress from a dchp server...

  if (esp8266.setIPrange("192.168.5.1","192.168.5.1","255.255.255.0") <= 0)
    {
      Serial.println(F("Failed setting IP range"));
    }

  //set AccessPoint name and password
  if ( !esp8266.setAP(SSID,PASSWORD) )
    {
      Serial.println(F("Error setting AP"));
    }

  //enable esp8266 DHCP
  if ( !esp8266.enableDHCP() )
    {
      Serial.println(F("Error enabling DHCP"));
    }

  // begin initializes a ESP8266Server object. It will
  // start a server on the port specified in the object's
  // constructor (in global area)
  server.begin();
  Serial.print(F("Server started! Go to "));
  Serial.println(esp8266.localIP());
  Serial.println();

}


void loop() {

  static uint16_t cur_sleep=0;
  static uint32_t lasttimeloop=0;

  //There was a button pressed, process it
  if (interFlag){
    print_menu();
    menu++;
    if (menu > MENU_MAX) menu=0;
    interFlag=false;
  }

  Serial.print("here");

  //serve the webpage
  serve_webpage();


  //Wait for 6 sec including the processing
  //to display the data on the lcd
  uint32_t wait_start=millis();
  while (millis()-wait_start<6000){
    // A return in the loop() function restarts to the begining of loop()
    // indeed, the call to loop() is within a endless for ;; loop in the main
    if (interFlag) return;
  }
  Serial.print("here");
  //-- From here, there was no interrupt after the waiting time --

  //-- turn off and set next display to menu 0
  meteo_device.turn_off();
  menu=0;

  //-- account for loop time the time spent in the main loop
  // if too much time was spent, stroe data
  if (lasttimeloop != 0) cur_sleep+= (uint32_t) (millis()-lasttimeloop)/8000;
  if (cur_sleep>Nsleeps) cur_sleep=Nsleeps;

  // //-- Go to sleep for the expected nuber of 8sec time
  // for (uint16_t isleep=cur_sleep;isleep<Nsleeps;isleep++){
  //   LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  //   // get out of here when button is pressed
  //   // and restart loop()
  //   if (interFlag) {
  //     //get the current time
  //     lasttimeloop=millis();
  //     return;
  //   }
  // }

  // Some time has passed :
  // Store data after the correct amount of sleeps
  // Set cur_sleep=0 to restart the whole sleep proccess
  meteo_device.store_data();
  cur_sleep=0;

}
