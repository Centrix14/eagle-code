// Программа для парома
// 24.03.2023

// , (GND) . (5V)
// . ( 10) . ( 9)
// . ( 13) . (11)
// . ( 12) .

#include <SPI.h>  // Подключаем библиотеку для работы с SPI-интерфейсом
#include <nRF24L01.h> // Подключаем файл конфигурации из библиотеки RF24
#include <RF24.h> // Подключаем библиотеку для работа для работы с модулем NRF24L01
#include <Servo.h> // Библиотека для работы с servo-мотором
#include <AceRoutine.h>

#define CHANNEL 120
#define PIPE0   0x7878787878LL

// Пины nRF
#define PIN_CE  10 // Номер пина Arduino, к которому подключен вывод CE радиомодуля
#define PIN_CSN 9  // Номер пина Arduino, к которому подключен вывод CSN радиомодуля

// Двигатель аппарели
#define PIN_RE_LOGIC   5 // Логический пин
#define PIN_RE_REVERSE 6 // Пин обратного хода

// Главный двигатель (main engine)
#define PIN_ME_LOGIC   3 // Логический пин
#define PIN_ME_REVERSE 4 // Пин обратного хода

// Руль
#define PIN_WHEEL 2

// Задержка реверса
#define REVERSE_DELAY 200

// Чувствительность, средние значения
#define SENSE       10
#define WHEEL_MEAN  125
#define RAMP_MEAN   150

#define ramp_rest() { \
  digitalWrite(PIN_RE_LOGIC, LOW); \
  COROUTINE_DELAY(REVERSE_DELAY); \
  digitalWrite(PIN_RE_REVERSE, LOW); \
  COROUTINE_DELAY(REVERSE_DELAY); \
}

#define motor_rest() { \
  digitalWrite(PIN_ME_LOGIC, LOW); \
  COROUTINE_DELAY(REVERSE_DELAY); \
  digitalWrite(PIN_ME_REVERSE, LOW); \
  COROUTINE_DELAY(REVERSE_DELAY); \
}

RF24 radio(PIN_CE, PIN_CSN); // Создаём объект radio с указанием выводов CE и CSN

Servo wheel; // Серва руля
Servo motor; // Основной двигатель парома (это не серва!)

// Данные для передачи
// Первый элемент - состояние кнопки реверса
// Второй элемент - состояние потенциометра апарели
// Третий элемент - состояние потенциометра скорости
// Третий элемент - состояние джойстика руля

uint8_t control_data[] = {0, 0, 0, 0};

uint8_t ramp_old_reverse_state = 0,
        motor_old_reverse_state = 0;

COROUTINE (apply_ramp) {
  COROUTINE_LOOP () {
    uint8_t ramp_state = control_data[1];

    if (abs(ramp_state - RAMP_MEAN) > SENSE) {
      bool ramp_reverse_state = 0;
      
      if (ramp_state < RAMP_MEAN)
        ramp_reverse_state = 1;

      if (ramp_reverse_state != ramp_old_reverse_state)
        ramp_rest();

      digitalWrite(PIN_RE_LOGIC, HIGH);
      digitalWrite(PIN_RE_REVERSE, ramp_reverse_state);

      ramp_old_reverse_state = ramp_reverse_state;
    }
    else {
      digitalWrite(PIN_RE_LOGIC, LOW);
      digitalWrite(PIN_RE_REVERSE, LOW);
    }
  }
}

COROUTINE (apply_motor) {
  COROUTINE_LOOP () {
    uint8_t motor_reverse_state = control_data[0];
    uint8_t motor_state = control_data[2];

    if (motor_reverse_state != motor_old_reverse_state)
      motor_rest();

    if (motor_reverse_state)
      digitalWrite(PIN_ME_REVERSE, HIGH);
    else
      digitalWrite(PIN_ME_REVERSE, LOW);
      
    if (motor_state > SENSE) {
      motor.write(motor_state);
    }
  }
}

COROUTINE (apply_wheel) {
  COROUTINE_LOOP () {
    uint8_t wheel_state = control_data[3];

    if (abs(wheel_state - WHEEL_MEAN) > SENSE) {
      if (wheel_state > WHEEL_MEAN)
        wheel.write(0);
      else
        wheel.write(180);
    }
    else
      wheel.write(90);
  }
}

void setup() {
  setup_radio();
  setup_pins();
  
  wheel.attach(PIN_WHEEL);
  motor.attach(PIN_ME_LOGIC);
  
  Serial.begin(9600);
}

void loop() {  
  if (radio.available()) { // Если в буфер приёмника поступили данные
    radio.read(&control_data, sizeof(control_data));
    apply_data(control_data);
    
    log_dbg();
  }
}

void setup_radio() {
  radio.begin();  // Инициализация модуля NRF24L01
  radio.setChannel(CHANNEL); // Обмен данными будет вестись на пятом канале (2,405 ГГц)
  radio.setDataRate(RF24_250KBPS); // Скорость обмена данными 1 Мбит/сек
  radio.setPALevel(RF24_PA_LOW); // Выбираем высокую мощность передатчика (-6dBm)
  radio.openReadingPipe(1, PIPE0); // Открываем трубу ID передатчика
  radio.startListening(); // Начинаем прослушивать открываемую трубу
}

void setup_pins() {
  pinMode(PIN_ME_LOGIC, OUTPUT);
  pinMode(PIN_ME_REVERSE, OUTPUT);
  
  pinMode(PIN_RE_LOGIC, OUTPUT);
  pinMode(PIN_RE_REVERSE, OUTPUT);
  
  pinMode(PIN_WHEEL, OUTPUT);
}

void log_dbg() {
  Serial.print(String(control_data[0]) + " ");
  Serial.print(String(control_data[1]) + " ");
  Serial.print(String(control_data[2]) + " ");
  Serial.println(control_data[3]);
}

void apply_data(uint8_t data[]) {
  uint8_t wheel = data[3];

  apply_motor.runCoroutine();
  apply_ramp.runCoroutine();

  apply_wheel.runCoroutine();
}
