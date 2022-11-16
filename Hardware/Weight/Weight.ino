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

const char* ssid = "RAJ 7539";
const char* password = "Alohomora";

//Your Domain name with URL path or IP address with path
String serverName = "http://192.168.10.81:8000/mess/register/";



//RFID decalarations
#define SS_PIN 5
#define RST_PIN 27
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key;
byte nuidPICC[4];


const int rs = 4, en = 13, d4 = 14, d5 = 21, d6 = 15, d7 = 22;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int Pincode;
int Exit = 0;
int EnterPin = 25;
int CancelPin = 26;
int httpResponseCode;
const byte interruptpin = 2;
int data = 0, Enter = 0, Cancel = 0;

void setup() {
  Serial.begin(115200);
 
  pinMode(EnterPin, INPUT);
  pinMode(CancelPin, INPUT);  
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
  lcd.print("Hello World");

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

  randomSeed(analogRead(0));
}


void loop(){
  /*
  if (valid_card()){
    Serial.println(Hex_to_String(rfid.uid.uidByte, rfid.uid.size));
    lcd.setCursor(0, 1);
    lcd.print(Hex_to_String(rfid.uid.uidByte, rfid.uid.size));
  }
  delay(1000);
  Serial.println("OK");*/
  Pincode = random(1000, 10000);  
  Serial.println(Pincode);
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print(Pincode);
  data = 0;
  Enter = 0;
  Cancel = 0;
  
  if (WiFi.status()==WL_CONNECTED){
    HTTPClient http;
    String serverpath = serverName + String(Pincode);
    Serial.println(serverpath);
    http.begin(serverpath.c_str());
    httpResponseCode = http.GET();
    if (httpResponseCode = 202) {
      Serial.print("PinCode Accepted");
      lcd.setCursor(0, 1);
      lcd.print("  Pin Accepted ");     
      }
    else{
      Serial.print("Rejected with Error code:");
      Serial.println(httpResponseCode);
    }
    
    while(data == 0){
      lcd.setCursor(0, 1);
      lcd.print("Tap ID or use pin"); 
      if (valid_card()){
        Serial.println("Valid Card detected");
        lcd.setCursor(0, 0);    
        lcd.print("    " + Hex_to_String(rfid.uid.uidByte, rfid.uid.size));
        lcd.setCursor(0, 1);
        lcd.print("  Card Detected  ");
        delay(1000);
        lcd.se tCursor(0, 1);
        lcd.print("  Push to send");
        Serial.print("waiting for user to push any button");
        delay(500);
        while(Enter == 0 && Cancel == 0){
          Serial.println("looping");       
          Enter = digitalRead(EnterPin);
          delay(10);
          Cancel = digitalRead(CancelPin);
          delay(10);                                               
        }
        if (Enter == 1){
          lcd.setCursor(0, 1);
          lcd.print("  sending...");
          delay(100);
          Serial.println(Hex_to_String(rfid.uid.uidByte, rfid.uid.size).length());
          serverpath = serverName + Hex_to_String(rfid.uid.uidByte, rfid.uid.size);
          http.begin(serverpath.c_str());
          httpResponseCode = http.GET();
          if (httpResponseCode = 423) {
            lcd.setCursor(0, 1);
            lcd.print("  Mapping Done  ");
          }
          else{
            lcd.setCursor(0, 1);
            lcd.print("Error");
          }                
        }
        data = 1;
      }      
    }

  } }

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
    for (byte i = 0; i < bufferSize; i++){
      if (buffer[i] <  0x10){
        ID = "Invalid";
        return ID;        
      }        
      Number = String(buffer[i], HEX);
      Number.toUpperCase();
      ID = ID + Number;
    }
    return ID;
}
