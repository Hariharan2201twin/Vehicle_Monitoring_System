#include <SoftwareSerial.h>
#define RX 8
#define TX 9

const int trigPin = A4;
const int echoPin = A5;
long duration;
int distanceCm, distanceInch;


String AP = "Oops";       // CHANGE ME
String PASS = "sweetchamp"; // CHANGE ME
String API = "8LF9CJLQ78WPZBDM";   // CHANGE ME
String HOST = "api.thingspeak.com";
String PORT = "80";
String field1 = "field1";
String field2 = "field2";
String field3 = "field3";

int countTrueCommand;
int countTimeCommand;
boolean found = false;
int valSensor = 1;
int val = 0;

// include the library code:
#include <LiquidCrystal.h>
// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(10, 3, 4, 5, 6, 7);

int encoder_pin = 2; // pulse output from the module
unsigned int rpm; // rpm reading
volatile byte pulses; // number of pulses
unsigned long timeold;
// number of pulses per revolution
// based on your encoder disc
unsigned int pulsesperturn = 12;
int LM35 = A0;
int POT = A1;
int last_val;
int Dis;

void counter()
{
  //Update count
  pulses++;
}
int Val;

int M1 = 11;

SoftwareSerial esp8266(RX, TX);

void setup() {
  Serial.begin(9600);
  esp8266.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  analogWrite(M1, 0);
  pinMode(encoder_pin, INPUT);
  //Interrupt 0 is digital pin 2
  //Triggers on Falling Edge (change from HIGH to LOW)
  attachInterrupt(0, counter, FALLING);
  // Initialize
  pulses = 0;
  rpm = 0;
  timeold = 0;
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Initializing... ");
  lcd.setCursor(0, 1);
  lcd.print("                ");
  delay(100);
  sendCommand("AT", 5, "OK");
  sendCommand("AT+CWMODE=1", 5, "OK");
  sendCommand("AT+CWJAP=\"" + AP + "\",\"" + PASS + "\"", 20, "OK");
}
void loop() {
  Dis = (Val * millis()/1000)/1000;
  String getData = "GET /update?api_key=" + API + "&" + field1 + "=" + String(analogRead(LM35) * 5 / 10.23) + "&" + field2 + "=" + String(Val) + "&" + field3 + "=" + String(Dis);
  sendCommand("AT+CIPMUX=1", 2, "OK");
  sendCommand("AT+CIPSTART=0,\"TCP\",\"" + HOST + "\"," + PORT, 2, "OK");
  sendCommand("AT+CIPSEND=0," + String(getData.length() + 4), 1, ">");
  esp8266.println(getData); countTrueCommand++;
  sendCommand("AT+CIPCLOSE=0", 1, "OK");

  if (millis() - timeold >= 1000) {
    //Don't process interrupts during calculations
    detachInterrupt(0);
    rpm = (60 * 1000 / pulsesperturn ) / (millis() - timeold) * pulses;
    timeold = millis();
    pulses = 0;
    //Serial.print("RPM = ");
    //Serial.println(rpm / 4, DEC);
    //Restart the interrupt processing
    attachInterrupt(0, counter, FALLING);
  }
  Val = map(analogRead(POT), 0, 1023, 0, 80);
  Serial.println(Val);
  analogWrite(M1, Val);

  lcd.setCursor(0, 0);
  lcd.print("RPM: ");
  lcd.print(Val*2);
  lcd.print("       ");
  /*lcd.print(" KM/h | ");
  lcd.print(Dis);
  lcd.print(" km   ");
  lcd.setCursor(0, 1);
  lcd.print("Temp:");
  lcd.print(analogRead(LM35) * 5 / 10.23);
  lcd.print("deg C  ");*/
}

void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while (countTimeCommand < (maxTime * 1))
  {
    esp8266.println(command);//at+cipsend
    if (esp8266.find(readReplay)) //ok
    {
      found = true;
      break;
    }

    countTimeCommand++;
  }

  if (found == true)
  {
    Serial.println("OYI");
    countTrueCommand++;
    countTimeCommand = 0;
  }

  if (found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }

  found = false;
}
