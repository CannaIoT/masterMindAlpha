// "CannaIoT - masterMindAlpha" - is an Arduino driven,
// grow control system for farming and coding enthusiasts.
// Although in early stage the project will be developed for Arduino Uno
// (with wi-fi connection through ESP8266 module) in Arduino IDE
// (using C++ similar language) the goal is to write as much code as possible
// in JavaScript for easy IoT communication, control, presenting values online
// and most important - to make the code universal for other IoT devices.
//
// (please mind that the code is still under development)
//
// "CannaIoT - growing for GEEKS, coding for FARMERS"
//
//
// Please read Software License Agreements of attached libraries.

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <TFT_ILI9163C.h>
#include "DHT.h"
#include <virtuabotixRTC.h>

//RTC connection: CLK -> 6, DAT -> 7, RST -> 5
virtuabotixRTC myRTC(6, 7, 5);

DHT dht;

// Color definitions
#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
//#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
//#define WHITE   0xFFFF

#define __CS 10
#define __DC 9


TFT_ILI9163C tft = TFT_ILI9163C(__CS, 8, __DC);


//For navigating menus
uint8_t mainMenuSelection = 1;
bool settingsMenu = 0;
bool settingsDayNightMenu = 0;

bool settingsMenuDay = 1;
bool settingsMenuNight = 0;
uint8_t setValues = 1;

//For entering / exiting with a button state change
int buttonPin = 12;
bool lastButtonState = 0;

//Joystick INPUT with button
#define joystickX  A4
#define joystickY A5

//Setting general info:
int startDate = 1;
int dayOfGrow = 1;
unsigned long startDateReset;

//Setting parameters for "Day"
uint8_t dayStartTimeHours = 5;
uint8_t dayStartTimeMinutes = 0;
float minDayTemp = 20;
float maxDayTemp = 30;
float minDayHumid = 50;
float maxDayHumid = 70;

//Setting parameters for "Night"
uint8_t nightStartTimeHours = 23;
uint8_t nightStartTimeMinutes = 0;
float minNightTemp = 15;
float maxNightTemp = 25;
float minNightHumid = 40;
float maxNightHumid = 70;

//For Day and Night in the programm
bool dayIndicator = 0;
float minTemp;
float maxTemp;
float minHumid;
float maxHumid;

bool module1State = HIGH; //Lamp
bool module2State = HIGH; //Main Ventilator
bool module3State = HIGH; //Mixing Ventilator
bool module4State = HIGH; //Humidifier


//Global variables set after first read of temperature and humidity from DHT11
float tempRead;
float humidRead;

unsigned long previousMillis = 0;



void setup() {
  pinMode(12, INPUT_PULLUP);
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);

  Serial.begin(9600);
  tft.begin();
  dht.setup(4); // data pin 4 for DHT11 sensor.

  // Lines below are to set the Real Time Clock for the first time.
  // You should put the right date when uploading the program for the first time to set RTC.
  // Afterwards the RTC is set, counts the time and there is no need for setting it again.

  // In brackets are: seconds, minutes, hours, day of the week, day of the month, month, year
  //myRTC.setDS1302Time(15, 8, 20, 5, 11, 12, 2020);

}

void loop() {

  //////////////////////////////////////////////////////////////////////////////
  ///////////////////////////// MAIN PROGRAM ///////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  // General info - program checks if it is night or day, sets min/max values //
  // (parameters) accordingly and sets ON or OFF each of 4 relay modules. //////
  //////////////////////////////////////////////////////////////////////////////

  myRTC.updateTime();

  // Recognizing Day/Night Time.
  if  ((nightStartTimeHours * 60 + nightStartTimeMinutes) > (dayStartTimeHours * 60 + dayStartTimeMinutes)) {
    if (((myRTC.hours * 60 + myRTC.minutes) >= (dayStartTimeHours * 60 + dayStartTimeMinutes))
        && ((myRTC.hours * 60 + myRTC.minutes) < (nightStartTimeHours * 60 + nightStartTimeMinutes))) {
      dayIndicator = 1;
    } else {
      dayIndicator = 0;
    }
  } else if ((nightStartTimeHours * 60 + nightStartTimeMinutes) < (dayStartTimeHours * 60 + dayStartTimeMinutes)) {

    if (((myRTC.hours * 60 + myRTC.minutes) < (dayStartTimeHours * 60 + dayStartTimeMinutes))
        && ((myRTC.hours * 60 + myRTC.minutes) >= (nightStartTimeHours * 60 + nightStartTimeMinutes))) {
      dayIndicator = 0;
    } else {
      dayIndicator = 1;
    }
  } else if ((nightStartTimeHours * 60 + nightStartTimeMinutes) == (dayStartTimeHours * 60 + dayStartTimeMinutes)) {
    dayIndicator = 1;
  }

  //Setting parameters according to day or night and setting ON/OFF relay modules

  if (dayIndicator == 1) {
    minTemp = minDayTemp;
    maxTemp = maxDayTemp;
    minHumid = minDayHumid;
    maxHumid = maxDayHumid;

    //For Realay Module state "LOW" is "ON".
    module1State = LOW;
    module2State = LOW;
    module3State = LOW;
    //module4State = LOW;

    //Humidfier is "ON" (state "LOW") only if humidity is too low.
    if (humidRead <= minHumid) {
      module4State = LOW;
    } else {
      module4State = HIGH;
    }
  }

  if (dayIndicator == 0) {
    minTemp = minNightTemp;
    maxTemp = maxNightTemp;
    minHumid = minNightHumid;
    maxHumid = maxNightHumid;

    module1State = HIGH;
    module2State = HIGH;
    module3State = HIGH;
    //module4State = HIGH;

    //Humidfier is "ON" (state "LOW") only if humidity is too low.
    if (humidRead <= minHumid) {
      module4State = LOW;
    } else {
      module4State = HIGH;
    }


  }

  digitalWrite(A0, module1State);
  digitalWrite(A1, module2State);
  digitalWrite(A2, module3State);
  digitalWrite(A3, module4State);


  //////////////////////////////////////////////////////////////////////////////
  ///////////////////////////  S C R E E N S    ////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  //(Navigating through screens and changing values - "mechanics" of programm)//
  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////


  //================= (Main menu) =================//

  //Screen 1 - Main Screen

  if ((mainMenuSelection == 1) && (settingsMenu == 0)) {
    mainScreen();   //Displaying screen function
    screenSelect(); //Navigating trough menus
  }

  //Screen 2 - Settings Range (parameters range) Screen

  if ((mainMenuSelection == 2) && (settingsMenu == 0)) {

    settingsScreen(); //Displaying screen function
    screenSelect(); //Navigating trough menus
    settingsMenu = buttonEnter(settingsMenu); //Entering Day and Night Settings submenu
  }

  //Screen 3 - CannaIoT Info Screen

  if ((mainMenuSelection == 3) && (settingsMenu == 0)) {

    cannaIoTInfo(); //Displaying screen function
    screenSelect(); //Navigating trough menus
  }


  //=== Settings submenu - Day or Night Settings selection ===//


  //Screen 4 - Settings submenu - Day / Night Settings selection

  if ((mainMenuSelection == 2) && (settingsMenu == 1) && (settingsDayNightMenu == 0)) {

    settingsDayNightScreen(); //Displaying screen function

    //Day or Night settings selection:
    int xAxis = analogRead(joystickX);
    if (xAxis < 200) {
      settingsMenuDay = 0;
      settingsMenuNight = 1;
      delay(50);
    }
    if (xAxis > 900) {
      settingsMenuDay = 1;
      settingsMenuNight = 0;
      delay(50);
    }

    settingsDayNightMenu = buttonEnter(settingsDayNightMenu); //Entering Day or Night Settings:

  }

  //=== Day Settings submenu - Day settings selection and changing values ===//

  //Screen 5 - Day Settings

  if ((mainMenuSelection == 2) && (settingsMenu == 1) && (settingsDayNightMenu == 1) && (settingsMenuDay == 1)) {

    daySettingsScreen(); //Displaying screen function

    setValues = analogYChange(setValues);
    if (setValues >= 8) {
      setValues = 8;
    }
    if (setValues <= 1) {
      setValues = 1;
    }


    //Settings for each parameter.
    //Please note that there are limits for each parameter.
    //Temperature for cannabis plants should be between 14 - 33 Celcious degrees.
    //Humidity should be between 30 - 70%.
    //If you disagree - please change.

    switch (setValues) {
      case 1: {
          //DayStartTimeHours:
          dayStartTimeHours = analogXChange(dayStartTimeHours);
          if (dayStartTimeHours > 23) {
            dayStartTimeHours = 0;
          }
          if (dayStartTimeHours < 0) {
            dayStartTimeHours = 23;
          }
        }
        break;
      case 2: {
          //dayStartTimeMinutes:
          dayStartTimeMinutes = analogXChange(dayStartTimeMinutes);
          if (dayStartTimeMinutes > 59) {
            dayStartTimeMinutes = 0;
          }
          if (dayStartTimeMinutes < 0) {
            dayStartTimeMinutes = 59;
          }
        }
        break;
      case 3: {
          //minDayTemp:
          minDayTemp = analogXChange(minDayTemp);
          if (minDayTemp > maxDayTemp) {
            minDayTemp = maxDayTemp;
          }
          if (minDayTemp < 14) {
            minDayTemp = 14;
          }
        }
        break;
      case 4: {
          //maxDayTemp:
          maxDayTemp = analogXChange(maxDayTemp);
          if (maxDayTemp > 33) {
            maxDayTemp = 33;
          }

          if (maxDayTemp < minDayTemp) {
            maxDayTemp = minDayTemp;
          }

        }
        break;
      case 5: {
          //minDayHumid:
          minDayHumid = analogXChange(minDayHumid);
          if (minDayHumid > maxDayHumid) {
            minDayHumid = maxDayHumid;
          }
          if (minDayHumid < 30) {
            minDayHumid = 30;
          }
        }
        break;
      case 6: {
          //maxDayHumid:
          maxDayHumid = analogXChange(maxDayHumid);
          if (maxDayHumid > 70) {
            maxDayHumid = 70;
          }
          if (maxDayHumid < minDayHumid) {
            maxDayHumid = minDayHumid;
          }
        }
        break;
      case 7: {
          //Day of the grow
          startDate = analogXChange(startDate);
          if (startDate < 0) {
            startDate = 0;
          }
        }
        break;
      case 8: {
          buttonExit();
        }
    }
  }
  //=== Night Settings submenu - Night settings selection and changing values ===//

  //Screen 6 - Night Settings

  if ((mainMenuSelection == 2) && (settingsMenu == 1) && (settingsDayNightMenu == 1) && (settingsMenuNight == 1)) {


    nightSettingsScreen(); //Displaying screen function


    //Selecting Value to change (accessing by moving joystick up and down)

    setValues = analogYChange(setValues);
    if (setValues >= 7) {
      setValues = 7;
    }
    if (setValues <= 1) {
      setValues = 1;
    }
    //Settings for each parameter.
    //Please note that there are limits for each parameter.
    //Temperature for cannabis plants should be between 14 - 33 celcious degrees.
    //Humidity should be between 30 - 70 %.
    //If you disagree - please change.

    switch (setValues) {
      case 1: {
          //NightStartTimeHours:
          nightStartTimeHours = analogXChange(nightStartTimeHours);
          if (nightStartTimeHours >= 24) {
            nightStartTimeHours = 00;
          }
          if (nightStartTimeHours < 0) {
            nightStartTimeHours = 23;
          }
        }
        break;
      case 2: {
          //nightStartTimeMinutes:
          nightStartTimeMinutes = analogXChange(nightStartTimeMinutes);
          if (nightStartTimeMinutes > 59) {
            nightStartTimeMinutes = 0;
          }
          if (nightStartTimeMinutes < 0) {
            nightStartTimeMinutes = 59;
          }

        }
        break;
      case 3: {
          //minNightTemp:
          minNightTemp = analogXChange(minNightTemp);
          if (minNightTemp > maxNightTemp) {
            minNightTemp = maxNightTemp;
          }
          if (minNightTemp < 14) {
            minNightTemp = 14;
          }
        }
        break;
      case 4: {
          //maxNightTemp:
          maxNightTemp = analogXChange(maxNightTemp);
          if (maxNightTemp > 33) {
            maxNightTemp = 33;
          }
          if (maxNightTemp < minNightTemp) {
            maxNightTemp = minNightTemp;
          }
        }
        break;
      case 5: {
          //minNightHumid:
          minNightHumid = analogXChange(minNightHumid);
          if (minNightHumid > maxNightHumid) {
            minNightHumid = maxNightHumid;
          }
          if (minNightHumid < 30) {
            minNightHumid = 30;
          }
        }
        break;
      case 6: {
          //maxNightHumid:
          maxNightHumid = analogXChange(maxNightHumid);
          if (maxNightHumid > 70) {
            maxNightHumid = 70;
          }
          if (maxNightHumid < minNightHumid) {
            maxNightHumid = minNightHumid;
          }
        }
        break;
      case 7: {
          buttonExit();
        }
    }
  }


  // Serial Communiacation via RX and TX pins on Arduino.
  // This is to emulate serial communication with any device connected to RX and TX pin.
  // There are some basic comends implemented.
  // If you connect the device to USB port and choose Serial Monitor you will be able to
  // interact with it and change set each parameter individualy.
  // This will be later used with communicating with ESP8266 device.

  // To check if the connection via serial port has been established type "hello"
  // To check values curretly set on the device type "values".
  // To set each parameter type "set+parameter you want to change+[space]+value".
  // (Please keep "camelCase")
  // For example:
  // "setMinDayTemp 25"
  // "setDayTimeStartHours 10"
  // "setMinNightHumid 45"

  serialCommunication();

}

//////////////////////////////////////////////////////////////////////////////
///////////////////////////  S C R E E N S    ////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//(Displaying values on  screens  - "graphics" /(User Interface of programm)//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//================ MAIN SCREEN ================//

unsigned long mainScreen() {
  unsigned long start = micros();

  //For 2 seconds sampling interval for DHT11:
  unsigned long currentMillis = millis();
  unsigned long samplingTime = 2000;

  //Displaying time from RTC

  //Displaying hours
  tft.setCursor(1, 4);
  tft.setTextSize(2);
  tft.setTextColor(YELLOW, BLACK);

  if (myRTC.hours < 10) {
    tft.print("0");
    tft.print(myRTC.hours);
  } else
  {
    tft.print(myRTC.hours);
  }

  tft.print(":");

  //Displaying minutes
  if (myRTC.minutes < 10) {
    tft.print("0");
    tft.print(myRTC.minutes);
  } else
  {
    tft.print(myRTC.minutes);
  }

  // Displaying day, month and year (month is changed from number to shorted word for each month)
  tft.setCursor(65, 4);
  tft.setTextColor(YELLOW, BLACK);
  tft.setTextSize(1);
  if (myRTC.dayofmonth < 10) {
    tft.setCursor(68, 4);
  }
  tft.print(myRTC.dayofmonth);
  tft.setCursor(79, 4);
  monthToWord(); //function changes month from number to shorted word for each month

  tft.setCursor(99, 4);
  tft.print(myRTC.year);

  tft.setCursor(93, 25);
  //Displaying if it is DAY or NIGHT for plants)
  if (dayIndicator == 1) {
    tft.setTextColor(YELLOW, BLACK);
    tft.print(" DAY ");
  } else
  {
    tft.setTextColor(BLUE, BLACK);
    tft.print("NIGHT");
  }

  //Displaying temerature
  tft.setCursor(0, 35);
  tft.setTextColor(YELLOW, BLACK);
  tft.print("Temp:");
  tft.setCursor(00, 25);
  tft.print("Now is:");

  //Display value from temperature sensor
  if (currentMillis - previousMillis >= samplingTime) {
    float humidity = dht.getHumidity();
    float temperature = dht.getTemperature();

    tft.setCursor(36, 35);
    tft.setTextColor(YELLOW, BLACK);
    tft.print(temperature);
    tempRead = temperature;

    //print "degree" character (moved up 3 pixels)
    tft.setCursor(68, 32);
    tft.print((char)248);
    //print "C" for Celcious
    tft.setCursor(72, 35);
    tft.print("C");

    //check if temperature is between right values
    tft.setCursor(90, 35);
    if (temperature >= minTemp && temperature <= maxTemp) {
      tft.setTextColor(GREEN, BLACK);
      tft.println("  OK  ");
    }
    else {
      tft.setTextColor(RED, BLACK);
      tft.println("NOT OK");
    }

    //Displaying humidity
    tft.setCursor(0, 45);
    tft.setTextColor(YELLOW, BLACK);
    tft.print("Humid:");

    //Displaying humidity valaue from humidity sensor
    tft.setCursor(36, 45);
    tft.print(humidity);
    humidRead = humidity;
    tft.setCursor(71, 45);
    tft.print("%");
    tft.setCursor(90, 45);

    //check if humidity is between right values
    if (humidity >= minHumid && humidity <= maxHumid) {
      tft.setTextColor(GREEN, BLACK);
      tft.println("  OK  ");
    }
    else {
      tft.setTextColor(RED, BLACK);
      tft.println("NOT OK");
    }
    previousMillis = currentMillis;
  }

  //Displaying relay modules status

  //Relay module 1 state (ie. LAMP)
  tft.setCursor(0, 57);
  tft.setTextColor(YELLOW, BLACK);
  tft.println("LAMP:");
  tft.setCursor(102, 57);
  if (module1State == LOW) {
    tft.setTextColor(GREEN, BLACK);
    tft.print("ON ");
  } else {
    tft.setTextColor(RED, BLACK);
    tft.print("OFF");
  }

  //Relay module 2 state (ie. Main ventilator)
  tft.setCursor(0, 67);
  tft.setTextColor(YELLOW, BLACK);
  tft.println("Main vent.:");
  tft.setCursor(102, 67);
  if (module2State == LOW) {
    tft.setTextColor(GREEN, BLACK);
    tft.print("ON ");
  } else {
    tft.setTextColor(RED, BLACK);
    tft.print("OFF");
  }

  //Relay module 3 state (ie. Mixing ventilators)
  tft.setCursor(0, 77);
  tft.setTextColor(YELLOW, BLACK);
  tft.println("Mixing vent.:");
  tft.setCursor(102, 77);
  if (module3State == LOW) {
    tft.setTextColor(GREEN, BLACK);
    tft.print("ON ");
  } else {
    tft.setTextColor(RED, BLACK);
    tft.print("OFF");
  }

  //Relay module 4 state (ie. Humidifier)
  tft.setCursor(0, 87);
  tft.setTextColor(YELLOW, BLACK);
  tft.println("Humidifier:");
  tft.setCursor(102, 87);
  if (module4State == LOW) {
    tft.setTextColor(GREEN, BLACK);
    tft.print("ON ");
  } else {
    tft.setTextColor(RED, BLACK);
    tft.print("OFF");
  }

  ////////////////////////////////// COUNTING DAYS OF GROW ////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////
  // (This can be set in "Day Settings".) /////////////////////////////////////////////////////
  // (Function "resets" day counter and adds to this value user defined day of grow.) /////////
  // If no changes were made to this value it will count days from the start of the program. //

  unsigned long timeElapsed = millis() - startDateReset ;
  int daysElapsed = (timeElapsed / 86400000);  //miliseconds per day (24h * 60 min * 60 sec * 1000 miliseconds)
  dayOfGrow = (startDate + daysElapsed);

  //Displaying day of grow
  tft.setCursor(3, 100);
  tft.setTextSize(3);
  tft.setTextColor(GREEN, BLACK);
  tft.print("DAY");
  tft.setTextColor(GREEN, BLACK);
  tft.setCursor(53, 100);
  tft.print(":");
  tft.setCursor(68, 100);
  tft.print(dayOfGrow);
  return micros() - start;
}


//================ SETTINGS SCREEN (Displaying settings RANGE for each parameter)


///////// This is a TOTAL range of set humidity and temperature for the grow. /////////////
// (For example - if humidity for night is set higher than for day - the night humidity ///
// will be diplayed as maximal humidity.) /////////////////////////////////////////////////

unsigned long settingsScreen() {
  unsigned long start = micros();

  tft.setCursor(17, 4);
  tft.setTextColor(GREEN, BLACK);
  tft.setTextSize(1);
  tft.println("SETTINGS RANGE:");

  //Printng settings for minimal temperature
  tft.setCursor(0, 25);
  tft.print("Min. temp:    ");
  if (minNightTemp <= minDayTemp) {
    tft.print(minNightTemp);
  } else {
    tft.print(minDayTemp);
  }
  //print "degree" char (moved up 3 pixels)
  tft.setCursor(114, 22);
  tft.print((char)248);
  //print "C" for Celcious
  tft.setCursor(118, 25);
  tft.print("C");

  //Printng settings for maximal temperature
  tft.setCursor(0, 35);
  tft.print("Max. temp:    ");
  if (maxDayTemp >= maxNightTemp) {
    tft.print(maxDayTemp);
  } else {
    tft.print(maxNightTemp);
  }
  //print "degree" char (moved up 3 pixels)
  tft.setCursor(114, 32);
  tft.print((char)248);
  //print "C" for Celcious
  tft.setCursor(118, 35);
  tft.print("C");

  //Printing settings for mimial humidity
  tft.setCursor(0, 45);
  tft.print("Min. humid:   ");
  if (minNightHumid <= minDayHumid) {
    tft.print(minNightHumid);
  } else {
    tft.print(minDayHumid);
  }
  tft.setCursor(118, 45);
  tft.print("%");

  //Printing settings for maximal humidity
  tft.setCursor(0, 55);
  tft.print("Max. humid:   ");
  if (maxNightHumid >= maxDayHumid) {
    tft.print(maxNightHumid);
  } else {
    tft.print(maxDayHumid);
  }
  tft.setCursor(118, 55);
  tft.print("%");

  //Displaying settings for "Day time"
  tft.setCursor(0, 65);
  tft.print("'Day' from: ");
  tft.setCursor(85, 65);
  if (dayStartTimeHours < 10) {
    tft.print("0");
    tft.print(dayStartTimeHours);
  } else tft.print(dayStartTimeHours);
  tft.print(":");
  if (dayStartTimeMinutes < 10) {
    tft.print("0");
    tft.print(dayStartTimeMinutes);
  } else tft.print(dayStartTimeMinutes);


  //Displaying settings for "Night Time"
  tft.setCursor(0, 75);
  tft.print("'Night' from: ");
  tft.setCursor(85, 75);
  if (nightStartTimeHours < 10) {
    tft.print("0");
    tft.print(nightStartTimeHours);
  } else tft.print(nightStartTimeHours);
  tft.print(":");
  if (nightStartTimeMinutes < 10) {
    tft.print("0");
    tft.print(nightStartTimeMinutes);
  } else tft.print(nightStartTimeMinutes);

  //Displaying grow cycle (day/night)
  tft.setCursor(0, 85);
  tft.setTextColor(GREEN, BLACK);
  tft.print("Fotoperiod:");
  tft.setCursor(85, 85);
  if (nightStartTimeHours > dayStartTimeHours) {
    if ((nightStartTimeHours - dayStartTimeHours) < 10) {
      tft.print(nightStartTimeHours - dayStartTimeHours);
      tft.print(" ");
    } else {
      tft.print(nightStartTimeHours - dayStartTimeHours);
    }
    tft.setCursor(98, 85);
    tft.print("/");
    tft.setCursor(105, 85);
    tft.print(24 - (nightStartTimeHours - dayStartTimeHours));
  }

  if (nightStartTimeHours < dayStartTimeHours) {
    if ((24 - (dayStartTimeHours - nightStartTimeHours)) < 10) {
      tft.print(24 - (dayStartTimeHours - nightStartTimeHours));
      tft.print(" ");
    }
    else {
      tft.print(24 - (dayStartTimeHours - nightStartTimeHours));
    }
    tft.setCursor(98, 85);
    tft.print("/");
    tft.setCursor(105, 85);
    tft.print(dayStartTimeHours - nightStartTimeHours);
    tft.print(" ");
  }
  //
  if (nightStartTimeHours == dayStartTimeHours) {
    tft.print("24");
    tft.setCursor(98, 85);
    tft.print("/");
    tft.setCursor(105, 85);
    tft.print("0 ");
  }

  //Displaying grow cycle hint

  /////// GROW CYCLE HINT ///////
  // This is just a hint for the grower - if "day" is longer than 12 hours the plants will be in VEGETATIVE stage //
  // if "day" is 12 hours or less - plants will be in BLOOMING stage. //////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // (this does not apply to "auto" strains, those are "fotoperiod independent" and will bloom even under 24/0 /////
  // day to night fotoperiod - to override 24h (24/0) fotoperiod set day and night start at the same time. /////////

  tft.setCursor(0, 95);
  tft.print("Grow cycle: ");
  tft.setCursor(83, 95);
  tft.setTextColor(GREEN, BLACK);

  if ((nightStartTimeHours * 60 + nightStartTimeMinutes) > (dayStartTimeHours * 60 + dayStartTimeMinutes)) {
    if ((nightStartTimeHours - dayStartTimeHours) > 12) {
      tft.setTextColor(GREEN, BLACK);
      tft.print("(VEGE) ");
    } else {
      tft.setTextColor(MAGENTA, BLACK);
      tft.print("(BLOOM)");
    }
  }
  if ((nightStartTimeHours * 60 + nightStartTimeMinutes) < (dayStartTimeHours * 60 + dayStartTimeMinutes)) {

    if ((24 - (dayStartTimeHours - nightStartTimeHours)) > 12) {
      tft.setTextColor(GREEN, BLACK);
      tft.print("(VEGE) ");
    } else {
      tft.setTextColor(MAGENTA, BLACK);
      tft.print("(BLOOM)");
    }
  }

  if ((nightStartTimeHours * 60 + nightStartTimeMinutes) == (dayStartTimeHours * 60 + dayStartTimeMinutes)) {
    tft.setTextColor(RED, BLACK);
    tft.print("<24h>   ");
  }

  tft.setCursor(5, 110);
  tft.setTextColor(GREEN);
  tft.setTextSize(1);
  tft.println("Press button to edit");
  return micros() - start;
}



//================ NIGHT OR DAY SETTINGS SELECTION SCREEN ================//


unsigned long settingsDayNightScreen() {
  unsigned long start = micros();

  tft.setCursor(8, 15);
  tft.setTextColor(GREEN);
  tft.setTextSize(1);
  tft.println("EDIT SETTINGS FOR:");

  tft.setCursor(15, 60);
  tft.setTextColor(YELLOW);
  tft.setTextSize(2);
  tft.println("DAY");
  tft.setCursor(65, 60);
  tft.setTextColor(BLUE);
  tft.println("NIGHT");

  if (settingsMenuNight == 1) {
    tft.drawRect(4, 55, 55, 25, 0x0000);
    tft.drawRect(61, 55, 64, 25, 0x07E0);

  }

  if (settingsMenuDay == 1) {
    tft.drawRect(61, 55, 64, 25, 0x0000);
    tft.drawRect(4, 55, 55, 25, 0x07E0);
  }
  tft.setCursor(10, 100);
  tft.setTextColor(GREEN);
  tft.setTextSize(1);
  tft.println("Selet day / night");
  tft.setCursor(20, 110);
  tft.println("and press button");
  return micros() - start;
}


//================ DAY SETTINGS SCREEN ================//

unsigned long daySettingsScreen() {
  unsigned long start = micros();
  tft.setTextColor(GREEN);
  tft.setCursor(10, 10);
  tft.setTextSize(1);
  tft.println("Settings for Day");
  tft.setCursor(0, 20);
  tft.print("Start Hour:");
  tft.setCursor(0, 30);
  tft.print("Start Minutes:");
  tft.setCursor(0, 40);
  tft.print("Min. temp.:");
  tft.setCursor(0, 50);
  tft.print("Max. temp.:");
  tft.setCursor(0, 60);
  tft.print("Min. humid.:");
  tft.setCursor(0, 70);
  tft.print("Max. humid.:");
  tft.setCursor(0, 80);
  tft.print("Day of grow:");
  //-------------------------------
  tft.setTextColor(GREEN, BLACK);
  tft.setCursor(110, 20);
  if (dayStartTimeHours < 10) {
    tft.print("0");
    tft.print(dayStartTimeHours);
  } else tft.print(dayStartTimeHours);

  tft.setCursor(110, 30);
  if (dayStartTimeMinutes < 10) {
    tft.print("0");
    tft.print(dayStartTimeMinutes);
  } else tft.print(dayStartTimeMinutes);

  tft.setCursor(95, 40);
  tft.print(minDayTemp);

  tft.setCursor(95, 50);
  tft.print(maxDayTemp);

  tft.setCursor(95, 60);
  tft.print(minDayHumid);

  tft.setCursor(95, 70);
  tft.print(maxDayHumid);

  tft.setCursor(95, 80);
  tft.print(startDate);

  tft.setCursor(50, 100);
  tft.setTextColor(GREEN, BLACK);
  tft.print("EXIT");

  if (setValues == 1) {
    tft.fillRect(90, 30, 20, 10, BLACK);
    tft.setTextColor(GREEN);
    tft.setCursor(90, 20);
    tft.print("->");
  }

  if (setValues == 2) {
    tft.fillRect(90, 20, 20, 10, BLACK);
    tft.fillRect(75, 40, 20, 10, BLACK);
    tft.setTextColor(GREEN);
    tft.setCursor(90, 30);
    tft.print("->");
  }

  if (setValues == 3) {
    tft.fillRect(90, 30, 20, 10, BLACK);
    tft.fillRect(75, 50, 20, 10, BLACK);
    tft.setTextColor(GREEN);
    tft.setCursor(75, 40);
    tft.print("->");
  }

  if (setValues == 4) {
    tft.fillRect(75, 40, 20, 10, BLACK);
    tft.fillRect(75, 60, 20, 10, BLACK);
    tft.setTextColor(GREEN);
    tft.setCursor(75, 50);
    tft.print("->");
  }

  if (setValues == 5) {
    tft.fillRect(75, 50, 20, 10, BLACK);
    tft.fillRect(75, 70, 20, 10, BLACK);
    tft.setTextColor(GREEN);
    tft.setCursor(75, 60);
    tft.print("->");
  }

  if (setValues == 6) {
    tft.fillRect(75, 60, 20, 10, BLACK);
    tft.fillRect(75, 80, 20, 10, BLACK);
    tft.setTextColor(GREEN);
    tft.setCursor(75, 70);
    tft.print("->");
  }

  if (setValues == 7) {
    tft.fillRect(75, 70, 20, 10, BLACK);
    tft.setTextColor(GREEN);
    tft.setCursor(75, 80);
    tft.print("->");
  }

  if (setValues == 8) {
    tft.fillRect(75, 80, 20, 10, BLACK);
    tft.setCursor(50, 100);
    tft.setTextColor(GREEN, RED);
    tft.print("EXIT");
  }

  return micros() - start;
}


//================ NIGHT SETTINGS SCREEN ================//


unsigned long nightSettingsScreen() {
  unsigned long start = micros();
  tft.setTextColor(GREEN);
  tft.setCursor(10, 10);
  tft.setTextSize(1);
  tft.println("Settings for NIGHT");
  tft.setCursor(0, 20);
  tft.print("Start Hour:");
  tft.setCursor(0, 30);
  tft.print("Start Minutes:");
  tft.setCursor(0, 40);
  tft.print("Min. temp.:");
  tft.setCursor(0, 50);
  tft.print("Max. temp.:");
  tft.setCursor(0, 60);
  tft.print("Min. humid.:");
  tft.setCursor(0, 70);
  tft.print("Max. humid.:");
  //----------------------------
  tft.setTextColor(GREEN, BLACK);
  tft.setCursor(110, 20);
  if (nightStartTimeHours < 10) {
    tft.print("0");
    tft.print(nightStartTimeHours);
  } else {
    tft.print(nightStartTimeHours);
  }

  //Displaying values
  tft.setCursor(110, 30);
  if (nightStartTimeMinutes < 10) {
    tft.print("0");
    tft.print(nightStartTimeMinutes);
  } else {
    tft.print(nightStartTimeMinutes);
  }

  tft.setCursor(95, 40);
  tft.print(minNightTemp);

  tft.setCursor(95, 50);
  tft.print(maxNightTemp);

  tft.setCursor(95, 60);
  tft.print(minNightHumid);

  tft.setCursor(95, 70);
  tft.print(maxNightHumid);

  tft.setCursor(50, 100);
  tft.setTextColor(GREEN, BLACK);
  tft.print("EXIT");

  if (setValues == 1) {
    tft.fillRect(90, 30, 20, 10, BLACK);
    tft.setTextColor(GREEN);
    tft.setCursor(90, 20);
    tft.print("->");
  }

  if (setValues == 2) {
    tft.fillRect(90, 20, 20, 10, BLACK);
    tft.fillRect(75, 40, 20, 10, BLACK);
    tft.setTextColor(GREEN);
    tft.setCursor(90, 30);
    tft.print("->");
  }

  if (setValues == 3) {
    tft.fillRect(90, 30, 20, 10, BLACK);
    tft.fillRect(75, 50, 20, 10, BLACK);
    tft.setTextColor(GREEN);
    tft.setCursor(75, 40);
    tft.print("->");
  }

  if (setValues == 4) {
    tft.fillRect(75, 40, 20, 10, BLACK);
    tft.fillRect(75, 60, 20, 10, BLACK);
    tft.setTextColor(GREEN);
    tft.setCursor(75, 50);
    tft.print("->");
  }

  if (setValues == 5) {
    tft.fillRect(75, 50, 20, 10, BLACK);
    tft.fillRect(75, 70, 20, 10, BLACK);
    tft.setTextColor(GREEN);
    tft.setCursor(75, 60);
    tft.print("->");
  }

  if (setValues == 6) {
    tft.fillRect(75, 60, 20, 10, BLACK);
    tft.fillRect(75, 80, 20, 10, BLACK);
    tft.setTextColor(GREEN);
    tft.setCursor(75, 70);
    tft.print("->");
  }

  if (setValues == 7) {

    tft.fillRect(75, 70, 20, 10, BLACK);
    tft.setCursor(50, 100);
    tft.setTextColor(GREEN, RED);
    tft.print("EXIT");
  }
  return micros() - start;
}


//================ CannaIoT INFO SCREEN ================//

unsigned long cannaIoTInfo() {
  unsigned long start = micros();
  tft.setTextColor(GREEN);
  tft.setCursor(2, 2);
  tft.setTextSize(1);
  tft.println("CannaIoT");
  tft.setCursor(35, 12);
  tft.println("by Hemp-help.pl");
  tft.setCursor(15, 45);
  tft.setTextSize(2);
  tft.println("CannaIoT");

  tft.setTextSize(1);
  tft.setCursor(50, 64);
  tft.println("ver. 0.440");
  tft.setCursor(8, 75);
  tft.println("Prototype440");
  tft.setCursor(3, 100);
  tft.println("Growing for GEEKS,");
  tft.setCursor(14, 112);
  tft.println("coding for FARMERS.");

  return micros() - start;
}

void screenSelect() {
  int xAxis = analogRead(joystickX);
  if (xAxis < 200) {
    mainMenuSelection++;
    if (mainMenuSelection > 3) {
      mainMenuSelection = 3;
    }
    else {
      tft.fillScreen();
    }
    delay(50);

  }
  if (xAxis > 900) {
    mainMenuSelection--;
    if (mainMenuSelection < 1) {
      mainMenuSelection = 1;
    }
    else {
      tft.fillScreen();
    }
    delay(50);
  }
}

void monthToWord() {
  if (myRTC.month == 1) {
    tft.print("Jan");
  };
  if (myRTC.month == 2) {
    tft.print("Feb");
  };
  if (myRTC.month == 3) {
    tft.print("Mar");
  };
  if (myRTC.month == 4) {
    tft.print("Apr");
  };
  if (myRTC.month == 5) {
    tft.print("May");
  };
  if (myRTC.month == 6) {
    tft.print("Jun");
  };
  if (myRTC.month == 7) {
    tft.print("Jul");
  };
  if (myRTC.month == 8) {
    tft.print("Aug");
  };
  if (myRTC.month == 9) {
    tft.print("Sep");
  };
  if (myRTC.month == 10) {
    tft.print("Oct");
  };
  if (myRTC.month == 11) {
    tft.print("Nov");
  };
  if (myRTC.month == 12) {
    tft.print("Dec");
  }
}
float analogXChange(float a) {
  int xAxis = analogRead(joystickX);
  if (xAxis < 250) {
    a++;
    delay(50);
  }
  if (xAxis > 850) {
    a--;
    delay(50);
  }
  return a;
}

int analogYChange(int a) {
  int yAxis = analogRead(joystickY);
  if (yAxis < 300) {
    a++;
    delay(50);
  }
  if (yAxis > 900) {
    a--;
    delay(50);
  }
  return a;
}

bool buttonEnter(bool a) {
  bool buttonState = digitalRead(buttonPin);
  if (buttonState != lastButtonState) {
    if (buttonState == LOW) {
      a = 1;
      delay(50);
      tft.fillScreen();
    }
    lastButtonState = buttonState;
  }
  return a;
}

void buttonExit() {
  bool buttonState = digitalRead(buttonPin);
  if (buttonState != lastButtonState) {
    if (buttonState == LOW) {
      settingsMenu = 0;
      settingsDayNightMenu = 0;
      setValues = 1;
      delay(250);
      tft.fillScreen();
    }
    lastButtonState = buttonState;
  }
}

void serialCommunication() {
  String incomingString = "";
  String valueString = "";

  while (Serial.available() > 0) {
    char inChar = Serial.read();
    incomingString += (char)inChar;

    // if you get a newline, print the string:
    if (inChar == '\n') {
      incomingString = "";
    }

    //Settings parameters via Serial communication

    //Day Time Start Hours
  
    if (incomingString == "setDayTimeStartHours") {
      delay(20);
      while (Serial.available() > 0) {
        char inChar = Serial.read();
        valueString += (char)inChar;
        if (inChar == '\n') {
          dayStartTimeHours = valueString.toInt();

          // clear the string for new input:
          valueString = "";
          incomingString = "";
        }
      }
    }

    //Setting Day Time Start Minutes

    if (incomingString == "setDayTimeStartMinutes") {
      delay(20);
      while (Serial.available() > 0) {
        char inChar = Serial.read();
        valueString += (char)inChar;
        if (inChar == '\n') {
          dayStartTimeMinutes = valueString.toInt();

          // clear the string for new input:
          valueString = "";
          incomingString = "";
        }
      }
    }

    //Setting minimal day temperature
    if (incomingString == "setMinDayTemp") {

      while (Serial.available() > 0) {
        char inChar = Serial.read();
        valueString += (char)inChar;
        if (inChar == '\n') {
          minDayTemp = valueString.toFloat();

          // clear the string for new input:
          valueString = "";
          incomingString = "";
        }
      }
    }

    //Setting max day temp

    if (incomingString == "setMaxDayTemp") {
      delay(20);
      while (Serial.available() > 0) {
        char inChar = Serial.read();
        valueString += (char)inChar;
        if (inChar == '\n') {
          maxDayTemp = valueString.toFloat();
          // clear the string for new input:
          valueString = "";
          incomingString = "";
        }
      }
    }

    //setting min Day Humid
    if (incomingString == "setMinDayHumid") {
      delay(20);

      while (Serial.available() > 0) {
        char inChar = Serial.read();
        valueString += (char)inChar;
        if (inChar == '\n') {
          minDayHumid = valueString.toFloat();
          valueString = "";
          incomingString = "";
        }
      }
    }
    //setting max Day Humid
    if (incomingString == "setMaxDayHumid") {
      delay(20);
      while (Serial.available() > 0) {
        char inChar = Serial.read();
        valueString += (char)inChar;
        if (inChar == '\n') {
          maxDayHumid = valueString.toFloat();
          valueString = "";
          incomingString = "";
        }
      }
    }

    //Settings NIGHT parameters via Serial communication

    //Night Start Time Hours
    if (incomingString == "setNightTimeStartHours") {
      delay(20);
      while (Serial.available() > 0) {
        char inChar = Serial.read();
        valueString += (char)inChar;
        if (inChar == '\n') {
          nightStartTimeHours = valueString.toInt();
          // clear the string for new input:
          valueString = "";
          incomingString = "";
        }
      }
    }

    //Setting Night Start Time Minutes

    if (incomingString == "setNightTimeStartMinutes") {
      delay(20);
      while (Serial.available() > 0) {
        char inChar = Serial.read();
        valueString += (char)inChar;
        if (inChar == '\n') {
          nightStartTimeMinutes = valueString.toInt();
          // clear the string for new input:
          valueString = "";
          incomingString = "";
        }
      }
    }

    //Setting minimal Night temperature
    if (incomingString == "setMinNightTemp") {
      delay(20);
      while (Serial.available() > 0) {
        char inChar = Serial.read();
        valueString += (char)inChar;
        if (inChar == '\n') {
          minNightTemp = valueString.toFloat();

          // clear the string for new input:
          valueString = "";
          incomingString = "";
        }
      }
    }

    //Setting max Night temp

    if (incomingString == "setMaxNightTemp") {
      delay(20);

      while (Serial.available() > 0) {
        char inChar = Serial.read();
        valueString += (char)inChar;
        if (inChar == '\n') {
          maxNightTemp = valueString.toFloat();

          // clear the string for new input:
          valueString = "";
          incomingString = "";
        }
      }
    }

    //setting min Night Humid
    if (incomingString == "setMinNightHumid") {
      delay(20);

      while (Serial.available() > 0) {
        char inChar = Serial.read();
        valueString += (char)inChar;
        if (inChar == '\n') {
          minNightHumid = valueString.toFloat();
          valueString = "";
          incomingString = "";
        }
      }
    }

    //setting max Night Humid
    if (incomingString == "setMaxNightHumid") {
      delay(20);
      while (Serial.available() > 0) {
        char inChar = Serial.read();
        valueString += (char)inChar;
        if (inChar == '\n') {
          maxNightHumid = valueString.toFloat();
          valueString = "";
          incomingString = "";
        }
      }
    }

// Updating values in ESP8266.
// (under developement)

    if (incomingString == "valuesUpdate") {
      
      Serial.println("setDayTimeStartHours");
      delay(10);
      Serial.println(dayStartTimeHours);
      delay(1000);
      Serial.println();
      
//      Serial.print("setDayTimeStartMinutes ");
//      delay(100);
//      Serial.println(dayStartTimeMinutes);
//      delay(1000);
//      Serial.println();
//      
//      Serial.print("setMinDayTemp ");
//      delay(100);
//      Serial.println(minDayTemp);
//      delay(500);
//      Serial.println();
//      delay(100);
//      
//      Serial.print("setMaxDayTemp");
//      delay(100);
//      Serial.println(maxDayTemp);
//      delay(100);
//      Serial.println();
//      delay(100);
//      
//      Serial.print("setMinDayHumid");
//      delay(100);
//      Serial.println(minDayHumid);
//      delay(100);
//      Serial.println();
//      delay(100);
//      
//      Serial.print("setMaxDayHumid");
//      delay(100);
//      Serial.println(maxDayHumid);
//      delay(100);
//      Serial.println();
//      delay(100);
//      
//      Serial.print("setNightTimeStartHours");
//      delay(100);
//      Serial.println(nightStartTimeHours);
//      delay(100);
//      Serial.println();
//      delay(100);
//      
//      Serial.print("setNightTimeStartMinutes");
//      delay(100);
//      Serial.println(nightStartTimeMinutes);
//      delay(100);
//      Serial.println();
//      delay(100);
//      
//      Serial.print("setMinNightTemp");
//      delay(100);
//      Serial.println(minNightTemp);
//      delay(100);
//      Serial.println();
//      delay(100);
//      
//      Serial.print("setMaxNightTemp");
//      delay(100);
//      Serial.println(maxNightTemp);
//      delay(100);
//      Serial.println();
//      delay(100);
//      
//      Serial.print("setMinNightHumid");
//      delay(100);
//      Serial.println(minNightHumid);
//      delay(100);
//      Serial.println();
//      delay(100);
//      
//      Serial.print("setMaxNightHumid");
//      delay(100);
//      Serial.println(maxNightHumid);
//      delay(100);
//      Serial.println();
//      delay(100);
    }

    if (incomingString == "hello") {
      Serial.println("Hello! This is CannaIoT masterMindAlpha (ver. Prototype440)");
      delay(100);
    }

    if (incomingString == "handshake") {
      Serial.println("handshake");
    }
  }
}

// END OF PROGRAM //
