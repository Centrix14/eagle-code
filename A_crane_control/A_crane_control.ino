// Программа для пульта от крана
// 24.03.2023

#include <SPI.h>  // Подключаем библиотеку для работы с SPI-интерфейсом
#include <nRF24L01.h> // Подключаем файл конфигурации из библиотеки RF24
#include <RF24.h> // Подключаем библиотеку для работа для работы с модулем NRF24L01
#include <AceRoutine.h>

using namespace ace_routine;

#define PIN_CE  10  // Номер пина Arduino, к которому подключен вывод CE радиомодуля
#define PIN_CSN 9 // Номер пина Arduino, к которому подключен вывод CSN радиомодуля
#define PIN_BTTN 7 // Пин кнопки
#define PIN_BTTN_LED 6 // Пин светодиода кнопки
#define PIN_JOYSTICK1_X A4 // Пин джойстика упрвляющего кареткой
#define PIN_JOYSTICK2_Y A5 // Пин джойстика управляющего высотой магнита

#define PIN_CARRIAGE_HORIZONTAL_MOTOR 4
#define PIN_CARRIAGE_HORIZONTAL_MOTOR_REVERSE 5
#define PIN_CARRIAGE_VERTICAL_MOTOR 2
#define PIN_CARRIAGE_VERTICAL_MOTOR_REVERSE 3
#define PIN_CARRIAGE_MAGNET 13

#define CHANNEL 100
#define PIPE1 0xB4B5B6B7F1 // Труба для управления кареткой
#define PIPE2 0xB4B5B6B7CD // Труба для управления включения и выключения магнита

#define MEAN_JX 507
#define MEAN_JY 513
#define SENSE 500

#define REVERSE_DELAY 1000

RF24 radio(PIN_CE, PIN_CSN); // Создаём объект radio с указанием выводов CE и CSN

// Текущее и предыдущее состояние кнопки
uint8_t bttn_current_state = 0;
uint8_t bttn_old_state = 0;

bool hm_old_reverse_state = 0,
     vm_old_reverse_state = 0;

// Движение каретки
uint8_t carriage_move = 0;

// Движение магнита
uint8_t magnet_move = 0;

// Включение и выключение магнита
uint8_t magnet_control = 0;

// Результат отправки
bool is_sended = 0;

void setup() {
  delay(1000);
  setup_pins();
  //setup_radio();

  Serial.begin(115200);
}

void loop() {
  int state = analogRead(PIN_JOYSTICK2_Y);
  if (abs(state - MEAN_JY) < SENSE)
    handle_horizontal_motor();

  state = analogRead(PIN_JOYSTICK1_X);
  if (abs(state - MEAN_JX) < SENSE)
    handle_vertical_motor();
  
  //handle_joystick();
  
  //log_dbg();
  //send_data();
}

void handle_horizontal_motor() {
  int motor_position = analogRead(PIN_JOYSTICK1_X);
  int reverse = 0;
  //Serial.println(motor_position);

    if (abs(motor_position - MEAN_JX) > SENSE) {
      if (motor_position > MEAN_JX)
        reverse = 0;
      else
        reverse = 1;

      if (hm_old_reverse_state != reverse) {
        digitalWrite(PIN_CARRIAGE_HORIZONTAL_MOTOR, LOW);
        digitalWrite(PIN_CARRIAGE_HORIZONTAL_MOTOR_REVERSE, LOW);

        delay(REVERSE_DELAY);
      }
        
      digitalWrite(PIN_CARRIAGE_HORIZONTAL_MOTOR, HIGH);
      digitalWrite(PIN_CARRIAGE_HORIZONTAL_MOTOR_REVERSE, reverse);

      hm_old_reverse_state = reverse;
    }
    else {
      digitalWrite(PIN_CARRIAGE_HORIZONTAL_MOTOR, LOW);
      digitalWrite(PIN_CARRIAGE_HORIZONTAL_MOTOR_REVERSE, LOW);
    }
}

void handle_vertical_motor() {
  int motor_position = analogRead(PIN_JOYSTICK2_Y);
    int vm_reverse_state = 0;
    //Serial.println(motor_position);

    if (abs(motor_position - MEAN_JY) > SENSE) {
      if (motor_position > MEAN_JY)
        vm_reverse_state = 0;
      else
        vm_reverse_state = 1;

      if (vm_old_reverse_state != vm_reverse_state) {
        digitalWrite(PIN_CARRIAGE_VERTICAL_MOTOR, LOW);
        digitalWrite(PIN_CARRIAGE_VERTICAL_MOTOR_REVERSE, LOW);

        delay(REVERSE_DELAY);
      }

      digitalWrite(PIN_CARRIAGE_VERTICAL_MOTOR, HIGH);
      digitalWrite(PIN_CARRIAGE_VERTICAL_MOTOR_REVERSE, vm_reverse_state);
      Serial.println("VM Work: " + String(vm_reverse_state));

    }
    else {
      digitalWrite(PIN_CARRIAGE_VERTICAL_MOTOR, LOW);
      digitalWrite(PIN_CARRIAGE_VERTICAL_MOTOR_REVERSE, LOW);
      Serial.println("VM Sleep");
    }

    vm_old_reverse_state = vm_reverse_state;
}

void setup_pins() {
  pinMode(PIN_BTTN, INPUT);
  pinMode(PIN_BTTN_LED, OUTPUT);
  pinMode(PIN_JOYSTICK1_X, INPUT);
  pinMode(PIN_JOYSTICK2_Y, INPUT);

  pinMode(PIN_CARRIAGE_HORIZONTAL_MOTOR, OUTPUT);
  pinMode(PIN_CARRIAGE_HORIZONTAL_MOTOR_REVERSE, OUTPUT);
  pinMode(PIN_CARRIAGE_VERTICAL_MOTOR, OUTPUT);
  pinMode(PIN_CARRIAGE_VERTICAL_MOTOR_REVERSE, OUTPUT);
  pinMode(PIN_CARRIAGE_MAGNET, OUTPUT);

  digitalWrite(PIN_BTTN_LED, 0);
}

// trash
void setup_radio() {
  radio.begin();                  // Инициализация модуля NRF24L01
  radio.setChannel(CHANNEL);      // Обмен данными будет вестись на пятом канале (2,405 ГГц)
  radio.setDataRate(RF24_1MBPS);  // Скорость обмена данными 1 Мбит/сек
  radio.setPALevel(RF24_PA_HIGH); // Выбираем высокую мощность передатчика (-6dBm)
  radio.openWritingPipe(PIPE1);   // Открываем трубу с уникальным ID
}

// trash
void handle_bttn() {
  bttn_current_state = digitalRead(PIN_BTTN);
  Serial.println(bttn_current_state);
  if (bttn_old_state && !bttn_current_state) {
    magnet_control = !magnet_control;

    digitalWrite(PIN_BTTN_LED, magnet_control);
    digitalWrite(PIN_CARRIAGE_MAGNET, magnet_control);
  }
  bttn_old_state = bttn_current_state;
}

// trash
void handle_joystick() {
  int raw_input = 0;

  raw_input = analogRead(PIN_JOYSTICK2_Y);
  magnet_move = map(raw_input, 0, 1024, 0, 255);

  raw_input = analogRead(PIN_JOYSTICK1_X);
  carriage_move = map(raw_input, 0, 1024, 0, 255);
}

// trash
void send_data() {
  uint8_t carriage_data[] = {carriage_move, magnet_move};
  
  // Передаём данные о движении магнита
  radio.openWritingPipe(PIPE1);
  is_sended = radio.write(&carriage_data, sizeof(carriage_data)); // Отправляем считанные показания по радиоканалу
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

// trash
void log_dbg() {
  Serial.print(String(carriage_move) + " ");
  Serial.print(String(magnet_move) + " ");
  Serial.println(magnet_control);
}
