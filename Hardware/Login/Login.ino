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
String serverName = "http://192.168.0.101:8000/mess/login/";


//RFID decalarations
#define SS_PIN 5
#define RST_PIN 27
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key;
byte nuidPICC[4];


//LCD declarations
const int rs = 4, en = 13, d4 = 14, d5 = 21, d6 = 15, d7 = 22;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


//Variable declarations
int Pincode;
int httpResponseCode;
int Buzzer = 25;
int TIME_THR = 2000;
unsigned long  CurrentTime, PreviousTime=0;
unsigned long wifi_delay = 5000;

String extent;

void setup() {
  Serial.begin(115200);
  Serial.println("Yayyy!!");
  //setup for HX711
  //pinMode(reset, INPUT_PULLUP);
  //scale.set_scale();
  //scale.tare();
  pinMode(Buzzer, OUTPUT);
  
  // setup for RC522
  SPI.begin();      // Init SPI bus
  rfid.PCD_Init();  // Init MFRC522 
  
  for (byte i = 0; i < 6; i++) {            //48 bits key
    key.keyByte[i] = 0xFF;
  }
  
  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  Serial.println(Hex_to_String(key.keyByte, MFRC522::MF_KEY_SIZE));

  //LCD setup for 16x2 display module
  lcd.begin(16, 2);
  LCDprint("hello world", 0);

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

  //random seed
  randomSeed(analogRead(0));
  valid_card_beep();
}


void loop(){
  
  Pincode = random(1000, 10000);                                        //generate a 4-digit pin
  Serial.println(Pincode);                            
  lcd.clear();
  LCDprint(String(Pincode), 0);

  
  if (WiFi.status()==WL_CONNECTED){
    HTTPClient http;
    String serverpath = serverName + "pin?code=" + String(Pincode);
    Serial.println(serverpath);
    http.begin(serverpath.c_str());
    httpResponseCode = http.GET();
    if (httpResponseCode = 202) {
      Serial.println("PinCode Accepted");
      LCDprint("Pincode Accepted", 1);   
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
      //delay(500);
      serverpath = serverName + extent;
      Serial.println(serverpath);      
      http.begin(serverpath.c_str());
      switch (httpResponseCode = http.GET()){
        case 204:
        {
          delay(200);
          continue;                    
        }
        case 404:
        {
          LCDprint("Not Registered", 0);
          error_beep();
          delay(1000);
          break;          
        }
        case 403:
        {
          String payload = http.getString();
          LCDprint("Sorry, " + payload, 0);
          LCDprint("Meal Forbidden", 1);
          error_beep();          
          delay(1000);   
          break;       
        }
        case 208:
        {
          String payload = http.getString();
          LCDprint("Hii, " + payload, 0); 
          LCDprint("Already Taken", 0);
          two_long();          
          delay(1000);
          break;          
        } 
        case 201:
        {
          String payload = "Hii, " +  http.getString();
          LCDprint(payload, 0);
          LCDprint("Take Your Plate", 1); 
          mapping_done_beep();          
          delay(1000);                   
          break;       
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
