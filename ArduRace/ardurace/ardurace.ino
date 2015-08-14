/***********************************************************************
*                                                                      *
* ArduRace is a simple driver for a starting line gateway made with    *
* arduino. Copyright (C) 2015 Patricio Tula and Lucas Chiesa.          *
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

const int AWAITING_TIME_TO_BLOCK = 850; //Tiempo para bloquear los sensores
const boolean finish_vector[6]={HIGH,HIGH,LOW,HIGH,LOW,HIGH};//Motivos de colores para el fin de carrera 

//Variables globales para el control de la barrera
int sensor1_in;
int sensor2_in;
unsigned long time;
unsigned char serial_command;
state_t state;
boolean blocked;
unsigned int lap;

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
  lap=0;
}

boolean lap_counter()
{
  if ((sensor1_in == SENSOR_BLOCKED || sensor2_in == SENSOR_BLOCKED) && time == 0 && blocked == false) 
    time = millis();  
  if (time != 0 && (millis() - time) > AWAITING_TIME_TO_BLOCK) 
  {
    if (sensor1_in == SENSOR_BLOCKED || sensor2_in == SENSOR_BLOCKED) 
    {
      Serial.write('L');
      digitalWrite(led_b, HIGH); delay(100);
      blocked = false;
      lap++;
    } 
    else
      digitalWrite(led_b, LOW);
    time = 0;
  }
}

void race()
{ 
  lap_counter();
  if (lap == 2)
  {
    state = STOP;
    
#if FINISH_COLORS
    for(int i=0; i < 20; i++ )
    {
      digitalWrite(led_r, finish_vector[i%6]);
      digitalWrite(led_g, finish_vector[(i+1)%6]);
      delay(90);
    }
#endif
  
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
    lap = 0;
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

