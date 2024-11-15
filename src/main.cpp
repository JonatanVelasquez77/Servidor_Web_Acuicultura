// ********************************************************************* LIBRERIAS *********************************************************************

#include <Arduino.h> // Libreria para la adecuacion con Arduino
#include <WiFi.h> // Libreria para la conexion Wifi
#include <AsyncEventSource.h> // Libreria para los eventos del servidor
#include <SPIFFS.h> // Libreria para la carga de archivos con SPIFFS
#include <WebSocketsServer.h> // Libreria para la comunicacion con websockets en el servidor 
#include <Wire.h> // Libreria para comunicacion I2C
#include <BH1750.h> // libreria sensor BH1750
#include "ClosedCube_HDC1080.h" // Libreria sensor HDC1080
#include <Adafruit_Sensor.h>  // Libreria para lectura de sensores
#include <Adafruit_BMP280.h>  // Libreria sensor BMP280
#include "GravityTDS.h"   // Libreria para el sensor TDS 
#include <Adafruit_SSD1306.h> // Libreria para el controlador de pantalla OLED
#include <Adafruit_GFX.h>    // Libreria de gráficos básica
#include <OneWire.h>    // Libreria de comunicacion 
#include <DallasTemperature.h>  // Libreria para el sensor de Temperatura Interna

//*****************************************************************************************************************************************************

// ************************************************************** INICIALIZACION DE VARIABLES *********************************************************

// ****** Sensores I2C ******
// I2C = PIN 21 : DATA: SDA
// I2C = PIN 22 : CLOCK: SCL

// ***** Configuración de pines de los sensores y relés *****
#define TdsSensorPin 33     //Pin 33 para lectura de TDS
#define analogInPinPH 35    //Pin 35 para lectura de pH
#define ONE_WIRE_BUS 14     //Pin 14 para lectura de Temperatura Interna
#define ADC_VOLTAGE_REF 5.0   //Referencia de voltaje 5v 
#define ADC_RESOLUTION 1024   //Resolucion de 1024 bits
#define RELAY_ON LOW          // Rele encendido
#define RELAY_OFF HIGH        // Rele Apagado
const int rele_1 = 25, rele_2 = 26, rele_3 = 27, rele_4 = 32;  //Pines para los 4 reles respectivamente

// ***** Configuración de límites para TDS *****
const float tdsMin = 400.0;  // Valor mínimo de TDS para activar la bomba
const float tdsMax = 800.0;  // Valor máximo de TDS para desactivar la bomba
bool releTdsAutomatico = false; // Indica si el relé está activado automáticamente por TDS

// ***** Configuración de límites para Temperatura Interna *****
const float tempIntMin = 22.0; // Valor mínimo de temperatura para activar el aireador
const float tempIntMax = 28.0; // Valor máximo de temperatura para desactivar el aireador
bool releTempAutomatico = false; // Indica si el relé está activado automáticamente por temperatura


// ***** Configuración de intervalos de tiempo *****
const uint16_t dataTxTimeInterval = 500;
const unsigned long wifiTimeout = 10000;    // Tiempo máximo de espera para la conexión (en milisegundos)

// ***** Configuración de sensores *****
Adafruit_BMP280 bmp;    // Crea objeto con nombre bmp para el sensor BM280
BH1750 lightSensor(0x23);   // Crea objeto con nombre lightSensor para el sensor BH1750
ClosedCube_HDC1080 hdc1080;   // Crea objeto con nombre hdc1080 para el sensor HDC1080
GravityTDS gravityTds;      // Crea objeto con nombre gravityTds para el sensor TDS
OneWire oneWire(ONE_WIRE_BUS);    // Crea objeto con nombre oneWire para el sensor de Temperatura Interna
DallasTemperature sensors(&oneWire);

// ***** Variables de lectura de sensores *****
float temperaturaHDC, humedad, temperaturaBMP, presion, altitud, lux, P0, PH, temperaturaInt;
float tdsValue = 0;
float buf[10];
unsigned long previousMillis = 0, interval = 5000;
bool showFirstSet = true;

// ***** Configuración de pantalla OLED *****
#define SCREEN_WIDTH 128    // Ancho de la pantalla OLED
#define SCREEN_HEIGHT 64    // Altura de la pantalla OLED
Adafruit_SSD1306 oled = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);   // Inicialización del objeto de la pantalla OLED

// ***** Variables para el control de los Reles *****
bool rele1On = false, rele2On = false, rele3On = false, rele4On = false;
bool rele1Manual = false, rele2Manual = false, rele3Manual = false, rele4Manual = false;
unsigned long rele1StartTime, rele2StartTime, rele3StartTime, rele4StartTime;
int tiempoActivacion1 = 10000, tiempoInactividad1 = 50000;
int tiempoActivacion2 = 25000, tiempoInactividad2 = 35000;
int tiempoActivacion3 = 5000, tiempoInactividad3 = 55000;
int tiempoActivacion4 = 40000, tiempoInactividad4 = 20000;

// ****** Variables para la conexion con el punto de acceso WIFI ******
const char *ssid = "TPLINK_ESP32"; // Nombre de red Wifi
const char *password = "esp32-7777"; // Contrasena para acceder a la red

// ****** Incializacion para el servidor ******
AsyncWebServer server(80); // Crea objeto para servidor web asincrono en puerto 80
WebSocketsServer websockets(81); // Crea objeto para la comunicacion websocket bidireccional en puerto 81

// ********************************************************************* FUNCIONES *********************************************************************

// ***** Función para leer los datos de los sensores *****
void leerSensores() {
  lux = lightSensor.readLightLevel();
  temperaturaHDC = hdc1080.readTemperature();
  humedad = hdc1080.readHumidity();
  temperaturaBMP = bmp.readTemperature();
  presion = bmp.readPressure() / 100;
  altitud = bmp.readAltitude(P0);
  sensors.requestTemperatures();
  temperaturaInt = sensors.getTempCByIndex(0);
  gravityTds.setTemperature(temperaturaInt);
  gravityTds.update();
  tdsValue = gravityTds.getTdsValue();

  // Cálculo de pH
  int inValue = 0;
  for (int i = 0; i < 10; i++) {
    buf[i] = analogRead(analogInPinPH);
    delay(10);
  }
  for (int i = 2; i < 8; i++) {
    inValue += buf[i];
  }
  float PHVol = (float)inValue * ADC_VOLTAGE_REF / ADC_RESOLUTION / 6;
  PH = -0.57 * PHVol + 21.338;

  
}

// ***** Funcion para Enviar datos a través del WebSocket *****
void enviarDatosWebSocket() {
  String data = "{\"Luz\": " + String(lux) + ", \"Temperatura\": " + String(temperaturaHDC) +
                ", \"Humedad\": " + String(humedad) + ", \"Presion\": " + String(presion) +
                ", \"Altitud\": " + String(altitud) + ", \"pH\": " + String(PH) +
                ",\"TempInt\": " + String(temperaturaInt) + ", \"tds\": " + String(tdsValue);

  unsigned long tiempoActual = millis();
  data += ", \"rele1Time\": " + String(rele1On ? (tiempoActual - rele1StartTime) / 1000 : 0);
  data += ", \"rele2Time\": " + String(rele2On ? (tiempoActual - rele2StartTime) / 1000 : 0);
  data += ", \"rele3Time\": " + String(rele3On ? (tiempoActual - rele3StartTime) / 1000 : 0);
  data += ", \"rele4Time\": " + String(rele4On ? (tiempoActual - rele4StartTime) / 1000 : 0);
  data += "}";

  websockets.broadcastTXT(data);
  Serial.println(data);
}

// ***** Función para manejar eventos WebSocket *****
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Desconectado!\n", num);
      break;

    case WStype_CONNECTED:
    {
      IPAddress ip = websockets.remoteIP(num);
      Serial.printf("[%u] Conectado desde %s\n", num, ip.toString().c_str());
      websockets.sendTXT(num, "Conectado en servidor:");
    }
    break;

    case WStype_TEXT: {
      Serial.printf("[%u] Mensaje recibido: %s\n", num, payload);
      String msg = String((char*)payload);
      if (msg == "rele1on") { rele1On = true; rele1Manual = true; rele1StartTime = millis(); }
      if (msg == "rele1off") { rele1On = false; rele1Manual = true; }
      if (msg == "rele2on") { rele2On = true; rele2Manual = true; rele2StartTime = millis(); }
      if (msg == "rele2off") { rele2On = false; rele2Manual = true; }
      if (msg == "rele3on") { rele3On = true; rele3Manual = true; rele3StartTime = millis(); }
      if (msg == "rele3off") { rele3On = false; rele3Manual = true; }
      if (msg == "rele4on") { rele4On = true; rele4Manual = true; rele4StartTime = millis(); }
      if (msg == "rele4off") { rele4On = false; rele4Manual = true; }
    }
    break;
  default:
    break;
  }
}

// ***** Funcion para Controlar el estado de cada relé *****
void controlarRele(unsigned long tiempoActual, int relePin, bool &releOn, bool &releManual, int tiempoAct, int tiempoInact, bool isTdsControlled = false, bool isTempControlled = false) {
  if (isTdsControlled && releTdsAutomatico) {
    // Control por TDS
    if (tdsValue < tdsMin) {
      digitalWrite(relePin, RELAY_ON);
      releOn = true;
    } else if (tdsValue > tdsMax) {
      digitalWrite(relePin, RELAY_OFF);
      releOn = false;
      releTdsAutomatico = false;
    }
  } else if (isTempControlled && releTempAutomatico) {
    // Control por Temperatura Interna
    if (temperaturaInt > tempIntMax) {
      digitalWrite(relePin, RELAY_ON);
      releOn = true;
    } else if (temperaturaInt < tempIntMin) {
      digitalWrite(relePin, RELAY_OFF);
      releOn = false;
      releTempAutomatico = false;
    }
  } else if (!releManual) {
    // Control automático general
    if (tiempoActual % (tiempoAct + tiempoInact) < tiempoAct) {
      digitalWrite(relePin, RELAY_ON);
      releOn = true;
    } else {
      digitalWrite(relePin, RELAY_OFF);
      releOn = false;
    }
  } else {
    // Control manual
    digitalWrite(relePin, releOn ? RELAY_ON : RELAY_OFF);
  }
}

// ***** Funcion de Control de reles *****
void controlarReles() {
  unsigned long tiempoActual = millis();

  // Control del relé 1 basado en TDS
  if (!rele1Manual) { // Solo si el relé no está en modo manual
    if (tdsValue >= tdsMin && tdsValue <= tdsMax) {
      releTdsAutomatico = true; // Activa el control automático por TDS
    }
  }
  controlarRele(tiempoActual, rele_1, rele1On, rele1Manual, tiempoActivacion1, tiempoInactividad1, true);

  // Control del relé 2 basado en Temperatura Interna
  if (!rele2Manual) { // Solo si el relé no está en modo manual
    if (temperaturaInt > tempIntMax) {
      releTempAutomatico = true; // Activa el control automático por Temperatura Interna
    }
  }
  controlarRele(tiempoActual, rele_2, rele2On, rele2Manual, tiempoActivacion2, tiempoInactividad2, false, true);

  // Control de los demás relés con lógica preexistente
  controlarRele(tiempoActual, rele_3, rele3On, rele3Manual, tiempoActivacion3, tiempoInactividad3);
  controlarRele(tiempoActual, rele_4, rele4On, rele4Manual, tiempoActivacion4, tiempoInactividad4);
}


// ****** Funcion para mostrar los datos en en pantalla OLED ******
void mostrarLectura(float sensor1,float sensor2, float sensor3, float sensor4, float sensor5) {
  oled.clearDisplay();
  oled.fillRect(40, 0, 80, 12, SSD1306_BLACK); // Borra el área de la pantalla
  oled.setCursor(10, 0);  // Establece la posición del cursor
  oled.print("Luz: "); // Muestra un texto en la pantalla
  oled.setCursor(40, 0);                         // Establece la posición del cursor
  oled.print(sensor1,1);                            // Muestra la lectura en la pantalla
  oled.setCursor(70, 0);
  oled.print("lx");

  oled.fillRect(85, 12, 40, 12, SSD1306_BLACK); // Borra el área de la pantalla
  oled.setCursor(10, 12);  // Establece la posición del cursor
  oled.print("Temperatura:"); // Muestra un texto en la pantalla
  oled.setCursor(85, 12);                         // Establece la posición del cursor
  oled.print(sensor2,1);                            // Muestra la lectura en la pantalla
  oled.setCursor(110, 12);
  oled.print("C");

  oled.fillRect(60, 24, 50, 12, SSD1306_BLACK); // Borra el área de la pantalla
  oled.setCursor(10, 24);  // Establece la posición del cursor
  oled.print("Humedad:"); // Muestra un texto en la pantalla
  oled.setCursor(60, 24);                         // Establece la posición del cursor
  oled.print(sensor3,1);                            // Muestra la lectura en la pantalla
  oled.setCursor(90, 24);
  oled.print("%");

  oled.fillRect(60, 36, 50, 12, SSD1306_BLACK); // Borra el área de la pantalla
  oled.setCursor(10, 36);  // Establece la posición del cursor
  oled.print("Presion:"); // Muestra un texto en la pantalla
  oled.setCursor(60, 36);                         // Establece la posición del cursor
  oled.print(sensor4,1);                            // Muestra la lectura en la pantalla
  oled.setCursor(100, 36);
  oled.print("hPa");

  oled.fillRect(60, 48, 50, 12, SSD1306_BLACK); // Borra el área de la pantalla
  oled.setCursor(10, 48);  // Establece la posición del cursor
  oled.print("Altitud:"); // Muestra un texto en la pantalla
  oled.setCursor(60, 48);                         // Establece la posición del cursor
  oled.print(sensor5,1);                            // Muestra la lectura en la pantalla
  oled.setCursor(90, 48);
  oled.print("m");

  oled.display();                                 // Actualiza la pantalla
}

void mostrarLecturaTres(float sensor6,float sensor7, float sensor8) {
  oled.clearDisplay();
  oled.fillRect(40, 0, 80, 12, SSD1306_BLACK); // Borra el área de la pantalla
  oled.setCursor(10, 0);  // Establece la posición del cursor
  oled.print("pH: "); // Muestra un texto en la pantalla
  oled.setCursor(40, 0);                         // Establece la posición del cursor
  oled.print(sensor6,1);                            // Muestra la lectura en la pantalla
  oled.setCursor(70, 0);
  oled.print("pH");

  oled.fillRect(85, 12, 40, 12, SSD1306_BLACK); // Borra el área de la pantalla
  oled.setCursor(10, 12);  // Establece la posición del cursor
  oled.print("Temperatura:"); // Muestra un texto en la pantalla
  oled.setCursor(85, 12);                         // Establece la posición del cursor
  oled.print(sensor7,1);                            // Muestra la lectura en la pantalla
  oled.setCursor(110, 12);
  oled.print("C");

  oled.fillRect(60, 24, 50, 12, SSD1306_BLACK); // Borra el área de la pantalla
  oled.setCursor(10, 24);  // Establece la posición del cursor
  oled.print("TDS:"); // Muestra un texto en la pantalla
  oled.setCursor(60, 24);                         // Establece la posición del cursor
  oled.print(sensor8,1);                            // Muestra la lectura en la pantalla
  oled.setCursor(90, 24);
  oled.print("ppm");

  oled.display();                                 // Actualiza la pantalla
}

// ***** Funcion para actualizar la pantalla OLED *****
void actualizarPantallaOLED(){
  // Alternar entre los dos conjuntos de lecturas
  if (millis() - previousMillis >= interval) {
      previousMillis = millis();
      showFirstSet = !showFirstSet;
  }

  if (showFirstSet) {
      mostrarLectura(lux, temperaturaHDC, humedad, presion, altitud);
  } else {
      mostrarLecturaTres(PH, temperaturaInt, tdsValue);
  }
}

// ********************************************************************* SETUP  ***********************************************************************
void setup() {
  Wire.begin();
  Serial.begin(115200);    // Inicializa comunicacion serie a 115200 bps
  Serial.println("Iniciando:");   // Texto de inicio

  // Conexión a WiFi

  // Activar WiFi en modo estación y desconectar cualquier conexión previa
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Intento de conexión WiFi
  Serial.println("Conectando a WiFi...");
  WiFi.begin(ssid, password);

  unsigned long startAttemptTime = millis();

  // Esperar hasta que se conecte o hasta que se agote el tiempo de espera
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < wifiTimeout) {
    delay(500);
    Serial.print(".");
  }

  // Verificar si se ha conectado
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConectado a WiFi");
    Serial.print("Dirección IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nNo se pudo conectar a WiFi. Tiempo de espera agotado.");
  }

  // Iniciar SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Error al montar SPIFFS");
    return;
  }
  // Configuración de rutas para cargar archivos
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/Pagina.html", "text/html");
  });
  server.on("/estilo.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/estilo.css", "text/css");
  });
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/script.js", "application/javascript");
  });
  server.on("/Fondo.jpg", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/Fondo.jpg", "image/jpeg");
  });

  // Manejador de error 404 para rutas no encontradas
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "404: Not Found");
  });

  // Iniciar el servidor
  server.begin();

  // Iniciar WebSocket
  websockets.begin();
  websockets.onEvent(webSocketEvent);


  // Inicialización de BMP280
  if (!bmp.begin()) {
    Serial.println("Error al iniciar el sensor BMP280");
    while (1); // Detiene el programa si el sensor no se inicializa
  }

  // Inicialización de cada sensor con verificación de errores
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Modo de operación. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Sobremuestreo de temperatura */
                  Adafruit_BMP280::SAMPLING_X16,    /* Sobremuestreo de presión */
                  Adafruit_BMP280::FILTER_X16,      /* Filtro. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Tiempo de espera. */
  
  P0 = bmp.readPressure() / 100;
  
  // Inicializa el sensor de luz BH1750 en modo de resolución continua y alta resolución
  if (!lightSensor.begin(BH1750::CONTINUOUS_HIGH_RES_MODE_2)) {
    Serial.println("Error al inicializar el sensor BH1750");
    while (1);
  }

  // Inicia comunicacion con sensor HDC1080
  hdc1080.begin(0x40);
  
  // Inicia el seteo y comunicacion con el sensor TDS
  gravityTds.setPin(TdsSensorPin);
  gravityTds.setAref(ADC_VOLTAGE_REF);
  gravityTds.setAdcRange(ADC_RESOLUTION);
  gravityTds.begin();

  // Inicia el seteo y comunicacion con el sensor de Temperatura Interna
  sensors.begin();

  // Configuración de pantalla OLED
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Inicialización de la pantalla OLED
  delay(1000);
  oled.clearDisplay();  // Borra el contenido de la pantalla
  oled.display();       // Actualiza la pantalla

  oled.setTextSize(1);  // Configura el tamaño del texto
  oled.setTextColor(SSD1306_WHITE); // Configura el color del texto

  // Configuración inicial para pines de los relés
  pinMode(rele_1, OUTPUT);
  pinMode(rele_2, OUTPUT);
  pinMode(rele_3, OUTPUT);
  pinMode(rele_4, OUTPUT);
  digitalWrite(rele_1, RELAY_OFF);
  digitalWrite(rele_2, RELAY_OFF);
  digitalWrite(rele_3, RELAY_OFF);
  digitalWrite(rele_4, RELAY_OFF);
}

// ********************************************************************* LOOP  ***********************************************************************

void loop() {
  websockets.loop(); // Bucle para la función WebSocket

  static uint32_t prevMillis = 0;   // Variable para la función millis()

  if (millis() - prevMillis >= dataTxTimeInterval) { // Condición para realizar las lecturas en tiempo determinado
    prevMillis = millis();

    leerSensores();             // Leer sensores y calcular valores
    enviarDatosWebSocket();     // Enviar datos a través del WebSocket
    actualizarPantallaOLED();   // Mostrar datos en pantalla OLED
  }

  controlarReles(); // Control de relés
}

//*****************************************************************************************************************************************************
