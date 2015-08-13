/***********************************************************************
*                                                                      *
* ArduRace is a simple driver for a starting line gateway made with    *
* arduino. Copyright (C) 2015 Patricio Tula.                           *
* This program is free software: you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation, either version 3 of the License, or    *
* (at your option) any later version.                                  *
* This program is distributed in the hope that it will be useful,      *
* but WITHOUT ANY WARRANTY; without even the implied warranty of       *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
* GNU General Public License for more details.                         *
* You should have received a copy of the GNU General Public License    *
* along with this program.  If not, see <http://www.gnu.org/licenses/>.*
*                                                                      *
************************************************************************/

#define SENSOR_BLOCKED HIGH
#define SENSOR_CLEAR LOW
#define FINISH_COLORS true

typedef enum{STOP,READY}state_t;

const int sensor_1 = A5;
const int sensor_2 = A4;
const int led_r = 11;
const int led_g = 3;
const int led_b = 7;
const int tone_out = 5;

const int DEBOUNCE_TIME = 20; //Tiempo para hacer debounce
const int DEPARTURE_AWAITING_TIME = 2000; //Tiempo de espera para que pase el segundo Autito
const boolean finish_vector[6]={HIGH,HIGH,LOW,HIGH,LOW,HIGH};//Motivos de colores para el fin de carrera 

//Variables globales para el control de la barrera
int sensor1_in;
int sensor2_in;
unsigned long time;
unsigned char serial_command;
state_t state;
boolean departure, blocked;


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
  blocked = false;
  departure=true;
}

boolean crossed_barrier()
{
  boolean crossed = false;
  if ((sensor1_in == SENSOR_BLOCKED || sensor2_in == SENSOR_BLOCKED) && time == 0 && blocked == false) 
    time = millis();  
  if (time != 0 && (millis() - time) > DEBOUNCE_TIME) 
  {
    //Cuando un auto pase la barrera
    if (sensor1_in == SENSOR_BLOCKED || sensor2_in == SENSOR_BLOCKED) 
    {
      digitalWrite(led_b, HIGH);
      blocked = false;
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
  if(departure == true)//Primer vuelta
  {
    departure = (crossed_barrier()==true) ? false:true;
    if (departure == false)//Delay en espera a pase el segundo autito
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
    {
      Serial.write('L');
      state = STOP;

#if FINISH_COLORS
      for(int i=0; i < 13; i++ )
      {
        digitalWrite(led_r, finish_vector[i%6]);
        digitalWrite(led_g, finish_vector[(i+1)%6]);
        delay(100);
      }
#endif

    }
  }
}


void loop() 
{
  sensor1_in = digitalRead(sensor_1);
  sensor2_in = digitalRead(sensor_2);
  serial_command = Serial.read();
  switch(serial_command)
  {
    case 'D':
      state = READY;
      break;
    case 'd':
      state = STOP;
      break;
    default:
      break;
  }

  if(state == STOP)
  {
    digitalWrite(led_r, HIGH);
    digitalWrite(led_g, LOW);
    digitalWrite(led_b, LOW);
    departure=true;
  }
  else
    if(state == READY)
    {
      digitalWrite(led_r, LOW);
      digitalWrite(led_g, HIGH);
      digitalWrite(led_b, LOW);
      race();
    }
}

