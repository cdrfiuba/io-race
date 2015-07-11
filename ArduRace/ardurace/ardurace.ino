#define SENSOR_BLOCKED HIGH
#define SENSOR_CLEAR LOW

const int sensor_1 = A5;
const int sensor_2 = A4;
const int led_r = 11;
const int led_g = 3;
const int led_b = 7;
const int tone_out = 5;

int sensor1_in;
int sensor2_in;

unsigned long time;
int blocked;

void setup() {
  pinMode(sensor_1, INPUT);
  pinMode(sensor_2, INPUT);
  pinMode(led_r, OUTPUT);
  pinMode(led_g, OUTPUT);
  pinMode(led_b, OUTPUT);
  pinMode(tone_out, OUTPUT);
  
  Serial.begin(9600);
  
  digitalWrite(led_b, LOW);
  digitalWrite(led_r, LOW);
  digitalWrite(led_g, LOW);
  
  tone(tone_out, 38000);
  time = 0;
  blocked = 0;
}

void loop() {
  sensor1_in = digitalRead(sensor_1);
  sensor1_in = digitalRead(sensor_2);
  
  if ((sensor1_in == SENSOR_BLOCKED || sensor2_in == SENSOR_BLOCKED) && time == 0 && blocked == 0) {
    time = millis();
  }
  
  if (time != 0 && (millis() - time) > 1000) {
    if (sensor1_in == SENSOR_BLOCKED || sensor2_in == SENSOR_BLOCKED) {
      digitalWrite(led_b, HIGH);
      blocked = 0;
    } else {
      digitalWrite(led_b, LOW);
    }
    time = 0;
  } 
}

