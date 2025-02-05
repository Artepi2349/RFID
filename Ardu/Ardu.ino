#include <RFID.h> // Библиотека для работы с RFID-считывателем
#include <SPI.h> // Библиотека для обмена данными между платой Arduino и подключенными устройствами
#include <Adafruit_GFX.h> // Библиотека для ядра графики
#include <Adafruit_SSD1306.h> // Библиотека драйверов для дисплеев 128x64 и 128x32
#include <Stepper.h> // Библиотека для работы с шаговым мотором
#include <AccelStepper.h>
#include <NewPing.h> // Библиотека для работы с датчиком расстояния

#define SS_PIN 9 // Подключение пина SDA для RFID-считвателя
#define RST_PIN 8 // Подключение пина RESET для RFID-считвателя

// Подключение пинов RGB-светодиода
#define blueLED 2 
#define greenLED 3
#define redLED 4

// Подключение пинов кнопок
#define buttonPinEmergencyShutdown 5
#define buttonPinClose 6
#define buttonPinOpen 7

#define OLED_RESET 20  // Подключение пина SDA OLED-экрана

// Количество шагов на оборот внутреннего двигателя в 4- х ступенчатом режиме
#define STEPS_PER_MOTOR_REVOLUTION 200     
// Число ступеней на оборот выходного вала (=редуктор; 2048 ступеней)
#define STEPS_PER_OUTPUT_REVOLUTION 800

#define PIN_TRIG 43 // Подключение пина TRIG датчика расстояния
#define PIN_ECHO 41 // Подключение пина ECHO датчика расстояния
#define MAX_DISTANCE 200 // Константа для определения максимального расстояния, которое мы будем считать корректным

// Пины для управления шаговым мотором
#define pinStep 11
#define pinDir 10


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
Stepper stepper(STEPS_PER_MOTOR_REVOLUTION, pinDir, pinStep);
AccelStepper mystepper(STEPS_PER_MOTOR_REVOLUTION, pinDir, pinStep);

String access = "denied"; // Переменная для РЗД
String uidString = ""; // Переменная для хранения UID, получаемого с RFID-считывателя
String lastUID = ""; // Переменная для хранения UID последнего пропущенного на территорию пользователя

int Steps2Take  =  STEPS_PER_OUTPUT_REVOLUTION;  // Повернуть CW 1 оборот

// Переменные для хранения состояния кнопок
int buttonStateOpen = digitalRead(buttonPinOpen);
int buttonStateClose = digitalRead(buttonPinClose);
int buttonStateEmergencyShutdown = digitalRead(buttonPinEmergencyShutdown);

bool close = true;
bool stepFlag = false;

void setup() {
  
  Serial.begin(9600);
  SPI.begin(); // Инициировать SPI
  RC522.init(); // Инициировать считыватель

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // Инициализация экрана

  display.cp437(true); // для русского текста

  
  closeState();

  pinMode(blueLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  stepper.setSpeed(1000);

  pinMode(buttonPinOpen, INPUT_PULLUP);
  pinMode(buttonPinClose, INPUT_PULLUP);
  pinMode(buttonPinEmergencyShutdown, INPUT_PULLUP);

}

void loop() {
  buttonStateEmergencyShutdown = digitalRead(buttonPinEmergencyShutdown); // Проверка нажатия кнопки аварийного выключения
  if (buttonStateEmergencyShutdown == 0) {
    offLED();
    clearDisplay();
    emergencyShutdown();
  }
  if(RC522.isCard()) { // Если карта считана
    readRFID();
    delay(50);
    if (Serial.available() > 0) { // Считывание ответа с Serial порта
      access = Serial.readString();
      oledOut("Проверка");
    }
    if (access == "granted") {
      access = "denied";
      oledOut("Разрешён");
      GLED();
      delay(1000);
      openGatesStep();
      buttonStateOpen = 1;
      GLED();
      unsigned int distance = 30;
      while (distance>25) { // Ожидание прохода человека в рамку
        distance = sonar.ping_cm();
        delay(500);
      }
      while (distance<25) { // Ожидание выхода человека из рамки
        distance = sonar.ping_cm();
        delay(500);
      }
      while (close == true) {
        close = false;
        closeGatesStep();
      }
      closeState();                   
      buttonStateClose = 1;
      buttonStateOpen = 1;
      close=true;
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
    if (access == "secondusage") {
      RLED();
      oledOut("Запрещено");
      delay(2000);
      closeState();
    }
  }
}

void yield() {
  buttonStateEmergencyShutdown = digitalRead(buttonPinEmergencyShutdown);
  if (buttonStateEmergencyShutdown == 0) {
    offLED();
    clearDisplay();
    //emergencyShutdown();
  }
}

void openGatesStep() { // Функция для открытия ворот
  while (buttonStateOpen != 0) {
    buttonStateEmergencyShutdown = digitalRead(buttonPinEmergencyShutdown);
    blink();
    if (buttonStateEmergencyShutdown == 0) {
      offLED();
      clearDisplay(); 
      emergencyShutdown();
    }
    stepper.step(Steps2Take);
    buttonStateOpen = digitalRead(buttonPinOpen);
  }  
}

void closeGatesStep() { // Функция для закрытия ворот
  while (buttonStateClose != 0) {
    buttonStateEmergencyShutdown = digitalRead(buttonPinEmergencyShutdown);
    blink();
    if (buttonStateEmergencyShutdown == 0) {
      offLED();
      clearDisplay();
      emergencyShutdown();
    }
    stepper.step(-Steps2Take);
    buttonStateClose = digitalRead(buttonPinClose);
    unsigned int distance = 70;
    distance = sonar.ping_cm();
    if (distance < 25) {
      mystepper.stop();
      unsigned int distance = 30;
      while (distance>25) {
        distance = sonar.ping_cm();
        delay(500);
      }
      while (distance<25) {
        distance = sonar.ping_cm();
        delay(500);
      }
      buttonStateClose = 1;
      // Часть кода, которая оказалась ненужна по тз, но она хороша(Если её раскомментировать, то ворота снова откроются и будут ждать проезда объекта)
      /*
      while (buttonStateOpen != 0) {
        stepper.step(Steps2Take);
        buttonStateOpen = digitalRead(buttonPinOpen);
        close = true;
      }
      buttonStateOpen = 1;
      */
    }
  }
}

void RLED() { // Функция для включения красного цвета светодиода
  digitalWrite(redLED, HIGH);
  digitalWrite(blueLED, LOW);
  digitalWrite(greenLED, LOW);
}

void BLED() { // Функция для включения синего цвета светодиода
  digitalWrite(redLED, LOW);
  digitalWrite(blueLED, HIGH);
  digitalWrite(greenLED, LOW);
}

void GLED() { // Функция для включения зелёного цвета светодиода
  digitalWrite(redLED, LOW);
  digitalWrite(blueLED, LOW);
  digitalWrite(greenLED, HIGH);
}

void WLED() { // Функция для включения белого цвета светодиода
  digitalWrite(redLED, HIGH);
  digitalWrite(blueLED, HIGH);
  digitalWrite(greenLED, HIGH);
}

void offLED() { // Функция для выключения светодиода
  digitalWrite(redLED, LOW);
  digitalWrite(blueLED, LOW);
  digitalWrite(greenLED, LOW);
}

void blink() { // Функция для мигания светодиодом
  stepFlag = !stepFlag;
    if (stepFlag == true) {
      WLED();
    }
    if (stepFlag == false) {
      offLED();
    }
}

void readRFID() { // Функция для считывания метки и отправки на микрокомпьютер
  
  RC522.readCardSerial();
  uidString = String(RC522.serNum[0] + String("-") + RC522.serNum[1] + String("-") + RC522.serNum[2] + String("-") + RC522.serNum[3]);
  
  if (uidString !=  lastUID) {
    Serial.println(uidString);
    lastUID = uidString;
  }
}

void oledOut(String s) { // Функция для вывода на экран
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

void clearDisplay() { // Функция для очистки экрана
  display.clearDisplay();
  display.display();
}

void closeState() { // Функция для начального состояния ворот
  RLED();
  display.clearDisplay();
  display.display();
  display.setTextColor(WHITE); // or BLACK);
  display.setTextSize(2);
  display.setCursor(10,8); 
  display.print(utf8rus("Закрыто")); //максимум 9 символов в строке с размером текста 2
  display.display();
}

void emergencyShutdown() { // Функция для выхода из программы 
  exit(0);
}

String utf8rus(String source) { // Функция для перевода в русский текст
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