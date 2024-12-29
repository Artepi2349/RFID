#include <MFRC522.h>
#include <SPI.h>

#define SS_PIN 10
#define RST_PIN 9

#define SOUND_PIN 3
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

int code[] = {225,243,51,3}; //This is the stored UID
int codeRead = 0;
String access = "denied";
String uidString = "";
String lastUID = "";

void setup() {
  
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 
  
  pinMode (SOUND_PIN, OUTPUT); // Для динамика пин 3 в режиме работы выхода

}

void loop() {
  if(  rfid.PICC_IsNewCardPresent())
  {
      readRFID();

      if (Serial.available() > 0) {
          access = Serial.readString();
      }
    
      if (access == "granted")
      {
        openGates();
        access = "denied";
      }

  }
  
  delay(100);
}

void openGates()
{
    analogWrite(SOUND_PIN, 50); // включаем динамик
    //tone(SOUND_PIN, 600);
    delay(1000);
    //noTone(SOUND_PIN);
    analogWrite(SOUND_PIN, 0); // через секундк выключаем динамик
    delay(1000);
    
}

void readRFID()
{
  
  rfid.PICC_ReadCardSerial();
  uidString = String(rfid.uid.uidByte[0])+"-"+String(rfid.uid.uidByte[1])+"-"+String(rfid.uid.uidByte[2])+ "-"+String(rfid.uid.uidByte[3]);
  
  if (uidString !=  lastUID) {
    Serial.println(uidString);
    lastUID = uidString;
  }

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}

void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}

