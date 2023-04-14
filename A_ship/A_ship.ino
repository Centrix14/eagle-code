 // Программа для парома
// 24.03.2023

#include <SPI.h>  // Подключаем библиотеку для работы с SPI-интерфейсом
#include <nRF24L01.h> // Подключаем файл конфигурации из библиотеки RF24
#include <RF24.h> // Подключаем библиотеку для работа для работы с модулем NRF24L01
#include <Servo.h> // Библиотека для работы с servo-мотором

#define PIN_CE  10  // Номер пина Arduino, к которому подключен вывод CE радиомодуля
#define PIN_CSN 9 // Номер пина Arduino, к которому подключен вывод CSN радиомодуля

#define PIN_ENGINE 12 // Пин реле двигателя
#define PIN_REVERSE 13 // Пин реле реверса

#define PIN_WHEEL 6 // Пин руля
#define PIN_VELOCITY 5 // Пин управления скоростью

#define REVERSE_DELAY 1000 // Задержка реверса

RF24 radio(PIN_CE, PIN_CSN); // Создаём объект radio с указанием выводов CE и CSN
Servo wheel_servo; // Серва руля
Servo engine;

// Данные для передачи
// Первый элемент - состояние кнопки реверса
// Второй элемент - состояние потенциометра скорости
// Третий элемент - состояние потенциометра апарели
// Третий элемент - состояние джойстика руля

uint8_t control_data[] = {0, 0, 0, 0};

uint8_t current_reverse_state = 0;

void setup() {  
  wheel_servo.attach(PIN_WHEEL);
  engine.attach(PIN_VELOCITY);
  
  setup_radio();
  
  Serial.begin(9600);
}

void loop() {  
  if (radio.available()) { // Если в буфер приёмника поступили данные
    radio.read(&control_data, sizeof(control_data));
    //log_dbg();
    apply_data(control_data);
  }

  analogWrite(6, control_data[1]);
}

void setup_radio() {
  radio.begin();  // Инициализация модуля NRF24L01
  radio.setChannel(9); // Обмен данными будет вестись на пятом канале (2,405 ГГц)
  radio.setDataRate (RF24_1MBPS); // Скорость обмена данными 1 Мбит/сек
  radio.setPALevel(RF24_PA_HIGH); // Выбираем высокую мощность передатчика (-6dBm)
  radio.openReadingPipe (1, 0x7878787878LL); // Открываем трубу ID передатчика
  radio.startListening(); // Начинаем прослушивать открываемую трубу
}

void log_dbg() {
  Serial.print(String(control_data[0]) + " ");
  Serial.print(String(control_data[1]) + " ");
  Serial.print(String(control_data[2]) + " ");
  Serial.println(control_data[3]);
}

void apply_data(uint8_t data[]) {
  uint8_t reverse_bttn = data[0];
  uint8_t velocity = data[1];
  uint8_t aparelle = data[2];
  uint8_t wheel = data[3];

  if (reverse_bttn != current_reverse_state) {
    reverse_move();
  }

  apply_wheel(wheel);
  apply_velocity(velocity);
}

void apply_wheel(uint8_t angle) {
  Serial.println(angle);
  wheel_servo.write(angle);
}

void apply_velocity(uint8_t velocity) {
  engine.write(velocity);
}

void reverse_move() {
  //Serial.println("Before: " + String(current_reverse_state));
  
  // Выключаем двигатель и даём схеме отдохнуть
  digitalWrite(PIN_ENGINE, LOW);
  digitalWrite(PIN_REVERSE, LOW);
  delay(REVERSE_DELAY);

  // Запускаем всё обратно
  current_reverse_state = !current_reverse_state;
  digitalWrite(PIN_REVERSE, current_reverse_state);
  digitalWrite(PIN_ENGINE, HIGH);
  
  //Serial.println("After: " + String(current_reverse_state));
}
