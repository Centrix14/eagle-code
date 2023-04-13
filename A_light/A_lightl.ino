// Программа для фонаря
// 29.03.2023

// TODO
// Сейчас дальномер питается через цифровой пин, а не через vcc

#include <SPI.h>      // Подключаем библиотеку для работы с SPI-интерфейсом
#include <nRF24L01.h> // Подключаем файл конфигурации из библиотеки RF24
#include <RF24.h>     // Подключаем библиотеку для работа для работы с модулем NRF24L01
#include <NewPing.h>  // Библиотека для работы с SRF05

#define PIN_CE  10  // Номер пина Arduino, к которому подключен вывод CE радиомодуля
#define PIN_CSN 9   // Номер пина Arduino, к которому подключен вывод CSN радиомодуля

#define PIN_MAGNET 7 // Пин реле магнита

#define PIPE2 0xB4B5B6B7CD // Труба, по которой пульт общается с магнитом
#define PIPE3 0xB4B5B6B7A3 // Труба, по которой фонарь общается с кареткой

#define TRIG_PIN 3  // Пин trig
#define ECHO_PIN 4  // Эхо пин
#define MAX_DIST 30 // Пороговая дальность

#define DEAD_DIST 5 // «Дистинция смерти»
#define VOID_DIST 3 // «Пустая дистанция»

#define DBG

RF24 radio(PIN_CE, PIN_CSN); // Создаём объект radio с указанием выводов CE и CSN
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DIST);

uint8_t data = 0; // Состояние магнита

void setup() {  
  setup_radio();
  setup_sonar();
  
  pinMode(PIN_MAGNET, OUTPUT);
  
  Serial.begin(9600);
}

void loop() {
  digitalWrite(2, HIGH); // TODO! Этот вот колхоз убрать
  check_sonar();
  
  if (radio.available()) { // Если в буфер приёмника поступили данные
    radio.read(&data, sizeof(data));
    turn_magnet(data);
  }

#ifdef DBG
  log_dbg();
#endif
}

void setup_radio() {
  radio.begin();                   // Инициализация модуля NRF24L01
  radio.setChannel(9);             // Обмен данными будет вестись на пятом канале (2,405 ГГц)
  radio.setDataRate(RF24_1MBPS);   // Скорость обмена данными 1 Мбит/сек
  radio.setPALevel(RF24_PA_HIGH);  // Выбираем высокую мощность передатчика (-6dBm)
  radio.openReadingPipe(1, PIPE2); // Открываем трубу ID передатчика
  radio.startListening();          // Начинаем прослушивать открываемую трубу
}

void setup_sonar() {
  pinMode(2, OUTPUT);
}

void log_dbg() {
  Serial.println(sonar.ping_cm());
}

void turn_magnet(uint8_t value) {
  digitalWrite(PIN_MAGNET, value);
}

void check_sonar() {
  int dist = sonar.ping_cm();

  if (dist <= DEAD_DIST && dist >= VOID_DIST) {
    Serial.println("I send data.");
    warn_carriage();
  }
}

void warn_carriage() {
  RF24 carriage(PIN_CE, PIN_CSN);
  uint8_t msg = 1;

  carriage.begin();                  // Инициализация модуля NRF24L01
  carriage.setChannel(9);            // Обмен данными будет вестись на пятом канале (2,405 ГГц)
  carriage.setDataRate (RF24_1MBPS); // Скорость обмена данными 1 Мбит/сек
  carriage.setPALevel(RF24_PA_HIGH); // Выбираем высокую мощность передатчика (-6dBm)
  carriage.openWritingPipe(PIPE3);   // Открываем трубу с уникальным ID

  carriage.write(&msg, sizeof(uint8_t));
}
