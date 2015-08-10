#define SENSOR_BLOCKED HIGH
#define SENSOR_CLEAR LOW

typedef enum{STOP,READY,RACE}state_t;

const int sensor_1 = A5;
const int sensor_2 = A4;
const int led_r = 11;
const int led_g = 3;
const int led_b = 7;
const int tone_out = 5;

int DEBOUNCE_TIME = 20; //TODO hay que ponerle un poco mas para que pase el auto entero
int DEPARTURE_AWAITING_TIME = 1500; //TODO probar si deja bloquear este tiempo

int sensor1_in;
int sensor2_in;
unsigned long time;
int blocked;
unsigned char serial_command;
state_t state;
boolean departure;

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
  departure=true;
}

boolean crossed_barrier()
{
  boolean crossed = false;
  if ((sensor1_in == SENSOR_BLOCKED || sensor2_in == SENSOR_BLOCKED) && time == 0 && blocked == 0) 
    time = millis();  
  if (time != 0 && (millis() - time) > DEBOUNCE_TIME) 
  {
    //Cuando un auto pase la barrera
    if (sensor1_in == SENSOR_BLOCKED || sensor2_in == SENSOR_BLOCKED) 
    {
      Serial.write('L');
      digitalWrite(led_b, HIGH);
      blocked = 0;
      crossed=true;
    } 
    else
      digitalWrite(led_b, LOW);
    time = 0;
  }
  return crossed; 
}

void race()
{ 
  //Primer vuelta
  if(departure == true)
  {
    departure = (crossed_barrier()==true)?false:true;
    //Si un auto paso la barrera espero un rato para dejar que pase el otro
    if (departure == false)
    {
      digitalWrite(led_b, HIGH);
      delay(DEPARTURE_AWAITING_TIME);
      digitalWrite(led_b, LOW);
    }
  }
  else//Segunda vuelta
  {
    departure = crossed_barrier();
    if (departure == true)
      state = STOP;
  }
}


void loop() 
{
  sensor1_in = digitalRead(sensor_1);
  sensor2_in = digitalRead(sensor_2);
  unsigned char serial_command = Serial.read();
  
  switch(serial_command)
  {
    case 'D':
      state = READY;
      break;
    case 'd':
      state = STOP;
      break;
    case 'p':
        state = RACE;
      break;
    default:
      state = STOP;
      break;
  }

  if(state == READY)
  {
    digitalWrite(led_r, LOW);
    digitalWrite(led_g, HIGH);
    digitalWrite(led_b, LOW);
  }
  else
  {
    if(state == STOP)
    {
      digitalWrite(led_r, HIGH);
      digitalWrite(led_g, LOW);
      digitalWrite(led_b, LOW);
    }
    else
    {
      if(state == RACE)
      {
        digitalWrite(led_r, LOW);
        digitalWrite(led_g, HIGH);
        digitalWrite(led_b, LOW);
        race();
      }
    }
  } 
}


