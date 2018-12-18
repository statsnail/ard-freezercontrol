#include <OneWire.h>                // Library for reading voltage from temperature sensor
#include <DallasTemperature.h>      // Library for converting voltage into temparature
#include <PID_v1.h>                 // Library for PID regulator
#include <LiquidCrystal.h>          // Library for controlling LCD display conectet to Arduino microcontroller

// Statsnail Freezer controller v1, requires extra libraries:
// PID v1.2.0 by Brett
// DallasTemperature by Miles Burton
// OneWire 2.3.4 maintained by Paul Stoffregen

// Temperature +- 2 degree Celsius, continue work on PID
// ard-freezercontrol.ino

#define ONE_WIRE_BUS 2              // Temperature sensor is conected to pin 2
#define HOT_WIRE_RELAY 10           // Realy is conected to pin 10 and 11
#define NEUTRAL_WIRE_RELAY 11       // Realy is conected to pin 10 and 11

OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);   // Setting up pins for LCD disply

double Setpoint, Input, Output;         // Defining double presision float variables for PID
PID myPID(&Input, &Output, &Setpoint,2,5,1, REVERSE);

float sensor0 = 0.0;      // 

int adc_key_in  = 0;    // Defining buttons on LCD display
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

byte delta[8] =         // Defining a delta for the LCD display
{
  B00000,
  B00000,
  B00100,
  B01010,
  B10001,
  B11111,
  B00000,
  B00000
};

int read_LCD_buttons()            // Defining buttons on LCD display
{
 adc_key_in = analogRead(0);
 if (adc_key_in > 1000) return btnNONE;
 if (adc_key_in < 50)   return btnRIGHT;  
 if (adc_key_in < 250)  return btnUP; 
 if (adc_key_in < 450)  return btnDOWN; 
 if (adc_key_in < 650)  return btnLEFT; 
 if (adc_key_in < 850)  return btnSELECT;  
 return btnNONE;
}

void setup()                 // Procdure settup 
{
  sensors.begin();           // start the library
  pinMode(HOT_WIRE_RELAY, OUTPUT);
  pinMode(NEUTRAL_WIRE_RELAY, OUTPUT);
  digitalWrite(HOT_WIRE_RELAY, !LOW);
  digitalWrite(NEUTRAL_WIRE_RELAY, !LOW);

  sensors.requestTemperatures();
  sensor0 = sensors.getTempCByIndex(0);
  
  Input = sensor0;      // reading temperature into Input variable
  
  Setpoint = +4; // Default temperature setpoint (Degrees C)
 
  myPID.SetMode(AUTOMATIC);
  lcd.createChar(0, delta);
  lcd.begin(16, 2);              // start the library

  lcd.setCursor(0,0);
  lcd.print("Starting PID"); // print a simple message
  Serial.begin(9600);
}

void turnOnRelays(){                             // Function to turn on relays
  digitalWrite(HOT_WIRE_RELAY, !HIGH);
  digitalWrite(NEUTRAL_WIRE_RELAY, !HIGH);
}

void turnOffRelays(){                          // Function to turn off relays
  digitalWrite(HOT_WIRE_RELAY, !LOW);
  digitalWrite(NEUTRAL_WIRE_RELAY, !LOW);
}

void updateLcd(){                             // Function for updating LCD display 
 lcd.setCursor(0,0);
 lcd.print("SP:       ");
 lcd.setCursor(9,0);
 lcd.print(Setpoint);
 lcd.print("\337C");
 lcd.setCursor(0,1);
 lcd.print(sensor0);
 lcd.print("\337C");
 lcd.print(" ");
 lcd.print("\366");
 lcd.write(byte(0));
 lcd.print(Output);
}
 
void loop()                                //Procedure loop, run continiusly
{
  unsigned long startTime = millis();
  Serial.println("Start of cycle");
  sensors.requestTemperatures();
  sensor0 = sensors.getTempCByIndex(0);

  Serial.print("Input: ");
  Serial.println(sensor0);
  Input = sensor0;
  myPID.Compute();
  Serial.print("Output: ");
  Serial.println(Output);

  // if (Output > 100){              // PID is still running, but no longer influence the relays
  //   turnOnRelays();
  // } else {
  //   turnOffRelays();
  // }

  if (sensor0 > (Setpoint+0.1)){     // Turn on the relays if measured temperature is larger than (Setpoint+0.1). 0.1 is the hysterese. 
    turnOnRelays();                  // Turning on the relays means turnng on the cooling. 
  }
  if (sensor0 < (Setpoint-0.1)){     // Turn on the relays if measured temperature is smaler than (Setpoint-0.1). 0.1 is the hysterese.
    turnOffRelays();                 // Temperature will vary 0.2 degreees C. This to prevent that the relays are switched too often. 
  }

  if (read_LCD_buttons() == btnUP){
    Setpoint = Setpoint + 1;
  }
  if (read_LCD_buttons() == btnDOWN){
    Setpoint = Setpoint - 1;
  }

  updateLcd();
  
  Serial.print("End of cycle, ms: ");
  Serial.println(millis()-startTime);
}
