"""
cronometro.py A simple cronometer made with pygame
Copyright (C) 2010  Santiago Piccinini

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
"""

"""
El programa necesita de python-serial y python-pygame (debian/ubuntu)
(probado con la 1.8)
se corre con permisos necesarios para abrir el puerto serie el cual hay que especificar
en las primeras lineas del archivo.

Se corre con python cronometro.py.
Teclas:
 * ESC: mostrar ganador
 * q: salir del programa
 * s: stop o 0x00 en el puerto serie
 * p: play o 0x01 en el puerto serie
 * d: stop pero descartar el tiempo.
 * r: reset

Al finalizar escribe en un archivo de texto (tiempos.txt) en modo escritura los tiempos
medidos de las carreras y tambien los imprime por pantalla.
"""

import sys, pygame, datetime, serial, os
import cars

print cars.cars

SERIAL_PORT = "/dev/ttyACM0" # "/dev/ttyACM0" o "COM3"

pygame.init()
pygame.font.init()
size = width, height = 800, 600
colors = {"red":(255, 0, 0), "green":(0,255,0), "black":(0,0,0), "white":(255,255,255)}
screen = pygame.display.set_mode(size, pygame.FULLSCREEN|pygame.HWSURFACE|pygame.DOUBLEBUF)

pygame.display.set_caption("Cronometro")
pygame.mouse.set_visible(False)
ck = pygame.time.Clock()

background = pygame.Surface(screen.get_size())
background = background.convert()

def load_image(name):
    fullname = os.path.join('.', name)
    image = pygame.image.load(fullname)
    image = image.convert_alpha()
    return image, image.get_rect()

class SerialIO(object):
    def __init__(self, port="COM3", timeout=0):
        self.ser = serial.Serial(port=port, timeout=timeout)
    def try_read_byte(self):
        rcv = self.ser.read(1)
        return rcv

    def __del__(self):
        self.ser.close()


class Cronometro(object):
    def __init__(self):
        self.running = False
        self.elapsed = datetime.timedelta(0)
        self.font = pygame.font.SysFont("Monospace", 120, bold=True)
        self.render(self.elapsed)
        
        background.blit(self.text, self.textpos)

    def reset(self):
        self.started = datetime.datetime.now()
        self.elapsed = datetime.timedelta(0)
        self.render(self.elapsed)

    def start(self):
        self.running = True
        self.started = datetime.datetime.now()

    def stop(self):
        if self.running:
            self.running = False
            now = datetime.datetime.now()
            self.elapsed = now - self.started
            self.render(self.elapsed)
       
    def update(self):
        if self.running:
            now = datetime.datetime.now()
            self.render(now - self.started)  
        else:
            self.render()

    def render(self, timedelta=None):
        if timedelta is not None:
            elapsed_txt = clockformat(timedelta)
            self.text = self.font.render(elapsed_txt, 1, (10, 10, 10))
        self.textpos = self.text.get_rect(centerx=width/2, centery=height/2)
        background.blit(self.text, self.textpos)

def clockformat(timedelta):
    return "%02d:%02d.%03d" % (timedelta.seconds /60,
                               timedelta.seconds % 60, 
                               int(str(timedelta.microseconds)[:3]),
                             )

ESPERANDO, CONTANDO, TERMINADO = range(3)

class World(object):
    def __init__(self):
        self.estado = ESPERANDO
        self.font = pygame.font.SysFont("Monospace", 120, bold=True)
        self.font_chica = pygame.font.SysFont("Monospace", 70, bold=True)
    def render(self):
        if self.estado == ESPERANDO:
            background.fill((255,255,255))
            cronometro.update()
        elif self.estado == CONTANDO:
            background.fill((192, 192, 0))
            cronometro.update()
        elif self.estado == TERMINADO:
            background.fill((0, 255, 0))
            cronometro.update()
        background.blit(banner, (102,0))
        nombre_carrera = self.font.render(self.nombre_carrera, 1, (10, 10, 10))
        background.blit(nombre_carrera, nombre_carrera.get_rect(centerx=width/2, centery=int(height*5.0/6.0)))
        if self.carreras:
            self.carreras.sort()
            mejor_carrera = self.carreras[0]
            #s = "Top:%s %s" % (mejor_carrera.nombre, clockformat(mejor_carrera.tiempo))
	    s = "Mejor tiempo"
            highscore = self.font_chica.render(s, 1, (230, 30, 30))
            background.blit(highscore, highscore.get_rect(centerx=width /2.0, centery=int(height*1.0/8.5)))
	    #s = "%s: %s" % (mejor_carrera.nombre, clockformat(mejor_carrera.tiempo))
	    s = "'%s'" % (mejor_carrera.nombre)
            highscore = self.font_chica.render(s, 1, (230, 30, 30))
            background.blit(highscore, highscore.get_rect(centerx=width /2.0, centery=int(height*1.0/4.5)))

	    s = "%s" % (clockformat(mejor_carrera.tiempo))
            highscore = self.font_chica.render(s, 1, (230, 30, 30))
            background.blit(highscore, highscore.get_rect(centerx=width /2.0, centery=int(height*1.0/3.0)))

class Carrera(object):
    def __init__(self, timedelta, nombre):
        self.tiempo = timedelta
        self.nombre = nombre
    def __cmp__(self, otro):
        if self.tiempo >= otro.tiempo:
            return 1
        else:
            return -1
    def __str__(self):
        return "%s, %s" % (self.nombre, clockformat(self.tiempo))
    def __repr__(self):
        return str(self)

def imprimir_carreras(carreras, archivo):
    carreras.sort()
    for n, carrera in enumerate(carreras):
        print n+1, carrera
    f = open(archivo, "w")
    f.write("="*10 + "\n")
    f.write(datetime.datetime.now().strftime("%A, %d. %B %Y %I:%M%p") + "\n")
    for n, carrera in enumerate(carreras):
        salida = "%d --- %s\n" % (n+1, carrera)
        f.write(salida)
    f.close()

def leer_carreras(carreras, archivo):
    try: 
        f = open(archivo, "r")
    except:
        return
    for linea in f:
        if "---" not in linea:
            continue
        linea = linea[linea.index("---")+4:]
	nombre, tiempo = linea.split(",")
        seg, mili = tiempo.split(".")
        min, seg = seg.split(":")
        seg = int(min)*60+int(seg)
        tiempo = datetime.timedelta(0,seg,int(mili)*1000)
        carreras.append(Carrera(tiempo, nombre))
    f.close()


if len(sys.argv) > 1:
   archivo = sys.argv[1]
else:
   archivo = "tiempos.txt"

carreras = []
leer_carreras(carreras, archivo)

banner, banner_rect = load_image("logo-club-banner.png")
world = World()
cronometro = Cronometro()
serial_io = SerialIO(port=SERIAL_PORT, timeout=0)

import time,subprocess
time.sleep(1)
serial_io.ser.read(1000)

auto = 0
world.nombre_carrera = str(cars.cars[auto])
world.carreras = carreras
while 1:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            imprimir_carreras(carreras)
            sys.exit()
        elif event.type == pygame.KEYDOWN:
            if event.key == pygame.K_ESCAPE:
                background.fill((255,255,255))
                font = pygame.font.SysFont("Monospace", 100, bold=True)
                mejor_carrera = world.carreras[0]
                textos = ("Ganador!", mejor_carrera.nombre, clockformat(mejor_carrera.tiempo))
                for n, texto in enumerate(textos):
                    t = font.render(texto, 1, (110, 110, 110))
                    background.blit(t, t.get_rect(centerx=width/2.0, centery=height/float(len(textos))*(n+1)-100))
                screen.blit(background, (0, 0))
                pygame.display.flip()
                imprimir_carreras(carreras, archivo)
                while True:
                  for event in pygame.event.get():
                      if event.key == pygame.K_q:
                          sys.exit()
            elif event.key == pygame.K_s:
                cronometro.stop()
                world.estado = TERMINADO
                carreras.append(Carrera(cronometro, elapsed, world.nombre_carrera))
                carreras.sort()
            elif event.key == pygame.K_UP:
                if world.estado == ESPERANDO:
                     auto = (auto + 1)%len(cars.cars)
                     world.nombre_carrera = str(cars.cars[auto])
                     serial_io.ser.read(100)
                     world.render()
            elif event.key == pygame.K_DOWN:
                if world.estado == ESPERANDO:
                     auto = (auto - 1)%len(cars.cars)
                     world.nombre_carrera = str(cars.cars[auto])
                     serial_io.ser.read(100)
                     world.render()
            elif event.key ==  pygame.K_r:
                cronometro.reset()
                serial_io.ser.write('D')
                world.estado = ESPERANDO
                serial_io.ser.read(100)
                world.render()
            elif event.key == pygame.K_p:
                cronometro.start()
                serial_io.ser.write('D')
                world.estado = CONTANDO
            elif event.key == pygame.K_d:
                cronometro.stop()
                serial_io.ser.write('d')
                world.estado = TERMINADO

    # Vemos si paso algo en el mbed
    rcv = serial_io.try_read_byte()
    if rcv:
        if rcv == "L": # Partida
            if world.estado == ESPERANDO:
                cronometro.start()
                world.estado = CONTANDO
            elif world.estado == CONTANDO:
                cronometro.stop()
                serial_io.ser.write('d')
                world.estado = TERMINADO
                carreras.append(Carrera(cronometro.elapsed, world.nombre_carrera))

    world.render()
    
    screen.blit(background, (0, 0))
    pygame.display.flip()

    
