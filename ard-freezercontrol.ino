#include <OneWire.h> 
#include <DallasTemperature.h>
#include <PID_v1.h>
#include <LiquidCrystal.h>

// Statsnail Freezer controller v1, requires extra libraries:
// PID v1.2.0 by Brett
// DallasTemperature by Miles Burton
// OneWire 2.3.4 maintained by Paul Stoffregen

// Temperature +- 2 degree Celsius, continue work on PID
// ard-freezercontrol.ino

#define ONE_WIRE_BUS 2
#define HOT_WIRE_RELAY 10
#define NEUTRAL_WIRE_RELAY 11

OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

double Setpoint, Input, Output;
PID myPID(&Input, &Output, &Setpoint,2,5,1, REVERSE);

float sensor0 = 0.0;

int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

byte delta[8] =
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

int read_LCD_buttons()
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

void setup()
{
  sensors.begin();
  pinMode(HOT_WIRE_RELAY, OUTPUT);
  pinMode(NEUTRAL_WIRE_RELAY, OUTPUT);
  digitalWrite(HOT_WIRE_RELAY, !LOW);
  digitalWrite(NEUTRAL_WIRE_RELAY, !LOW);

  sensors.requestTemperatures();
  sensor0 = sensors.getTempCByIndex(0);
  
  Input = sensor0;
  
  Setpoint = -10; // Default temperature setpoint
 
  myPID.SetMode(AUTOMATIC);
  lcd.createChar(0, delta);
  lcd.begin(16, 2);              // start the library

  lcd.setCursor(0,0);
  lcd.print("Starting PID"); // print a simple message
  Serial.begin(9600);
}

void turnOnRelays(){
  digitalWrite(HOT_WIRE_RELAY, !HIGH);
  digitalWrite(NEUTRAL_WIRE_RELAY, !HIGH);
}

void turnOffRelays(){
  digitalWrite(HOT_WIRE_RELAY, !LOW);
  digitalWrite(NEUTRAL_WIRE_RELAY, !LOW);
}

void updateLcd(){
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
 
void loop()
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

  if (Output > 100){
    turnOnRelays();
  } else {
    turnOffRelays();
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
