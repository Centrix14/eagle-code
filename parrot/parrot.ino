// Пороговое значение
#define EDGE 600
#define PARROT_AVG_LOW  12 // Для апарели эти значения равны 13 и 14
#define PARROT_AVG_HIGH 13

// mm - main motor, главный мотор
#define REGULATOR_MM_REVERSE A0
#define REGULATOR_RAMP A1

// dev - device, внешние устройства
#define DEV_MM_REVERSE 9
#define DEV_RAMP_DIRECT 10
#define DEV_RAMP_REVERSE 11

typedef unsigned int uint;

int mm_parrot = 0, // «Попугаи» главного мотора
  ramp_parrot = 0; // «Попугаи» апарели

void setup() {
  // Настройка пинов регулятора
  pinMode(REGULATOR_MM_REVERSE, INPUT);
  pinMode(REGULATOR_RAMP, INPUT);

  // Настройка пинов нагрузки
  pinMode(DEV_MM_REVERSE, OUTPUT);
  pinMode(DEV_RAMP_REVERSE, OUTPUT);
  pinMode(DEV_RAMP_DIRECT, OUTPUT);

  // Настраиваем последовательный порт
  Serial.begin(9600);
}

void loop() {
  eval_parrots(&mm_parrot, REGULATOR_MM_REVERSE, 12, DEV_MM_REVERSE);
  //eval_parrots(&ramp_parrot, REGULATOR_RAMP, DEV_RAMP_DIRECT, DEV_RAMP_REVERSE);
}

void eval_parrots(int *parrots, int input, int motor_pin, int reverse_pin) {
  uint value = analogRead(input);

  if (value > EDGE)
    (*parrots)++;
  if (value < EDGE && (*parrots) > 0) {
    //Serial.println(*parrots);
    handle_motor(*parrots, motor_pin, reverse_pin);
    *parrots = 0;
  }
}

void handle_motor(int parrots, int motor_pin, int reverse_pin) {
  if (parrots > PARROT_AVG_HIGH) {
    digitalWrite(reverse_pin, LOW);
    digitalWrite(motor_pin, HIGH);

    Serial.println("Forward");
  }
  else if (parrots < PARROT_AVG_LOW) {
    digitalWrite(reverse_pin, HIGH);
    digitalWrite(motor_pin, HIGH);

    Serial.println("Backward");
  }
  else {
    digitalWrite(reverse_pin, LOW);
    digitalWrite(motor_pin, LOW);

    Serial.println("Stay");
  }
}
