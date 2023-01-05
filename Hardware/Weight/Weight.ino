/*
  The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
*/


#include <MFRC522.h>
#include <LiquidCrystal.h>
#include <HTTPClient.h>
#include <String.h>
#include <Wifi.h>
#include "HX711.h"

const char* ssid = "H5Mess";
const char* password = "hostel5mess";

//Your Domain name with URL path or IP address with path
String serverName = "http://192.168.0.101:8000/mess/weight/";



//RFID decalarations
#define SS_PIN 5
#define RST_PIN 27


#define WEIGHT_THR 30
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key;
byte nuidPICC[4];


//HX711 declaration
#define DOUT 33
#define CLK 32

HX711 scale(DOUT, CLK);


//LCD declarations
const int rs = 4, en = 13, d4 = 14, d5 = 21, d6 = 15, d7 = 22;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


//Variable declarations
int Pincode;
int httpResponseCode;
int Buzzer = 25;
int TIME_THR = 2000;
unsigned long CurrentTime, PreviousTime = 0;
unsigned long wifi_delay = 5000;
float calibrationFactor;
float knownWeight = 210;
String extent;

void setup() {
  Serial.begin(115200);
  pinMode(Buzzer, OUTPUT);

  //LCD setup for 16x2 display module
  lcd.begin(16, 2);
  LCDprint("hello World", 0);


  //WiFi setup
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() >= wifi_delay){
      LCDprint("Router not found", 0);
      LCDprint("Restarting...", 1);
      delay(500);
      ESP.restart();  
    }
    delay(500);
    Serial.print(".");
  }
 
 
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  valid_card_beep();

  
   //HX711 setup
  Calibrate();
  
  
  // setup for RC522
  SPI.begin();      // Init SPI bus
  rfid.PCD_Init();  // Init MFRC522 
  
  for (byte i = 0; i < 6; i++) {            //48 bits key
    key.keyByte[i] = 0xFF;
  }
  
  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  Serial.println(Hex_to_String(key.keyByte, MFRC522::MF_KEY_SIZE));


  //random seed
  randomSeed(analogRead(0));
  valid_card_beep();
 
  
}


void loop(){


  Pincode = random(1000, 10000);                                        //generate a 4-digit pin
  Serial.println(Pincode);                            
  lcd.clear();
  
  
       
  if (WiFi.status()==WL_CONNECTED){
    HTTPClient http;
    String serverpath = serverName + "pin?code=" +String(Pincode);
    Serial.println(serverpath);
    http.begin(serverpath.c_str());
    httpResponseCode = http.GET();
    if (httpResponseCode = 202) {
      Serial.println("PinCode Accepted");
      LCDprint("Pincode Accepted", 1); 
      delay(200);  
    }
    else{
      Serial.print("Rejected with Error code:");
      Serial.println(httpResponseCode);
    }
    
    while(true){
      LCDprint(String(Pincode), 0);
      LCDprint("Tap ID/Use pin", 1); 

      if ((WiFi.status() != WL_CONNECTED) && (millis() >= wifi_delay)){
          LCDprint("Router not found", 0);
          LCDprint("Reconnecting...", 1);
          delay(500);
          ESP.restart();
      }  
      
          
      if (valid_card()){
        Serial.println("Valid Card Found");   
        //LCDprint(Hex_to_String(rfid.uid.uidByte, rfid.uid.size), 0);
        LCDprint("Card Found", 1);  
        valid_card_beep();     
        extent = "recognise?rfid=" + Hex_to_String(rfid.uid.uidByte, rfid.uid.size);
      }
      else{
        extent= "recognise";
      }

      serverpath = serverName + extent;
      http.begin(serverpath.c_str());
      switch (httpResponseCode = http.GET()){
        case 204:
        {
          continue;                    
        }
        case 404:
        {
          LCDprint("Not Registered", 0);
          LCDprint("Register first", 1);
          error_beep();
          delay(1000);
          break;          
        }
        case 206:
        {
          String payload = http.getString();
          LCDprint("Hii, " + payload, 0);
          LCDprint("No Meal Taken", 1);
          error_beep();
          delay(1000);
          break;                                        
        }
        case 405:
        {
          String payload = http.getString();
          LCDprint("Sorry, " + payload, 0);
          LCDprint("Already Weighed", 1);   
          two_long();
          delay(1000);
          break;       
        }
        case 202:
        {
          String payload =http.getString();
          int weight = weighing(payload);  
          Serial.println(String(weight)); 
          if (weight == -1){
              error_beep();
              LCDprint("No weight", 0);
              LCDprint("Try again!", 1);
              delay(1000);              
              serverpath = serverName + "update";
          }
          else{    
            serverpath = serverName + "update?weight=" + String(weight);  
          }          
          http.begin(serverpath.c_str());
          switch(httpResponseCode = http.GET()){
            case 204:
            {
              LCDprint("No Content", 1);  
              delay(1000);            
              break;
            }
            case 205:
            {
              LCDprint("try again", 1);
              delay(1500);
              break;
            }
            case 423:
            {
              LCDprint("weight: " + String(weight) + "gms", 0);              
              LCDprint("Weight updated", 1); 
              mapping_done_beep();
              delay(1500);             
              break;
            }
            default:
            {
              LCDprint("Unknown Response", 0);
            }
          break;
          }
        } 
        default:
          LCDprint("Unknown Response", 0);                  
      }
      break;      
    }
  }
}

bool valid_card(){
    //not valid if no card is nearby
    if ( ! rfid.PICC_IsNewCardPresent()){
        return false;
    }
    // not valid if NUID is not readed
    if (! rfid.PICC_ReadCardSerial()){
        return false;
    }
    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
    //not valid if card is not MIFARE Classic type
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
        piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
        piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
            return false;
        }

    //valid if new card is read than previous one
    /*if (rfid.uid.uidByte[0] != nuidPICC[0] || 
        rfid.uid.uidByte[1] != nuidPICC[1] || 
        rfid.uid.uidByte[2] != nuidPICC[2] || 
        rfid.uid.uidByte[3] != nuidPICC[3] ) {
            for (byte i = 0; i < 4; i++) {
                nuidPICC[i] = rfid.uid.uidByte[i];
            }
            rfid.PICC_HaltA();
            rfid.PCD_StopCrypto1();
            return true;
        }*/
    CurrentTime = millis();
    if ((CurrentTime - PreviousTime) > TIME_THR) {
      PreviousTime = CurrentTime;
      for (byte i = 0; i < 4; i++){
        nuidPICC[i] = rfid.uid.uidByte[i];
      }
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
      return true;
    }
    
    
    // Halt PICC
    rfid.PICC_HaltA();

    // Stop encryption on PCD
    rfid.PCD_StopCrypto1();

    return false;
    }


void Calibrate(){
  single_beep();   
  LCDprint("Starting", 0);
  LCDprint("Calibration", 1);
  delay(500);
  
  single_beep();   
  LCDprint("Remove All", 0); 
  LCDprint("Weights", 1);
  delay(2000); 
  scale.set_scale();
  scale.tare();
  scale.read_average();  
  
  single_beep();   
  LCDprint("Place a known", 0); 
  LCDprint("Weight", 1);
  delay(2000);
  long reading = scale.get_units(20);
  single_beep(); 
  LCDprint("Remove the", 0); 
  LCDprint("known weight", 1);
  delay(2000);
  calibrationFactor = reading/knownWeight;
  scale.set_scale(calibrationFactor);
  scale.tare();
  mapping_done_beep();
  lcd.clear();
  Serial.println(String(calibrationFactor));
  LCDprint("Calibration :", 0); 
  LCDprint(String(calibrationFactor), 1);  
  delay(1000);
  lcd.clear();
}

void valid_card_beep(){
  digitalWrite(Buzzer, HIGH);
  delay(500);
  digitalWrite(Buzzer, LOW);
  delay(300);
  digitalWrite(Buzzer, HIGH);
  delay(100);
  digitalWrite(Buzzer, LOW);
}

void two_long(){
  for (int i=0; i<2; i++){
    digitalWrite(Buzzer, HIGH);
    delay(100);
    digitalWrite(Buzzer, LOW);
    delay(100);    
  }  
}

void mapping_done_beep(){
  digitalWrite(Buzzer, HIGH);
  delay(600);
  digitalWrite(Buzzer, LOW);
}

void error_beep(){
  for (int i=0; i<3; i++){
    digitalWrite(Buzzer, HIGH);
    delay(200);
    digitalWrite(Buzzer, LOW);
    delay(200);    
  }
}

void single_beep(){
  digitalWrite(Buzzer, HIGH);
  delay(300);
  digitalWrite(Buzzer, LOW);
}

String Hex_to_String(byte *buffer, byte bufferSize){
    String ID = "";
    String Number = "";
    for (byte i = 0; i< bufferSize; i++){
      if (buffer[i] < 0x10){
        Number = "0" + String(buffer[i], HEX); 
      }
      else{
        Number = String(buffer[i], HEX);
      }        
      Number.toUpperCase();
      ID = ID + Number;
    }      
    return ID;
}



int weighing(String name){
  name = "Hii! " + name; 
  LCDprint(name, 0);
  LCDprint("weigh your plate", 1);
  mapping_done_beep();  
  delay(500);
  for (int i=0; i<5; i++){
    
    long int reading1 = scale.get_units(20);
    delay(100);
    long int reading2 = scale.get_units(20);
    long int reading = (reading1 + reading2)/2;
    if (!((abs(reading1 - reading2) < WEIGHT_THR) && (reading1 > WEIGHT_THR) && (reading2 > WEIGHT_THR) && (reading < 10000))) {   //weight below 10000 and above WEIGHT_THR are valid
      LCDprint("waiting!", 1);
      single_beep();
      delay(10);                  
      LCDprint("  ", 1);
      continue;
    } 
    lcd.clear();
    return reading;
  }
  lcd.clear();
  return -1;
}



void LCDprint(String msg, int line){  
    int len = msg.length();
    Serial.println(len);        
    lcd.setCursor(0, line);  
    if (len > 16){
      lcd.print("   Length Error   ");
    }
    else{
      if (len % 2 == 0){
        Serial.println(multiply(" ", (16-len)/2) + msg + multiply(" ", (16-len)/2));
        lcd.print(multiply(" ", (16-len)/2) + msg + multiply(" ", (16-len)/2));  
      }
      else{
        Serial.println(multiply(" ", (16-len-1)/2) + msg + multiply(" ", (16-len+1)/2));
        lcd.print(multiply(" ", (16-len-1)/2) + msg + multiply(" ", (16-len+1)/2));  
      } 
    }
    }


String multiply(String msg, int multiple){
  String str = "";
  for (int i =0; i<multiple; i++){
    str = str + msg;
  }
  return str;
}
