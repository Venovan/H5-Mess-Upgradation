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

const char* ssid = "Samsung M31";
const char* password = "12345678d";

//Your Domain name with URL path or IP address with path
String serverName = "http://192.168.100.81:8000/mess/weight/";



//RFID decalarations
#define SS_PIN 5
#define RST_PIN 27
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key;
byte nuidPICC[4];


//HX711 declaration
#define DOUT 5
#define CLK 6
//HX711 scale(DOUT, CLK);


//LCD declarations
const int rs = 4, en = 13, d4 = 14, d5 = 21, d6 = 15, d7 = 22;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


//Variable declarations
int Pincode;
int httpResponseCode;


void setup() {
  Serial.begin(115200);
  
  //setup for HX711
  //pinMode(reset, INPUT_PULLUP);
  //scale.set_scale();
  //scale.tare();
  
  
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
  lcd.print("Hello world");

  //WiFi setup
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
 
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  //random seed
  randomSeed(analogRead(0));
}


void loop(){
  
  Pincode = random(1000, 10000);                                        //generate a 4-digit pin
  Serial.println(Pincode);                            
  lcd.clear();
  LCDprint(String(Pincode), 1);

  
  if (WiFi.status()==WL_CONNECTED){
    HTTPClient http;
    String serverpath = serverName + String(Pincode);
    Serial.println(serverpath);
    http.begin(serverpath.c_str());
    httpResponseCode = http.GET();
    if (httpResponseCode = 202) {
      Serial.print("PinCode Accepted");
      LCDprint("Pincode Accepted", 1);   
    }
    else{
      Serial.print("Rejected with Error code:");
      Serial.println(httpResponseCode);
    }
    
    while(true){
      LCDprint("Tap ID/Use pin", 1); 
      if (valid_card()){        
        Serial.println("Valid Card Found");   
        LCDprint(Hex_to_String(rfid.uid.uidByte, rfid.uid.size), 0);
        LCDprint("Valid Card Found", 1);
        delay(1000);
        serverpath = serverName + Hex_to_String(rfid.uid.uidByte, rfid.uid.size);
        http.begin(serverpath.c_str());
        switch(httpResponseCode = http.GET()){
          case 403:
          {
            String payload = http.getString();
            LCDprint("Sorry, " + payload, 0);
            LCDprint("Meal Forbidden", 1);
            break;   
          }                           
          case 200:
          {
            String payload =  http.getString();
            LCDprint("Hii, " + payload, 0);
            LCDprint("Take Your Plate", 1);                    
            break;
          }                      
          case 208:
          {           
            String payload = http.getString();
            LCDprint("Hii, " + payload, 0); 
            LCDprint("Already Taken", 0);
            break;
          }
          case 404:
          {         
            LCDprint("Not Registered", 0);
            break;
          }
          default:
            LCDprint("Unknown Response", 0);                              
        }
        break;                 
      }
      serverpath = serverName + "update";
      http.begin(serverpath.c_str());
      httpResponseCode = http.GET();
      Serial.println("Sent data");
      if (httpResponseCode == 204){
        Serial.println("204 no content");        
        continue;
      }
      switch(httpResponseCode){                     
        case 403:
        {
          String payload = http.getString();
          LCDprint("Sorry, " + payload, 0);
          LCDprint("Meal Forbidden", 1); 
          break; 
        }                 
        case 200:
        {
          String payload =  http.getString();
          LCDprint("Hii, " + payload, 0);
          LCDprint("Take Your Plate", 1);                    
          break;
        }
        case 208:
        {        
          String payload = http.getString();
          LCDprint("Hii, " + payload, 0); 
          LCDprint("Already Taken", 0);
          break;
        }
        case 404: 
        {                  
          LCDprint("Not Registered", 0);
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
    if (rfid.uid.uidByte[0] != nuidPICC[0] || 
        rfid.uid.uidByte[1] != nuidPICC[1] || 
        rfid.uid.uidByte[2] != nuidPICC[2] || 
        rfid.uid.uidByte[3] != nuidPICC[3] ) {
            for (byte i = 0; i < 4; i++) {
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
    lcd.setCursor(0, line);  
    if (len > 16){
      lcd.print("  Length Error  ");
    }
    else{
      if (len % 2 == 0){
        lcd.print(multiply(" ", (16-len)/2) + msg + multiply(" ", (16-len)/2));  
      }
      else{
        lcd.print(multiply(" ", (16-len-1)/2) + msg + multiply(" ", (16-len+1)/2));  
      } 
    }
    }


String multiply(String msg, int multiple){
  for (int i =0; i<multiple; i++){
    msg = msg + msg;
  }
  return msg;
}
