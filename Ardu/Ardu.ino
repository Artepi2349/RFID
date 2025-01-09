#include <MFRC522.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Stepper.h>

#define SS_PIN 10
#define RST_PIN 9

#define blueLED 2
#define greenLED 3
#define redLED 4

#define OLED_RESET 4

#define STEPS 32
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

Adafruit_SSD1306 display(OLED_RESET);

Stepper stepper(STEPS, 5, 7, 6, 8);

int code[] = {225,243,51,3}; //This is the stored UID
int codeRead = 0;
String access = "denied";
String uidString = "";
String lastUID = "";

void setup() {
  
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)

  display.cp437(true); // для русского текста

  // Clear the buffer.
  startWork();

  pinMode(blueLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  stepper.setSpeed(200);

}

void loop() {
  if(  rfid.PICC_IsNewCardPresent())
  {
      readRFID();

      if (Serial.available() > 0) {
        access = Serial.readString();
        oledOut("Проверка");
      }
    
      if (access == "granted") {
        access = "denied";
        oledOut("Разрешён");
        GLED();
        openGatesStep();
        delay(2000);
        startWork();
      }
      if (access == "dateout") {
        BLED();
        oledOut("Истёк");
        delay(2000);
        startWork();
      }
      if (access == "notfind") {
        RLED();
        oledOut("Нет в бд");
        delay(2000);
        startWork();
      }
  }
  
  delay(100);
}

void openGatesStep() {  
  stepper.step(1000);
}

void RLED() {
  digitalWrite(redLED, HIGH);
  digitalWrite(blueLED, LOW);
  digitalWrite(greenLED, LOW);
}

void BLED() {
  digitalWrite(redLED, LOW);
  digitalWrite(blueLED, HIGH);
  digitalWrite(greenLED, LOW);
}

void GLED() {
  digitalWrite(redLED, LOW);
  digitalWrite(blueLED, LOW);
  digitalWrite(greenLED, HIGH);
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

void oledOut(String s) {
    // Clear the buffer.
  display.clearDisplay();
  display.display();
  display.setTextColor(WHITE); // or BLACK);
  display.setTextSize(2);
  display.setCursor(10,8); 
  display.print(utf8rus(s)); //максимум 9 символов в строке с размером текста 2
  display.display();

  display.display();
  display.setTextColor(WHITE); // or BLACK);
  display.setTextSize(1);
  display.setCursor(10,0); 
  display.print("UID: " + uidString); //максимум 9 символов в строке с размером текста 2
  display.display();
}

void startWork() {
  RLED();
  display.clearDisplay();
  display.display();
  display.setTextColor(WHITE); // or BLACK);
  display.setTextSize(2);
  display.setCursor(10,8); 
  display.print(utf8rus("Закрыто")); //максимум 9 символов в строке с размером текста 2
  display.display();
  stepper.step(-1000);
}

String utf8rus(String source)
{
  int i,k;
  String target;
  unsigned char n;
  char m[2] = { '0', '\0' };
  k = source.length(); i = 0;
  while (i < k) {
    n = source[i]; i++;
    if (n >= 0xC0) {
      switch (n) {
        case 0xD0: {
          n = source[i]; i++;
          if (n == 0x81) { n = 0xA8; break; }
          if (n >= 0x90 && n <= 0xBF) n = n + 0x30;
          break;
        }
        case 0xD1: {
          n = source[i]; i++;
          if (n == 0x91) { n = 0xB8; break; }
          if (n >= 0x80 && n <= 0x8F) n = n + 0x70;
          break;
        }
      }
    }
    m[0] = n; target = target + String(m);
  }
return target;
}