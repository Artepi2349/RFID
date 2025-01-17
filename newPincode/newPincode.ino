//#include <Key.h>
#include <Keypad.h>
#include <Servo.h>
#include <NewPing.h> // Библиотека для работы с датчиком расстояния

#define servoPin 4

#define soundPin 13

#define PIN_TRIG 3 // Подключение пина TRIG датчика расстояния
#define PIN_ECHO 2 // Подключение пина ECHO датчика расстояния
#define MAX_DISTANCE 200 // Константа для определения максимального расстояния, которое мы будем считать корректным

const byte ROWS = 4; // число строк клавиатуры
const byte COLS = 3; // число столбцов клавиатуры

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[ROWS] = {12, 11, 10, 9}; // к каким выводам подключаем управление строками
byte colPins[COLS] = {8, 7, 6}; // к каким выводам подключаем управление столбцами

char pass[4] = {'7', '3', '1', '5'}; // верный пароль
char buttons[5]; // массив нажатых кнопок
int k = 0; // счетчик нажатий
unsigned int distance; // Переменная для хранения расстояния от стойки калитки


Servo Servo1;

Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// Создаем объект, методами которого будем затем пользоваться для получения расстояния
// В качестве параметров передаем номера пинов, к которым подключены выходы ECHO и TRIG датчика
NewPing sonar(PIN_TRIG, PIN_ECHO, MAX_DISTANCE);

void setup() {
  Serial.begin(9600);
  Servo1.attach(servoPin);
  pinMode(soundPin, OUTPUT);
}

void loop() {
  char customKey = customKeypad.getKey();
  if (customKey) {
    buttons[k] = customKey; // сохраняем значение кнопки в массиве
    Serial.print(customKey);
    k = k + 1; // увеличиваем счётчик нажатий на 1
    digitalWrite(soundPin, 50);
    delay(20);
    digitalWrite(soundPin, 0);
    if (k == 4) { 
      if (buttons[0] == pass[0] && buttons[1] == pass[1] && buttons[2] == pass[2] && buttons[3] == pass[3]) { 
        Serial.print("Access granted!"); // если пароль совпал
        k=0;
        Servo1.write(0);
        delay(2000);
        Servo1.write(90);
        unsigned int distance = 30;
        while (distance>25) {
          distance = sonar.ping_cm();
          Serial.print(distance);
          delay(500);
        }
        while (distance<25) {
          distance = sonar.ping_cm();
          Serial.print(distance);
          delay(500);
        }
        Servo1.write(180);
        delay(2000);
        Servo1.write(90);
      }
      else {
        Serial.print("Access denied!"); // если пароль не верен
        k=0;
      }
    }
  }
}