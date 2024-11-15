

# Servidor Web de Acuicultura

Este proyecto esta diseñado con el fin de obtener y monitorear graficamente las variables fisicas internas y externas que pueden existir en un estanque acuicola. En el se realiza la programacion en lenguaje C++ acoplado para un microcontrolador ESP32-WROOM mediante la extension de PlatformIO. 
Su funcion principal se basa en tomar la lectura de sensores para enviar esos datos a un servidor web y graficar estos datos en tiempo real, de la misma manera enviar desde el servidor activaciones digitales para actuadores dentro del mismo sistema.

## Contenido

* [Descripción del proyecto](#Servidor-Web-de-Acuicultura)
* [Contenido](#Contenido)
* [Requisitos](#Requisitos)
* [Instalacion](#Instalacion)
* [Tecnologias utilizadas](#Tecnologias-utilizadas)
* [Ejecucion](#Ejecucion)
* [Autores](#Autores)




## Requisitos 📋

- Software de edicion de codigo fuente o IDE, preferiblemnete Visual Studio Code o VScodium.
- Tarjeta de desarrollo ESP32
- Punto de acceso para conexion Wi-Fi
- Sensor pH (PH0-14) - Sensor de luz BH1750 - Sensor BMP280 - Sensor HDC 1080
- Pantalla oled 128x64  


## Instalacion  🔧

+ Instalar Visual Studio Code o VScodium.
+ Dentro de IDE descargar la extension PlatformIO IDE. 
+ En una nueva venta del IDE abrir la carpeta del proyecto.

    
## Tecnologias utilizadas 🛠
C++, Javascript, HTML, CSS.



## Ejecucion ⚙️

Al abrir el proyecto se debe configurar el archivo de configuracion "platformio.ini" para que se pueda cargar y ejecutar el programa en la esp32.

- Conectar la esp32 al computador e identificar el puerto COM de conexion que se le asigna.
- En el archivo "platformio.ini" cambiar el nombre del COM por el asignado por el computador para la esp32:

```bash
  upload_port = "Nombre del puerto COM"
```
- Guardar el archivo para asignar el nombre del puerto y que platformIO identifique las librerias necesarias y realice la descarga y acople al proyecto automaticamente:

#### LIBRERIAS UTILZADAS:
```bash
  lib_deps = 
	adafruit/Adafruit Unified Sensor@^1.1.13
	adafruit/Adafruit BMP280 Library@^2.6.8
	claws/BH1750 @ ^1.3.0
	closedcube/ClosedCube HDC1080@1.3.2
	zeed/ESP Async WebServer@^1.2.3
	links2004/WebSockets@^2.4.1
	adafruit/Adafruit SSD1306@^2.5.9
	paulstoffregen/OneWire@^2.3.8
	milesburton/DallasTemperature@^3.11.0
```

- Realizar la carga de archivos "HTML, CSS y JavaScript" a la memoria de la esp32 por medio del sistema de archivos SPIFFS:

1. En el menu de opciones de platformIO que se ubica en la barra izquierda del IDE seleccionar la opcion "Build Filesystem Image" en la siguiente ruta:
        
```bash
  platformIO => PROJECT TASKS => esp32dev => Platform => Build Filesystem Image
```

2. Luego realizar la carga con la opcion "Upload Filesystem Image" en la ruta:

```bash
  platformIO => PROJECT TASKS => esp32dev => Platform => Upload Filesystem Image
``` 

- Finalemente compilar y cargar el programa principal a la esp32:
```bash
  platformIO => PROJECT TASKS => esp32dev => General => Build
```
```bash
  platformIO => PROJECT TASKS => esp32dev => General => Upload
```
## Autores ✒️

- [@JonatanVelasquez77](https://github.com/JonatanVelasquez77)

