//#include <Key.h>
#include <Keypad.h>
#include <Servo.h>
#include <NewPing.h> // Библиотека для работы с датчиком расстояния

#define servoPin 3

#define soundPin 5

#define buttonPin 2

#define solenoidPin 22

#define PIN_TRIG 43 // Подключение пина TRIG датчика расстояния
#define PIN_ECHO 41 // Подключение пина ECHO датчика расстояния
#define MAX_DISTANCE 200 // Константа для определения максимального расстояния, которое мы будем считать корректным

const byte ROWS = 4; // число строк клавиатуры
const byte COLS = 3; // число столбцов клавиатуры

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[ROWS] = {6, 7, 8, 9}; // к каким выводам подключаем управление строками
byte colPins[COLS] = {10, 11, 12}; // к каким выводам подключаем управление столбцами

char pass[4] = {'7', '3', '1', '5'}; // верный пароль
char buttons[4]; // массив нажатых кнопок
int k = 0; // счетчик нажатий
unsigned int distance; // Переменная для хранения расстояния от стойки калитки
int buttonState = digitalRead(buttonPin);

Servo Servo1;

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// Создаем объект, методами которого будем затем пользоваться для получения расстояния
// В качестве параметров передаем номера пинов, к которым подключены выходы ECHO и TRIG датчика
NewPing sonar(PIN_TRIG, PIN_ECHO, MAX_DISTANCE);

void setup() {
  Serial.begin(9600);
  Servo1.attach(servoPin);
  pinMode(soundPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(solenoidPin, OUTPUT);
  digitalWrite(solenoidPin, LOW);
}

void loop() {
  buttonState = digitalRead(buttonPin);
  if (buttonState == 0) {
    exit(0);
  }
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
        digitalWrite(soundPin, 50);
        delay(1000);
        digitalWrite(soundPin, 0);
        k=0;
        digitalWrite(solenoidPin, HIGH);
        delay(500);
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
        delay(500);
        digitalWrite(solenoidPin, LOW);
      }
      else {
        Serial.print("Access denied!"); // если пароль не верен
        digitalWrite(soundPin, 50);
        delay(1000);
        digitalWrite(soundPin, 0);
        delay(500);
        digitalWrite(soundPin, 50);
        delay(1000);
        digitalWrite(soundPin, 0);
        delay(500);
        digitalWrite(soundPin, 50);
        delay(1000);
        digitalWrite(soundPin, 0);
        k=0;
      }
    }
  }
}
void yield() {
  buttonState = digitalRead(buttonPin);
  if (buttonState == 0) {
    exit(0);
  }
}