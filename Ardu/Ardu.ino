#include <RFID.h> // Библиотека для работы с RFID-считывателем
#include <SPI.h> // Библиотека для обмена данными между платой Arduino и подключенными устройствами
#include <Adafruit_GFX.h> // Библиотека для ядра графики
#include <Adafruit_SSD1306.h> // Библиотека драйверов для дисплеев 128x64 и 128x32
#include <Stepper.h> // Библиотека для работы с шаговым мотором
#include <NewPing.h> // Библиотека для работы с датчиком расстояния

#define SS_PIN 9 // Подключение пина SDA для RFID-считвателя
#define RST_PIN 8 // Подключение пина RESET для RFID-считвателя

// Подключение пинов RGB-светодиода
#define blueLED 2 
#define greenLED 3
#define redLED 4

#define OLED_RESET 20  // Подключение пина SDA OLED-экрана

// Количество шагов на оборот внутреннего двигателя в 4- х ступенчатом режиме
#define STEPS_PER_MOTOR_REVOLUTION 32     
// Число ступеней на оборот выходного вала (=редуктор; 2048 ступеней)
#define STEPS_PER_OUTPUT_REVOLUTION 32 * 64

#define PIN_TRIG 43 // Подключение пина TRIG датчика расстояния
#define PIN_ECHO 41 // Подключение пина ECHO датчика расстояния
#define MAX_DISTANCE 200 // Константа для определения максимального расстояния, которое мы будем считать корректным

// Создаем объект, методами которого будем затем пользоваться для получения расстояния
// В качестве параметров передаем номера пинов, к которым подключены выходы ECHO и TRIG датчика
NewPing sonar(PIN_TRIG, PIN_ECHO, MAX_DISTANCE);
 
// Создаем объект, методами которого будем затем пользоваться для получения UID
// В качестве параметров передаем номера пинов, к которым подключены выходы SDA и RESET RFID-считывателя
RFID RC522(SS_PIN, RST_PIN); 

// Создаем объект, методами которого будем затем пользоваться для вывода сообщений на экран
// В качестве параметров передаем номер пина, к которому подключён выход SDA OLED-экрана
Adafruit_SSD1306 display(OLED_RESET);

// Создаем объект, методами которого будем затем пользоваться для открытия калитки
// В качестве параметров передаем количество шагов и номера пинов, к которым подключены выходы мотора
Stepper stepper(STEPS_PER_MOTOR_REVOLUTION, 10, 12, 11, 13);

String access = "denied"; // Переменная для РЗД
String uidString = ""; // Переменная для хранения UID, получаемого с RFID-считывателя
String lastUID = ""; // Переменная для хранения UID последнего пропущенного на территорию пользователя

int Steps2Take  =  STEPS_PER_OUTPUT_REVOLUTION / 2;  // Повернуть CW 1/2 оборота

                 

void setup() {
  
  Serial.begin(9600);
  SPI.begin(); // Init SPI bus
  RC522.init(); // Init MFRC522 

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)

  display.cp437(true); // для русского текста

  // Clear the buffer.
  closeState();

  pinMode(blueLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  stepper.setSpeed(1000);

}

void loop() {
  if(RC522.isCard()) {
    readRFID();
    delay(50);
    if (Serial.available() > 0) {
      access = Serial.readString();
      oledOut("Проверка");
    }
    if (access == "granted") {
      access = "denied";
      oledOut("Разрешён");
      GLED();
      openGatesStep();
      unsigned int distance = 70;
      while (distance>50) {
      distance = sonar.ping_cm();
      delay(500);
      }
      while (distance<50) {
        distance = sonar.ping_cm();
        delay(500);
      }
      closeGatesStep();
      closeState(); 
    }
    if (access == "dateout") {
      BLED();
      oledOut("Истёк");
      delay(2000);
      closeState();
    }
    if (access == "notfind") {
      RLED();
      oledOut("Нет в бд");
      delay(2000);
      closeState();
    }
  }
}

void openGatesStep() {   
  stepper.step(Steps2Take);
}

void closeGatesStep() {
  stepper.step(-Steps2Take); 
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
  
  RC522.readCardSerial();
  uidString = String(RC522.serNum[0] + String("-") + RC522.serNum[1] + String("-") + RC522.serNum[2] + String("-") + RC522.serNum[3]);
  
  if (uidString !=  lastUID) {
    Serial.println(uidString);
    lastUID = uidString;
  }
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

void closeState() {
  RLED();
  display.clearDisplay();
  display.display();
  display.setTextColor(WHITE); // or BLACK);
  display.setTextSize(2);
  display.setCursor(10,8); 
  display.print(utf8rus("Закрыто")); //максимум 9 символов в строке с размером текста 2
  display.display();
}

String utf8rus(String source) {
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