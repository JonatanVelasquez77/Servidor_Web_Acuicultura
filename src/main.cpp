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
#include <Adafruit_SSD1306.h> // Libreria para el controlador de pantalla OLEDz
#include <Adafruit_GFX.h>    // Libreria de gráficos básica

//*****************************************************************************************************************************************************

// ************************************************************** INICIALIZACION DE VARIABLES *********************************************************

// ****** Sensores I2C ******
// I2C = PIN 21 : DATA: SDA
// I2C = PIN 22 : CLOCK: SCL 
Adafruit_BMP280 bmp;		// Crea objeto con nombre bmp para el sensor BM280
BH1750 lightSensor(0x23); // Crea objeto con nombre lightSensor para el sensor BH1750
ClosedCube_HDC1080 hdc1080; // Crea objeto con nombre hdc1080 para el sensor HDC1080

// ****** Variables de sensores ******
float temperaturaBMP;		// Variable para almacenar valor de temperatura con sensor BMP
float presion, P0, altitud;		// Variables para almacenar valor de presion atmosferica	/ y presion actual como referencia para altitud
float lux;  // Variable para almacenar valor de la luz
float temperaturaHDC, humedad;  // Variable para almacenar valor de temperatura y humedad con sensor HDC

// ****** Variables para sensor TDS
#define TdsSensorPin 39
GravityTDS gravityTds;
float temperature = 25,tdsValue = 0;

// ***** Variables para sensor de pH y Temperatura 
const int analogInPinPH = 35;
const int analogInPinTempInt = 34;  
int buf[10];
int temp=0;
unsigned long int inValue; 

const uint8_t ledPin = 2; // Led de la tarjeta esp32
const uint16_t dataTxTimeInterval = 500; // Variable para la funcion millis()

// ****** Variables de la pantalla OLED ******
const uint8_t ancho = 128;     // Ancho de la pantalla OLED
const uint8_t alto = 64;       // Altura de la pantalla OLED
#define OLED_RESET 4           // Pin de reset para la pantalla OLED (no utilizado en este caso)
Adafruit_SSD1306 oled = Adafruit_SSD1306(ancho, alto, &Wire, OLED_RESET); // Inicialización del objeto de la pantalla OLED

// ****** Variables para la funcionalidad de los Reles ******
#define RELAY_ON 0    
#define RELAY_OFF 1

int rele_1 = 32;  // IN 1
int rele_2 = 33;  // IN 2 
int rele_3 = 25;  // IN 3
int rele_4 = 26;  // IN 4

// Tiempos de activación y desactivación de cada relé en milisegundos
const unsigned long tiempoActivacion1 = 10000; // 10 segundos
const unsigned long tiempoInactividad1 = 50000; // 50 segundos

const unsigned long tiempoActivacion2 = 25000; // 25 segundos
const unsigned long tiempoInactividad2 = 35000; // 35 segundos

const unsigned long tiempoActivacion3 = 5000; // 5 segundos
const unsigned long tiempoInactividad3 = 55000; // 55 segundos

const unsigned long tiempoActivacion4 = 40000; // 40 segundos
const unsigned long tiempoInactividad4 = 20000; // 20 segundos

// ****** Variables para la conexion con el punto de acceso WIFI ******
const char *ssid = "TPLINK_ESP32"; // Nombre de red Wifi
const char *password = "esp32-7777"; // Contrasena para acceder a la red

// ****** Incializacion para el servidor ******
AsyncWebServer server(80); // Crea objeto para servidor web asincrono en puerto 80
WebSocketsServer websockets(81); // Crea objeto para la comunicacion websocket bidireccional en puerto 81

// ********************************************************************* FUNCIONES *********************************************************************

// ****** Funcion para error de pagina ******
void notFound(AsyncWebServerRequest *request){
	request->send(404, "text/plain", "Pagina No Encontrada");
}

// ****** Funcion para la conexion del servidor ******
void webSocketEvent(uint8_t num,WStype_t type,uint8_t *payload, size_t length ){
	switch (type)
	{
	case WStype_DISCONNECTED:
		Serial.printf("[%u] Desconectado!\n", num);
		break;
	
	case WStype_CONNECTED:
	{
		IPAddress ip = websockets.remoteIP(num);
		websockets.sendTXT(num, "Conectado en servidor:");
	}
	break;
	case WStype_TEXT:{
		Serial.printf("[%u] Mensaje recibido: %s\n", num, payload);
		String msg = String((char *)(payload));

		if (msg.equalsIgnoreCase("ledon"))
			digitalWrite(2, RELAY_ON);

		if (msg.equalsIgnoreCase("ledoff"))
			digitalWrite(2, RELAY_OFF);

	}	
	default:
		break;
	}
}

// ****** Funcion para mostrar los datos en en pantalla OLED ******
void mostrarLectura(float sensor1,float sensor2, float sensor3, float sensor4, float sensor5) {
  oled.fillRect(40, 0, 80, 12, SSD1306_BLACK); // Borra el área de la pantalla
  oled.setCursor(40, 0);                         // Establece la posición del cursor
  oled.print(sensor1,1);                            // Muestra la lectura en la pantalla
  oled.setCursor(70, 0);
  oled.print("lx");

  oled.fillRect(85, 12, 40, 12, SSD1306_BLACK); // Borra el área de la pantalla
  oled.setCursor(85, 12);                         // Establece la posición del cursor
  oled.print(sensor2,1);                            // Muestra la lectura en la pantalla
  oled.setCursor(110, 12);
  oled.print("C");

  oled.fillRect(60, 24, 50, 12, SSD1306_BLACK); // Borra el área de la pantalla
  oled.setCursor(60, 24);                         // Establece la posición del cursor
  oled.print(sensor3,1);                            // Muestra la lectura en la pantalla
  oled.setCursor(90, 24);
  oled.print("%");

  oled.fillRect(60, 36, 50, 12, SSD1306_BLACK); // Borra el área de la pantalla
  oled.setCursor(60, 36);                         // Establece la posición del cursor
  oled.print(sensor4,1);                            // Muestra la lectura en la pantalla
  oled.setCursor(100, 36);
  oled.print("hPa");

  oled.fillRect(60, 48, 50, 12, SSD1306_BLACK); // Borra el área de la pantalla
  oled.setCursor(60, 48);                         // Establece la posición del cursor
  oled.print(sensor5,1);                            // Muestra la lectura en la pantalla
  oled.setCursor(90, 48);
  oled.print("m");

  oled.display();                                 // Actualiza la pantalla
}
//*****************************************************************************************************************************************************

// ********************************************************************* SETUP  ***********************************************************************

void setup() {
  Serial.begin(115200);				// Inicializa comunicacion serie a 115200 bps
  Serial.println("Iniciando:");			// Texto de inicio


  if ( !bmp.begin() ) {				// Inicia comunicacion del sensor BMP280 y si falla la comunicacion con el sensor mostrar
    Serial.println("BMP280 no encontrado !");	// texto y detener flujo del programa
    while (1);					// mediante bucle infinito
  }
  P0 = bmp.readPressure()/100;			// almacena en P0 el valor actual de presion

  lightSensor.begin(BH1750::CONTINUOUS_HIGH_RES_MODE_2); // Inicializa el sensor de luz BH1750 en modo de resolución continua y alta resolución
	if (!lightSensor.begin()) {  // Verifica si hay un error al inicializar el sensor BH1750
    Serial.println("Error al inicializar el sensor BH1750"); // texto y detener flujo del programa
    while (1); // bucle infinito
  	}
  
  hdc1080.begin(0x40); // Inicia comunicacion con sensor HDC1080

  gravityTds.setPin(TdsSensorPin);
  gravityTds.setAref(5.0);          //Voltaje de referencia en el ADC, por defecto 5.0V 
  gravityTds.setAdcRange(1024);     //1024 para 10bit ADC;4096 para 12bit ADC
  gravityTds.begin();               //inicializamos sensor TDS

  WiFi.begin(ssid, password); // Inicia la comunicacion Wifi

	while (WiFi.status() != WL_CONNECTED) { // Verificacion de conexion
      delay(1000);
      Serial.println("Conectando al Servidor...");
  }
  Serial.println("Conexión exitosa");

  // Imprime la dirección IP asignada al Arduino
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());

  pinMode(ledPin,OUTPUT); // LED APAGADO

	//Definir los pines como salida
  pinMode (rele_1, OUTPUT); // pin rele 1
  pinMode (rele_2, OUTPUT); // pin rele 2
  pinMode (rele_3, OUTPUT); // pin rele 3
  pinMode (rele_4, OUTPUT); // pin rele 4
  

  if (!SPIFFS.begin(true)){  // Inicia montaje de sistema SPIFFS
    Serial.println("Error al montar SPIFFS");
    return;
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
        {request->send(SPIFFS, "/Pagina.html","text/html");}); // Envio para archivo pagina HTML
  
  server.on("/estilo.css", HTTP_GET, [](AsyncWebServerRequest *request){
          request->send(SPIFFS, "/estilo.css", "text/css");}); // Envio para archivo CSS

  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
          request->send(SPIFFS, "/script.js", "text/js");}); // Envio para archivo Java script

  server.on("/Fondo.jpg", HTTP_GET, [](AsyncWebServerRequest *request){
     			request->send(SPIFFS, "/Fondo.jpg", "text/jpg");}); // Envio para archivo jpg

  server.onNotFound(notFound); // Inicio para la funcion de pagina 
  server.begin(); // Inicia el servidor

  websockets.begin(); // Inicia websockets
  websockets.onEvent(webSocketEvent); // Encendido de funcion webSocketEvent

  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Inicialización de la pantalla OLED
  delay(1000);
  oled.clearDisplay();  // Borra el contenido de la pantalla
  oled.display();       // Actualiza la pantalla

  oled.setTextSize(1);  // Configura el tamaño del texto
  oled.setTextColor(SSD1306_WHITE); // Configura el color del texto

  oled.setCursor(10, 0);  // Establece la posición del cursor
  oled.print("Luz: "); // Muestra un texto en la pantalla

  oled.setCursor(10, 12);  // Establece la posición del cursor
  oled.print("Temperatura:"); // Muestra un texto en la pantalla

  oled.setCursor(10, 24);  // Establece la posición del cursor
  oled.print("Humedad:"); // Muestra un texto en la pantalla

  oled.setCursor(10, 36);  // Establece la posición del cursor
  oled.print("Presion:"); // Muestra un texto en la pantalla

  oled.setCursor(10, 48);  // Establece la posición del cursor
  oled.print("Altitud:"); // Muestra un texto en la pantalla

  oled.display();         // Actualiza la pantalla

}						

//*****************************************************************************************************************************************************

// ********************************************************************* LOOP  ***********************************************************************

void loop() {
  websockets.loop(); // Bucle para la funcion websocket
  static uint32_t prevMillis = 0; // Variable para la funcion millis()
  if(millis() - prevMillis >= dataTxTimeInterval){ // Condicion para realizar las lecturas en tiempo determinado
		prevMillis = millis(); 

		lux = lightSensor.readLightLevel(); // Lee el nivel de luz actual del sensor BH1750

		temperaturaHDC = hdc1080.readTemperature(); // Lee el nivel de Temperatura actual del sensor hdc1080
  	humedad = hdc1080.readHumidity();	// Lee el nivel de Humedad actual del sensor hdc1080

		temperaturaBMP = bmp.readTemperature();		// Almacena en variable el valor de temperatura
    presion = bmp.readPressure()/100;		// Almacena en variable el valor de presion divido // por 100 para covertirlo a hectopascales
	
		altitud = bmp.readAltitude(P0);		// Muestra valor de altitud con referencia a P0

    gravityTds.setTemperature(temperature);     // Establece la temperatura y ejecuta la compensación de temperatura.
    gravityTds.update();                        // Muestrea y calcula.
    tdsValue = gravityTds.getTdsValue();        // Luego, obtén el valor.

    // ****************** SENSOR DE pH y Temperatura
  //   /*para evitar demasiadas variaciones lecturaremos 
  // unos varios valores y luego sacaremos el promedio*/
    for(int i=0; i<10;i++){
      /*realizamos 10 lecturasy almacenamos en buff*/
        buf[i]= analogRead(analogInPinPH);
        delay(10);
    }
  //  /*luego realizamos un barrido de los valores lecturados
  //   * y descartamos los valores demasiado elevados y los 
  //   * valoresdemasiado bajos*/
    for(int i=0; i<9; i++){
      for(int j=i+1;j<10;j++){
        temp= buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
      }  
    }
  //   /*realicamos el calculo del promedio y la conversion
  //   a voltaje en mv*/
    inValue=0;
    for(int i=2; i<8; i++){
        inValue= inValue + buf[i];
    }
    float PHVol= (float)inValue*5/1024/6;

    float lectTemp =  analogRead(analogInPinTempInt);    
    float miliV_tem = 5.0 /1024 * lectTemp ;
    float temperaturaInt = miliV_tem; 

    float PH= -0.57*PHVol +21.338;

    mostrarLectura(lux,temperaturaHDC,humedad,presion,altitud); // Muestra los datos de los sensores en la pantalla OLED

    // Se recolectan los datos en un String para ser enviados
		//String	data = "{\"Luz\": "+ String(lux) +", \"Temperatura\": "+ String(temperaturaHDC)+", \"Humedad\": "+ String(humedad) +", \"Presion\": "+ String(presion) +", \"Altitud\": "+ String(altitud) +"}";
		String	data = "{\"Luz\": "+ String(lux) +", \"Temperatura\": "+ String(temperaturaHDC)+", \"Humedad\": "+ String(humedad) +", \"Presion\": "+ String(presion) +", \"Altitud\": "+ String(altitud) +", \"pH\": "+ String(PH) +",\"TempInt\": "+ String(temperaturaInt) +", \"tds\": "+ String(tdsValue)+"}";
    //String	data = "{\"Luz\": "+ String(lux) +", \"Temperatura\": "+ String(temperatura)+", \"Humedad\": "+ String(humedad) +"}";

		websockets.broadcastTXT(data); // Se envian los datos por protocolo websocket

		Serial.println(data); // Se imprimen los datos en la terminal
	}

  unsigned long tiempoActual = millis(); // Variable para funcion millis()

  // Control de relé 1
  if (tiempoActual % (tiempoActivacion1 + tiempoInactividad1) < tiempoActivacion1) {
    digitalWrite(rele_1, RELAY_ON); // Activa el relé 1
  } else {
    digitalWrite(rele_1, RELAY_OFF); // Desactiva el relé 1
  }

  // Control de relé 2
  if (tiempoActual % (tiempoActivacion2 + tiempoInactividad2) < tiempoActivacion2) {
    digitalWrite(rele_2, RELAY_ON); // Activa el relé 2
  } else {
    digitalWrite(rele_2, RELAY_OFF); // Desactiva el relé 2
  }

  // Control de relé 3
  if (tiempoActual % (tiempoActivacion3 + tiempoInactividad3) < tiempoActivacion3) {
    digitalWrite(rele_3, RELAY_ON); // Activa el relé 3
  } else {
    digitalWrite(rele_3, RELAY_OFF); // Desactiva el relé 3
  }

  // Control de relé 4
  if (tiempoActual % (tiempoActivacion4 + tiempoInactividad4) < tiempoActivacion4) {
    digitalWrite(rele_4, RELAY_ON); // Activa el relé 4
  } else {
    digitalWrite(rele_4, RELAY_OFF); // Desactiva el relé 4
  }

}

//*****************************************************************************************************************************************************