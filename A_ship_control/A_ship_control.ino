// Программа для пульта от парома
// 24.03.2023

#include <SPI.h>  // Подключаем библиотеку для работы с SPI-интерфейсом
#include <nRF24L01.h> // Подключаем файл конфигурации из библиотеки RF24
#include <RF24.h> // Подключаем библиотеку для работа для работы с модулем NRF24L01

#define PIN_CE  10  // Номер пина Arduino, к которому подключен вывод CE радиомодуля
#define PIN_CSN 9 // Номер пина Arduino, к которому подключен вывод CSN радиомодуля
#define PIN_BTTN 7 // Пин кнопки
#define PIN_BTTN_LED 6 // Пин светодиода кнопки
#define PIN_REOSTAT1 A4 // Пин реостата 1
#define PIN_REOSTAT2 A5 // Пин реостата 2
#define PIN_JOYSTICK_X A3 // Пин джойстика

RF24 radio(PIN_CE, PIN_CSN); // Создаём объект radio с указанием выводов CE и CSN

// Текущее и предыдущее состояние кнопки
uint8_t bttn_current_state = 0;
uint8_t bttn_old_state = 0;

// Данные для передачи
// Первый элемент - состояние кнопки
// Второй элемент - состояние потенциометра 1
// Третий элемент - состояние потенциометра 2
// Третий элемент - состояние джойстика

uint8_t control_data[] = {0, 0, 0, 0};

void setup() {
  setup_pins();
  setup_radio();

  Serial.begin(9600);
}

void loop() {
  handle_bttn();
  handle_reostat();
  handle_joystick();
  
  log_dbg();
  send_data();
}

void setup_pins() {
  pinMode(PIN_BTTN, INPUT);
  pinMode(PIN_BTTN_LED, OUTPUT);
  pinMode(PIN_REOSTAT1, INPUT);
  pinMode(PIN_REOSTAT2, INPUT);
  pinMode(PIN_JOYSTICK_X, INPUT);

  digitalWrite(PIN_BTTN_LED, 0);
}

void setup_radio() {
  radio.begin();  // Инициализация модуля NRF24L01
  radio.setChannel(9); // Обмен данными будет вестись на пятом канале (2,405 ГГц)
  radio.setDataRate (RF24_1MBPS); // Скорость обмена данными 1 Мбит/сек
  radio.setPALevel(RF24_PA_HIGH); // Выбираем высокую мощность передатчика (-6dBm)
  radio.openWritingPipe(0x7878787878LL); // Открываем трубу с уникальным ID
}

void handle_bttn() {
  bttn_current_state = digitalRead(PIN_BTTN);
  if (bttn_old_state && !bttn_current_state) {
    control_data[0] = !control_data[0];

    digitalWrite(PIN_BTTN_LED, control_data[0]);
  }
  bttn_old_state = bttn_current_state;
}

void handle_reostat() {
  int raw_input = 0;

  raw_input = analogRead(PIN_REOSTAT1);
  control_data[1] = map(raw_input, 0, 1024, 0, 180);

  raw_input = analogRead(PIN_REOSTAT2);
  control_data[2] = map(raw_input, 0, 1024, 0, 255);
}

void handle_joystick() {
  int raw_input = 0;

  raw_input = analogRead(PIN_JOYSTICK_X);
  control_data[3] = map(raw_input, 0, 1024, 0, 180);
}

void send_data() {
  uint8_t is_sended = radio.write(&control_data, sizeof(control_data)); // Отправляем считанные показания по радиоканалу
  if (!is_sended)
    Serial.println("Connection error");
  delay(50);
}

void log_dbg() {
  Serial.print(String(control_data[0]) + " ");
  Serial.print(String(control_data[1]) + " ");
  Serial.print(String(control_data[2]) + " ");
  Serial.println(control_data[3]);
}
