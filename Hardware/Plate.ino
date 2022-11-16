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



#include <Wifi.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>
#include <HTTPClient.h>
#include <String.h>


const char* ssid = "Hostel5Mess";
const char* password = "hostel5mess";

char path[] = "/echo";
char host[] = "demos.kaazing.com";

string login= "http://127.0.0.1/";

//RFID decalarations
#define SS_PIN 10
#define RST_PIN 9
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key;
byte nuidPICC[4];


const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int Pincode;


void setup() {
  Serial.begin(115200);
 
  // setup for RC522
  SPI.begin();      // Init SPI bus
  rfid.PCD_Init();  // Init MFRC522 
  
  for (byte i = 0; i < 6; i++) {            //48 bits key
    key.keyByte[i] = 0xFF;
  }
  
  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  Serial.println(Hex_to_String(key.keyByte, MFRC522::MF_KEY_SIZE);

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
 
  delay(5000);

 randomSeed(analogRead(0));
}



void loop() {
    //generate a random 4-digit pin
    Pincode = random(1000, 10000);
    //send this pin to server via websocket protocol
    if (Wifi.status() == WL_CONNECTED)){
        HTTPClient http;
        String server = serverName + "?" + String(Pincode);
        http.begin(server.c_str());
        while (data.length() ==0){
          int httpResponseCode = http.GET();
          if (httpResponseCode > 0){
            String data = http.getString();
            Serial.println(payload);
          }
          else {
            Serial.print("Error code: ");
            Serial.println(httpResponseCode);
          }
          
          if (valid_card()){
            http.begin((serverName + "?" + String(Hex_to_string(rfid.uid.uidByte)).c_str());
          }
        }
        char *temp;
        if (data.length() > 0){  //data == "/Name/status/"
          temp =strtok(data, "/");
          //print Hi, Name
          temp=strtok(data, "/");
          //shift to 2nd line and print the status
        }
        data = ""; //clear data
    }
    else{
        Serial.println("Client is disconnected.");
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
    String RFID = "";
    for (byte i = 0; i < bufferSize; i++) {
        RFID.concat(String(buffer[i] < 0x10 ? " 0" : " "));
        RFID.concat(String(buffer[i], HEX));
    }
    return RFID;
}