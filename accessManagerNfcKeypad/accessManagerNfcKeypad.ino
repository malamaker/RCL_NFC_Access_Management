#include <Key.h>
#include <Keypad.h>
#include <SPI.h>
#include <MFRC522.h>

/*
  We need to define MFRC522's pins and create instance
  Pin layout should be as follows (on Teensy 2.0):
  MOSI: Pin 2
  MISO: Pin 3
  SCK : Pin 1
  SDA : Pin 0
  RST : Pin 7
 */

#define SS_PIN 10
#define RST_PIN 9

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.

const byte numRows= 4; //number of rows on the keypad
const byte numCols= 3; //number of columns on the keypad

char keymap[numRows][numCols]= 
{
{'1', '2', '3'}, 
{'4', '5', '6'}, 
{'7', '8', '9'},
{'*', '0', '#'}
};

byte rowPins[numRows] = {8,7,6,5}; //Rows 0 to 3
byte colPins[numCols]= {4,3,2}; //Columns 0 to 2

Keypad myKeypad= Keypad(makeKeymap(keymap), rowPins, colPins, numRows, numCols);

byte readCard[4];
String UID = "";
String pin = "";
int pinCount = 300;
int pinCountMax = 300;
int successPin; 
int successRead;   // Variable integer to keep if we have Successful Read from Reader

void setup() {
  //Initialize
  Serial.begin(9600);   // Initialize serial communications with PC
  Serial.println(F("Access Manager NFC Keypad Module v1.0"));   // For debugging purposes
  
  SPI.begin();
  mfrc522.PCD_Init();
  ShowReaderDetails();  // Show details of PCD - MFRC522 Card Reader details

  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
  mfrc522.PCD_AntennaOn();
}

void loop() {
  do {
    successRead = getID();
    
    if (pinCount < pinCountMax) {
      pinCount = pinCount + 1;
      if (pinCount == pinCountMax) {
        Serial.println(UID + "|" + "ERROR");
      }
    }

    char keypressed = myKeypad.getKey();

    if (keypressed != NO_KEY && pinCount < pinCountMax) {
      if (keypressed == '#') {
        Serial.println(UID + "|" + pin);
        pinCount = pinCountMax;
      } else {
        pin = pin + keypressed;
      }
    }
  }
  while (!successRead);
}

void ShowReaderDetails() {
  // Get the MFRC522 software version
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print(F("MFRC522 Software Version: 0x"));
  Serial.print(v, HEX);
  if (v == 0x91)
    Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    Serial.print(F(" = v2.0"));
  else
    Serial.print(F(" (unknown)"));
  Serial.println("");
  // When 0x00 or 0xFF is returned, communication probably failed
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
    while (true); // do not go further
  }
}

int getID() {
  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) {   //Since a PICC placed get Serial and continue
    return 0;
  }
  // There are Mifare PICCs which have 4 byte or 7 byte UID care if you use 7 byte PICC
  // I think we should assume every PICC as they have 4 byte UID
  // Until we support 7 byte PICCs
  Serial.println(F("NFC Scanned:"));
  UID = "";
  for (int i = 0; i < 4; i++) {  //
    readCard[i] = mfrc522.uid.uidByte[i];
    UID = UID + String(readCard[i], HEX);
  }
  mfrc522.PICC_HaltA(); // Stop reading
  UID.toUpperCase();
  pinCount = 0;
  pin = "";
  return 1;
}

