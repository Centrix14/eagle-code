// Программа для каретки
// 29.03.2023

#include <SPI.h>      // Подключаем библиотеку для работы с SPI-интерфейсом
#include <nRF24L01.h> // Подключаем файл конфигурации из библиотеки RF24
#include <RF24.h>     // Подключаем библиотеку для работа для работы с модулем NRF24L01

#define PIN_CE  10  // Номер пина Arduino, к которому подключен вывод CE радиомодуля
#define PIN_CSN 9   // Номер пина Arduino, к которому подключен вывод CSN радиомодуля

#define PIN_REVERSE_ENABLE 3 // Пин 1 обратного хода
#define PIN_REVERSE_ON     4 // Пин 2 обратного хода
#define PIN_DIRECT_ENABLE  5 // Пин прямого хода

#define PIPE1       0xB4B5B6B7F1 // Труба по которой пульт общается с кареткой
#define AVG         117          // Среднее значение джойстика
#define SENSETIVITY 20           // Чувствительность джойстика

#define REVERSE_DELAY 1000 // Задержка реверса

RF24 radio(PIN_CE, PIN_CSN); // Создаём объект radio с указанием выводов CE и CSN

uint8_t data = 0;

uint8_t is_reverse = 0;     // Нужно ли сделать реверс
uint8_t is_reverse_old = 0; // Старое значение реверса
uint8_t is_move = 1;        // Нужно ли двигаться

unsigned long last_move_time = 0;

void setup() {  
  setup_radio();

  pinMode(PIN_DIRECT_ENABLE, OUTPUT);
  pinMode(PIN_REVERSE_ENABLE, OUTPUT);
  pinMode(PIN_REVERSE_ON, OUTPUT);

  Serial.begin(9600);
}

void loop() {  
  if (radio.available()) { // Если в буфер приёмника поступили данные
    radio.read(&data, sizeof(data));
    
    log_dbg();

    if (abs(AVG - data) > SENSETIVITY)
      is_move = 1;
    else
      is_move = 0;
    
    if (AVG - data > SENSETIVITY)
      is_reverse = 1;
    else
      is_reverse = 0;

    if (is_reverse != is_reverse_old)
      take_a_break();

    movement(is_reverse, is_move);
    
    is_reverse_old = is_reverse;
  }

  last_move_time = millis();
}

void setup_radio() {
  radio.begin();                   // Инициализация модуля NRF24L01
  radio.setChannel(9);             // Обмен данными будет вестись на пятом канале (2,405 ГГц)
  radio.setDataRate(RF24_1MBPS);   // Скорость обмена данными 1 Мбит/сек
  radio.setPALevel(RF24_PA_HIGH);  // Выбираем высокую мощность передатчика (-6dBm)
  radio.openReadingPipe(1, PIPE1); // Открываем трубу ID передатчика
  radio.startListening();          // Начинаем прослушивать открываемую трубу
}

void log_dbg() {
  Serial.println(String(is_move) + String(is_reverse));
}

void take_a_break() {
  //Serial.println("Break start");
  // Выключаем двигатель и даём схеме отдохнуть
  digitalWrite(PIN_DIRECT_ENABLE, LOW);
  digitalWrite(PIN_REVERSE_ENABLE, LOW);
  digitalWrite(PIN_REVERSE_ON, LOW);
  delay(REVERSE_DELAY);
  //Serial.println("Break end");
}

void movement(uint8_t is_reverse, uint8_t is_move) {
  if (is_move && !is_reverse) {
    digitalWrite(PIN_REVERSE_ENABLE, LOW);
    digitalWrite(PIN_DIRECT_ENABLE, HIGH);
    digitalWrite(PIN_REVERSE_ON, LOW);
  }
  else if (is_move && is_reverse) {
    digitalWrite(PIN_REVERSE_ENABLE, HIGH);
    digitalWrite(PIN_DIRECT_ENABLE, LOW);
    digitalWrite(PIN_REVERSE_ON, HIGH);
  }
  else {
    digitalWrite(PIN_REVERSE_ENABLE, LOW);
    digitalWrite(PIN_DIRECT_ENABLE, LOW);
    digitalWrite(PIN_REVERSE_ON, LOW);
  }
}
