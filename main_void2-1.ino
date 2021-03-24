/*
  Стенд для испытаний ВМГ
  Эдвин Денисов-Элерс
  denisovelers@inbox.ru
  Кванториум Сампо, Аэроквантум
  Петрозаводск, 2020-2021
*/

#include "Servo.h"                        // библиотека для работы с мотором
#include "Wire.h"                        // I2c библиотека
#include "HX711.h"                      // библиотека для тензодатчика
#include "LiquidCrystal_I2C.h"         // библиотека для работы с дисплеем

LiquidCrystal_I2C lcd(0x27, 16, 2);   // устанавливаем дисплей

#define voltagePin A0               // вольтметр подключен к пину A0
#define currentPin A1              // амперметр подключен к пину A1
float voltageScale = 19.05;       // переменная для хранения коэффициента напряжения
float voltage;                   // переменная для хранения значения напряжения
float current;                  // переменная для хранения значения тока

//HX711 start
#define DT  3                             // подключаем DT к 3 пину
#define SCK 2                            // подключаем SCK ко 2 пину
HX711 scale;                             // создаём объект scale
float calibration_factor = 7.16;        // вводим калибровочный коэффициент 0.82
float units;                           // задаём переменную для измерений в граммах
float ounces;                         // задаём переменную для измерений в унциях
// HX711 end

//PWM start
#define autotest_pin 9        // пин подключения кнопки автоматического тестирования
Servo motor;                 // создаем объект motor
int mot_pin = 7;            // пин подключения мотора
int max_pwm = 2000;        // максимальное значение ШИМ 2 мс
int min_pwm = 1000;       // минимальное значение ШИМ 1 мс
int speed = min_pwm;     // делаем начальное значение скорости равным минимальному ШИМ
//PWM end

String string2;   // строка для вывода в порт

void setup() {
  pinMode(autotest_pin, INPUT_PULLUP);
  pinMode(voltagePin, INPUT);
  pinMode(currentPin, INPUT);
  
  Serial.begin(9600);  // открываем последовательный порт
  
  lcd.init();          // инициализация дисплея
  lcd.backlight();    // Включаем подсветку дисплея



  // HX711 start
  scale.begin(DT, SCK);                       // инициируем работу с датчиком
  scale.set_scale();                         // выполняем измерение значения без калибровочного коэффициента
  scale.tare();                             // сбрасываем значения веса на датчике в 0
  scale.set_scale(calibration_factor);     // устанавливаем калибровочный коэффициент
  // HX711 end

  // PWM start
  motor.attach(mot_pin);                       // инициализация мотора
  /*delay(1000);                              // калибровка
    motor.writeMicroseconds(max_pwm);
    delay(2000);
    motor.writeMicroseconds(min_pwm);
    delay(4000);*/  
  // PWM end

}

void loop() {
  pwm();
  disp();
  params();
}

void pwm() {
  while ((!digitalRead(autotest_pin) == 1)) {                       // пока кнопка нажата..
    thrust_meter();                                                // измеряем тягу
    float voltage = ((analogRead(voltagePin)) / (voltageScale));  // обновляем значение напряжения
    float current = analogRead(currentPin);                      // обновляем значение тока
    disp();                                                     // выводим на дисплей

    if (speed < max_pwm) {                                      // если ШИМ меньше максимального..
      speed += 50;                                             // прибавляем 50 микросекунд
      motor.writeMicroseconds(speed);                         // отправляем значение на мотор
      string2 = String(ounces * 10) + String(";") + String(speed) + String(";") + String(voltage) + String(";") + String(current / 10) + String(";");
      //генерируем строку с данными для вывода в порт
      Serial.println(string2);      // выводим
      delay(3000);                 // ждем 3 секунды
    } else {                                     // если ШИМ максимальный..
      speed = min_pwm;                          // сбрасываем значение ШИМ на минимальное
      motor.writeMicroseconds(speed);          // отправляем значение на мотор
      break;                                  // выходим из цикла
    }
  }

  if ((!digitalRead(autotest_pin) == 0)) {    // если кнопка не нажата..
    speed = min_pwm;                         // оставляем ШИМ минимальным
    motor.writeMicroseconds(speed);         // отправляем значение на мотор
  }

  motor.writeMicroseconds(speed);           // отправляем значение на мотор
  delay(5000);                             // у нас есть 5 секунд, чтобы отпустить кнопку, иначе тест запустится заново

}

void params() {                                                  // фунция для тока и напряжения
  float voltage = ((analogRead(voltagePin)) / (voltageScale));  // переменная для хранения значения напряжения
  float current = analogRead(currentPin);                      // переменная для хранения значения тока
}

void thrust_meter() {
  for (int i = 0; i < 10; i ++) {                              // усредняем показания, считав значения датчика 10 раз
    units = + scale.get_units(), 10;                          // суммируем показания 10 замеров
  }
  units = units / 10;                                          // усредняем показания, разделив сумму значений на 10
  ounces = units * 0.035274;                                  // переводим вес из унций в граммы
  if (ounces < 0) {                                          // если получается отрицательное значение..
    ounces = 0;                                             // приравниваем его к нулю
  }
}

void disp() {
  float voltage = ((analogRead(voltagePin)) / (voltageScale));  // обновляем значение напряжения
  float current = analogRead(currentPin);                      // обновляем значение тока
  lcd.setCursor(13, 0);
  lcd.print("    ");              // пробелы для очистки экрана
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.setCursor(2, 0);
  lcd.print("       ");           // пробелы для очистки экрана
  lcd.setCursor(2, 0);
  lcd.print(ounces * 10);         // выводим на дисплей тягу
  lcd.setCursor(9, 0);
  lcd.print("P:");
  lcd.setCursor(11, 0);
  lcd.print(speed + 50);          // выводим на дисплей ШИМ
  lcd.setCursor(0, 1);
  lcd.print("U:");
  lcd.setCursor(2, 1);
  lcd.print(voltage);             // выводим на дисплей напряжение
  lcd.setCursor(9, 1);
  lcd.print("I:");
  lcd.setCursor(11, 1);
  lcd.print(current / 10);        // выводим на дисплей ток
}
