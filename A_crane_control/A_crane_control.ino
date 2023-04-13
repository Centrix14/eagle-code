// Программа для пульта от крана
// 24.03.2023

#include <SPI.h>  // Подключаем библиотеку для работы с SPI-интерфейсом
#include <nRF24L01.h> // Подключаем файл конфигурации из библиотеки RF24
#include <RF24.h> // Подключаем библиотеку для работа для работы с модулем NRF24L01

#define PIN_CE  10  // Номер пина Arduino, к которому подключен вывод CE радиомодуля
#define PIN_CSN 9 // Номер пина Arduino, к которому подключен вывод CSN радиомодуля
#define PIN_BTTN 7 // Пин кнопки
#define PIN_BTTN_LED 6 // Пин светодиода кнопки
#define PIN_JOYSTICK1_X A4 // Пин джойстика упрвляющего кареткой
#define PIN_JOYSTICK2_Y A5 // Пин джойстика управляющего высотой магнита

#define CHANNEL 9
#define PIPE1 0xB4B5B6B7F1 // Труба для управления кареткой
#define PIPE2 0xB4B5B6B7CD // Труба для управления включения и выключения магнита

RF24 radio(PIN_CE, PIN_CSN); // Создаём объект radio с указанием выводов CE и CSN

// Текущее и предыдущее состояние кнопки
uint8_t bttn_current_state = 0;
uint8_t bttn_old_state = 0;

// Движение каретки
uint8_t carriage_move = 0;

// Движение магнита
uint8_t magnet_move = 0;

// Включение и выключение магнита
uint8_t magnet_control = 0;

// Результат отправки
bool is_sended = 0;

void setup() {
  setup_pins();
  setup_radio();

  Serial.begin(9600);
}

void loop() {
  handle_bttn();
  handle_joystick();
  
  //log_dbg();
  send_data();
}

void setup_pins() {
  pinMode(PIN_BTTN, INPUT);
  pinMode(PIN_BTTN_LED, OUTPUT);
  pinMode(PIN_JOYSTICK1_X, INPUT);
  pinMode(PIN_JOYSTICK2_Y, INPUT);

  digitalWrite(PIN_BTTN_LED, 0);
}

void setup_radio() {
  radio.begin();  // Инициализация модуля NRF24L01
  radio.setChannel(CHANNEL); // Обмен данными будет вестись на пятом канале (2,405 ГГц)
  radio.setDataRate (RF24_1MBPS); // Скорость обмена данными 1 Мбит/сек
  radio.setPALevel(RF24_PA_HIGH); // Выбираем высокую мощность передатчика (-6dBm)
  radio.openWritingPipe(PIPE1); // Открываем трубу с уникальным ID
}

void handle_bttn() {
  bttn_current_state = digitalRead(PIN_BTTN);
  if (bttn_old_state && !bttn_current_state) {
    magnet_control = !magnet_control;

    digitalWrite(PIN_BTTN_LED, magnet_control);
  }
  bttn_old_state = bttn_current_state;
}

void handle_joystick() {
  int raw_input = 0;

  raw_input = analogRead(PIN_JOYSTICK2_Y);
  carriage_move = map(raw_input, 0, 1024, 0, 255);

  raw_input = analogRead(PIN_JOYSTICK1_X);
  magnet_move = map(raw_input, 0, 1024, 0, 255);
}

void send_data() {
  // Передаём данные о движении магнита
  radio.openWritingPipe(PIPE1);
  is_sended = radio.write(&magnet_move, sizeof(magnet_move)); // Отправляем считанные показания по радиоканалу
  if (!is_sended)
    Serial.println("Error: fail to send data for magnet move");
  delay(50);

  // Передаём данные о состоянии магнита
  radio.openWritingPipe(PIPE2);
  is_sended = radio.write(&magnet_control, sizeof(magnet_control));
  if (!is_sended)
    Serial.println("Error: fail to send data for magnet turn on/off");
  delay(50);
}

void log_dbg() {
  Serial.print(String(carriage_move) + " ");
  Serial.print(String(magnet_move) + " ");
  Serial.println(magnet_control);
}
