// Программа для каретки
// 29.03.2023

// =========================================================================
// Подключаем библиотеки

#include <SPI.h>      // Подключаем библиотеку для работы с SPI-интерфейсом
#include <nRF24L01.h> // Подключаем файл конфигурации из библиотеки RF24
#include <RF24.h>     // Подключаем библиотеку для работа для работы с модулем NRF24L01
#include <AceRoutine.h>

// =========================================================================
// Параметры программы

// Пины радио модуля
#define PIN_CE  10  // Номер пина Arduino, к которому подключен вывод CE радиомодуля
#define PIN_CSN 9   // Номер пина Arduino, к которому подключен вывод CSN радиомодуля

// Пины каретки
#define PIN_CARRIAGE_REVERSE_ENABLE 3 // Включение реверса
#define PIN_CARRIAGE_REVERSE_POWER  4 // Питание обратного хода
#define PIN_CARRIAGE_DIRECT_POWER   5 // Питание прямого хода

// Пины подъёмника
#define PIN_MAGNET_REVERSE_ENABLE 8 // Включение реверса
#define PIN_MAGNET_REVERSE_POWER  7 // Питание обратного хода
#define PIN_MAGNET_DIRECT_POWER   6 // Питание прямого хода

// Трубы
#define PIPE1 0xB4B5B6B7F1 // Труба по которой пульт общается с кареткой

// Пороговые значения
#define AVG         117          // Среднее значение джойстика
#define SENSETIVITY 20           // Чувствительность джойстика

// Задержки
#define POWER_DELAY 100 // Задержка реверса

// Макрос, организующий отдых
#define rest(PIN1, PIN2, PIN3) \
  digitalWrite(PIN1, LOW); \
  COROUTINE_DELAY(POWER_DELAY); \
  digitalWrite(PIN2, HIGH); \
  COROUTINE_DELAY(POWER_DELAY); \
  digitalWrite(PIN3, LOW); \

// =========================================================================
// Основные переменные
RF24 radio(PIN_CE, PIN_CSN); // Создаём объект radio с указанием выводов CE и CSN

// Данные от пульта
uint8_t data[] = {0, 0};   // Данные, приходящие на приёмник
uint8_t carriage_move = 0; // Движение каретки
uint8_t magnet_move = 0;   // Движение магнита

// Переменные для управления реверсом каретки
uint8_t carriage_is_reverse = 0;     // Нужно ли сделать реверс
uint8_t carriage_is_reverse_old = 0; // Старое значение реверса

// Переменные для управления реверсом подъёмника
uint8_t magnet_is_reverse = 0;     // Нужно ли сделать реверс
uint8_t magnet_is_reverse_old = 0; // Старое значение реверса

// =========================================================================
// Корутины

// Корутина движения каретки
COROUTINE (move_carriage) {
  COROUTINE_LOOP() {
    if (carriage_move - AVG > SENSETIVITY) {
      Serial.println("Carriage forward!");
      // Движение вперёд
      carriage_is_reverse = 1;
  
      if (carriage_is_reverse != carriage_is_reverse_old) {
        rest(PIN_CARRIAGE_REVERSE_ENABLE, PIN_CARRIAGE_DIRECT_POWER, PIN_CARRIAGE_REVERSE_POWER);
      }
  
      carriage_is_reverse_old = carriage_is_reverse;
  
      digitalWrite(PIN_CARRIAGE_REVERSE_ENABLE, LOW);
      COROUTINE_DELAY(POWER_DELAY);
      digitalWrite(PIN_CARRIAGE_DIRECT_POWER, HIGH);
      COROUTINE_DELAY(POWER_DELAY);
      digitalWrite(PIN_CARRIAGE_REVERSE_POWER, LOW);
    }
    else if (AVG - carriage_move > SENSETIVITY) {
      Serial.println("Carriage backward!");
      // Движение назад
      carriage_is_reverse = 1;
  
      if (carriage_is_reverse != carriage_is_reverse_old) {
        rest(PIN_CARRIAGE_REVERSE_ENABLE, PIN_CARRIAGE_DIRECT_POWER, PIN_CARRIAGE_REVERSE_POWER);
      }
  
      carriage_is_reverse_old = carriage_is_reverse;
  
      digitalWrite(PIN_CARRIAGE_REVERSE_ENABLE, HIGH);
      COROUTINE_DELAY(POWER_DELAY);
      digitalWrite(PIN_CARRIAGE_DIRECT_POWER, LOW);
      COROUTINE_DELAY(POWER_DELAY);
      digitalWrite(PIN_CARRIAGE_REVERSE_POWER, HIGH);
    }
    else {
      Serial.println("Carriage stay!");
      // Стоим
      rest(PIN_CARRIAGE_REVERSE_ENABLE, PIN_CARRIAGE_DIRECT_POWER, PIN_CARRIAGE_REVERSE_POWER);
    }
  }
}

// Корутина движения магнита
COROUTINE (move_magnet) {
  COROUTINE_LOOP() {
    if (magnet_move - AVG > SENSETIVITY) {
      Serial.println("Magnet forward!");
      // Движение вперёд
      magnet_is_reverse = 1;
  
      if (magnet_is_reverse != magnet_is_reverse_old) {
        rest(PIN_MAGNET_REVERSE_ENABLE, PIN_MAGNET_DIRECT_POWER, PIN_MAGNET_REVERSE_POWER);
      }
  
      magnet_is_reverse_old = magnet_is_reverse;
  
      digitalWrite(PIN_CARRIAGE_REVERSE_ENABLE, LOW);
      COROUTINE_DELAY(POWER_DELAY);
      digitalWrite(PIN_CARRIAGE_DIRECT_POWER, HIGH);
      COROUTINE_DELAY(POWER_DELAY);
      digitalWrite(PIN_CARRIAGE_REVERSE_POWER, LOW);
    }
    else if (AVG - magnet_move > SENSETIVITY) {
      Serial.println("Magnet backward!");
      // Движение назад
      magnet_is_reverse = 1;
  
      if (magnet_is_reverse != magnet_is_reverse_old) {
        rest(PIN_MAGNET_REVERSE_ENABLE, PIN_MAGNET_DIRECT_POWER, PIN_MAGNET_REVERSE_POWER);
      }
  
      magnet_is_reverse_old = magnet_is_reverse;
  
      digitalWrite(PIN_CARRIAGE_REVERSE_ENABLE, HIGH);
      COROUTINE_DELAY(POWER_DELAY);
      digitalWrite(PIN_CARRIAGE_DIRECT_POWER, LOW);
      COROUTINE_DELAY(POWER_DELAY);
      digitalWrite(PIN_CARRIAGE_REVERSE_POWER, HIGH);
    }
    else {
      Serial.println("Magnet stay!");
      // Стоим
      rest(PIN_MAGNET_REVERSE_ENABLE, PIN_MAGNET_DIRECT_POWER, PIN_MAGNET_REVERSE_POWER);
    }
  }
}

// =========================================================================
// Функции

// Настройки
void setup() {  
  setup_radio();

  pinMode(PIN_CARRIAGE_DIRECT_POWER, OUTPUT);
  pinMode(PIN_CARRIAGE_REVERSE_ENABLE, OUTPUT);
  pinMode(PIN_CARRIAGE_REVERSE_POWER, OUTPUT);

  pinMode(PIN_MAGNET_DIRECT_POWER, OUTPUT);
  pinMode(PIN_MAGNET_REVERSE_ENABLE, OUTPUT);
  pinMode(PIN_MAGNET_REVERSE_POWER, OUTPUT);

  Serial.begin(9600);
}

// Цикл
void loop() {  
  if (radio.available()) { // Если в буфер приёмника поступили данные
    radio.read(&data, sizeof(data));
    carriage_move = data[0];
    magnet_move = data[1];
    
    move_carriage.runCoroutine();
    move_magnet.runCoroutine();
  }
}

// Настройка радио модуля
void setup_radio() {
  radio.begin();                   // Инициализация модуля NRF24L01
  radio.setChannel(9);             // Обмен данными будет вестись на пятом канале (2,405 ГГц)
  radio.setDataRate(RF24_1MBPS);   // Скорость обмена данными 1 Мбит/сек
  radio.setPALevel(RF24_PA_HIGH);  // Выбираем высокую мощность передатчика (-6dBm)
  radio.openReadingPipe(1, PIPE1); // Открываем трубу ID передатчика
  radio.startListening();          // Начинаем прослушивать открываемую трубу
}

// Отладочные записи
void log_dbg() {
  Serial.println(String(carriage_move) + String(magnet_move));
}
