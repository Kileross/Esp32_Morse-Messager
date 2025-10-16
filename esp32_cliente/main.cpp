#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "RTClib.h"
#include <WiFi.h>
              
// Configuración de la pantalla OLED
#define SCREEN_WIDTH 128 // Ancho de la pantalla OLED
#define SCREEN_HEIGHT 64 // Altura de la pantalla OLED
#define OLED_RESET -1    // Pin de reset (usualmente no se utiliza con I2C)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Configuración del pulsador
#define BUTTON_PIN 16    // Pin GPIO al que está conectado el pulsador
#define LED_PIN 17       // Pin GPIO al que está conectado el LED

// Variables adicionales
String outgoingMessage = ""; // Mensaje a enviar al servidor

// Variables para manejo de Morse
String morseInput = "";    // Guarda la secuencia Morse temporal
String decodedText = "";   // Guarda el texto decodificado
unsigned long pressStart = 0; // Momento en que se presionó el pulsador
bool isPressed = false;       // Indica si el pulsador está actualmente presionado
unsigned long lastInputTime = 0; // Tiempo de la última entrada Morse

// Configuración de Wi-Fi
const char ssid = "";  // Nombre de la red Wi-Fi
const char password = "";  // Contraseña de la red Wi-Fi
const char serverIP = "";  // IP de la Raspberry Pi
const int serverPort = 12500;  // Puerto para la conexión
bool connected = NULL; // Indica si el cliente está conectado al servidor

// Variables para conexión Wi-Fi y comunicación
WiFiClient client;
String receivedMessage = "";  // Mensaje recibido desde el servidor
String destino = "RaspberryPi:"; // Nombre del cliente
String name = "Esp32-kileros"; // Nombre del cliente

RTC_DS3231 rtc;

// Prototipos de funciones
void decodeMorse(); // Decodifica la entrada Morse actual
void displayContent(bool buttonState, String receivedMessage, bool wifiConnected); // Actualiza la pantalla con los contenidos
void resetDecodedText(); // Resetea el texto decodificado
void sendMessage(const String &message);
void reconnectServer();

void setup() {
  // Inicialización de la pantalla
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;); // Detén el programa si no se inicializa la pantalla
  }
  
  display.clearDisplay();
  display.setTextSize(1);          // Tamaño del texto
  display.setTextColor(SSD1306_WHITE); // Color del texto
  display.display();
  delay(2000);
  
  // Configuración del pulsador
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Configura el pin como entrada con resistencia pull-up interna
  // Configuración del LED
  pinMode(LED_PIN, OUTPUT);         // Configura el pin como salida
  digitalWrite(LED_PIN, LOW);       // Asegúrate de que el LED está apagado al inicio

  // Conexión a la red Wi-Fi
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a Wi-Fi...");
  }
  
  if (connected = client.connect(serverIP, serverPort)) {
    Serial.println("Connected to server");

    // Enviar el nombre del cliente al servidor
    client.println(name);

    // Enviar un mensaje a un cliente específico
    //String message = "RaspberryPi:Hello Raspy!";
    //client.println(message);
  } else {
    Serial.println("Failed to connect to server");
  }

/*
if (! rtc.begin()) {
    Serial.println("Could not find RTC! Check circuit.");
    while (1);
  }

  rtc.adjust(DateTime(__DATE__, __TIME__));
*/

}



void loop() {
  // Verifica la conexión Wi-Fi
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Reintentando conexión Wi-Fi...");
    }
    Serial.println("Wi-Fi reconectado.");
  }

  // Verifica la conexión al servidor
        // Verificar conexión al servidor
    /*
    if (!client.connected()) {
        reconnectServer();
    }
    */

/*
    DateTime now = rtc.now();
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
*/ 

  // Lee el estado del pulsador
  bool buttonState = digitalRead(BUTTON_PIN) == LOW;

  // Actualiza el estado del LED
  digitalWrite(LED_PIN, buttonState ? HIGH : LOW);

  // Detecta cambios en el estado del pulsador
  if (buttonState && !isPressed) {
    isPressed = true;
    pressStart = millis(); // Marca el inicio de la pulsación
  } else if (!buttonState && isPressed) {
    isPressed = false;
    unsigned long pressDuration = millis() - pressStart;

    // Decodifica la duración del pulso como un punto o una raya
    if (pressDuration < 300) {
      morseInput += "."; // Punto
    } else if (pressDuration < 1000) {
      morseInput += "-"; // Raya
    } else if (pressDuration >= 1000) {
      // Detecta un espacio largo como separación entre letras
      decodeMorse();
      morseInput = ""; // Resetea la entrada temporal
    }

    lastInputTime = millis(); // Actualiza el tiempo de la última entrada
  }
  else if (morseInput.length() > 0 && millis() - lastInputTime > 2000) { // Si pasaron 2 segundos sin recibir entradas, decodifica la letra
    decodeMorse();  // Decodificar la letra actual
    morseInput = ""; // Resetea la entrada temporal
  }
  else if (buttonState && (millis() - pressStart >= 3000)) { // Resetea el texto decodificado si se mantiene pulsado más de 5 segundos
    resetDecodedText();
  }

  // Recibir mensaje del servidor
  if (client.available()) {
    receivedMessage = client.readStringUntil('\n');
    Serial.println("Mensaje recibido: " + receivedMessage);
  }

  // Verifica el estado de la conexión Wi-Fi
  bool wifiConnected = WiFi.status() == WL_CONNECTED;

  // Actualizar la pantalla con el estado del pulsador y el mensaje recibido
  displayContent(buttonState, receivedMessage, wifiConnected);
}

// Función para decodificar la entrada Morse actual
void decodeMorse() {
  if (morseInput == ".-") {
      decodedText += "A";
  } else if (morseInput == "-...") {
      decodedText += "B";
  } else if (morseInput == "-.-.") {
      decodedText += "C";
  } else if (morseInput == "-..") {
      decodedText += "D";
  } else if (morseInput == ".") {
      decodedText += "E";
  } else if (morseInput == "..-.") {
      decodedText += "F";
  } else if (morseInput == "--.") {
      decodedText += "G";
  } else if (morseInput == "....") {
      decodedText += "H";
  } else if (morseInput == "..") {
      decodedText += "I";
  } else if (morseInput == ".---") {
      decodedText += "J";
  } else if (morseInput == "-.-") {
      decodedText += "K";
  } else if (morseInput == ".-..") {
      decodedText += "L";
  } else if (morseInput == "--") {
      decodedText += "M";
  } else if (morseInput == "-.") {
      decodedText += "N";
  } else if (morseInput == "---") {
      decodedText += "O";
  } else if (morseInput == ".--.") {
      decodedText += "P";
  } else if (morseInput == "--.-") {
      decodedText += "Q";
  } else if (morseInput == ".-.") {
      decodedText += "R";
  } else if (morseInput == "...") {
      decodedText += "S";
  } else if (morseInput == "-") {
      decodedText += "T";
  } else if (morseInput == "..-") {
      decodedText += "U";
  } else if (morseInput == "...-") {
      decodedText += "V";
  } else if (morseInput == ".--") {
      decodedText += "W";
  } else if (morseInput == "-..-") {
      decodedText += "X";
  } else if (morseInput == "-.--") {
      decodedText += "Y";
  } else if (morseInput == "--..") {
      decodedText += "Z";
  } else if (morseInput == ".----") {
      decodedText += "1";
  } else if (morseInput == "..---") {
      decodedText += "2";
  } else if (morseInput == "...--") {
      decodedText += "3";
  } else if (morseInput == "....-") {
      decodedText += "4";
  } else if (morseInput == ".....") {
      decodedText += "5";
  } else if (morseInput == "-....") {
      decodedText += "6";
  } else if (morseInput == "--...") {
      decodedText += "7";
  } else if (morseInput == "---..") {
      decodedText += "8";
  } else if (morseInput == "----.") {
      decodedText += "9";
  } else if (morseInput == "-----") {
      decodedText += "0";
  } else if (morseInput == ".-.-.-") {
      decodedText += ".";
  } else if (morseInput == "--..--") {
      decodedText += ",";
  } else if (morseInput == "..--..") {
      decodedText += "?";
  } else if (morseInput == "-....-") {
      decodedText += "-";
  } else if (morseInput == "-.--.") {
      decodedText += "(";
  } else if (morseInput == "-.--.-") {
      decodedText += ")";
  } else if (morseInput == ".----.") {
      decodedText += "'";
  } else if (morseInput == "-.-.--") {
      decodedText += "!";
  } else if (morseInput == "-..-.") {
      decodedText += "/";
  } else if (morseInput == "-...-") {
      decodedText += "=";
  } else if (morseInput == ".-.-.") {
      decodedText += "+";
  } else if (morseInput == ".-..-.") {
      decodedText += "\"";
  } else if (morseInput == "...-.-") {
      sendMessage(decodedText);
      decodedText = "";
      decodedText += "End of work: Sending";
  } else if (morseInput == "...---...") {
      decodedText += "SOS";
  } else if (morseInput == "-.-.-.") {
      decodedText += "Starting signal";
  } else if (morseInput == ".-...") {
      decodedText += "Understood";
  } else if (morseInput == "-.-.-") {
      decodedText += "Invitation to transmit";
  } else if (morseInput == "-...-") {
      decodedText += "Error";
  } else if (morseInput == "..-..") {
      decodedText += " ";  
  } else if (morseInput == "-.-..-") {
      decodedText += "Te quiero <3";  
  } else if (morseInput == "...---") {
      decodedText += "Reconectando Servidor";
      reconnectServer();
  } else {
      decodedText += ""; // Para entradas Morse no reconocidas
    }
}

// Función para actualizar la pantalla
void displayContent(bool buttonState, String receivedMessage, bool wifiConnected) {
  display.clearDisplay();

  // Texto pequeño arriba con el estado del botón
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(buttonState ? "PRESIONADO" : "NO PRESIONADO");

  // Indicador de si el wifi está activo o no 
  display.setCursor(116, 0);
  display.print(wifiConnected ? "#" : "!");

  display.setCursor(90, 0);
  display.print(connected ? "X" : "!X");

  // Línea para dividir emisario y receptor
  display.setCursor(0, 40);
  display.println("=====================");
  
  // Texto intermedio con la entrada Morse actual
  display.setCursor(0, 16);
  display.println(morseInput);

  // Texto pequeño abajo con el texto decodificado
  display.setCursor(0, 32);
  display.println(decodedText);

  display.setCursor(0, 48);
  display.println(receivedMessage);

  display.display();
}

// Función para resetear el texto decodificado
void resetDecodedText() {
  decodedText = "";
  morseInput = "";
}

void sendMessage(const String &message) {
    if (client.connected()) {
        client.println(destino + message);
        Serial.println("Mensaje enviado: " + message);
    } else {
        Serial.println("No se pudo enviar el mensaje, servidor desconectado");
        connected = false;
    }
}


void reconnectServer() {
  if (connected = client.connect(serverIP, serverPort)) {
    Serial.println("Connected to server");

    // Enviar el nombre del cliente al servidor 
    client.println(name);
  } else {
    Serial.println("Failed to connect to server");
    connected = false;
  }
}
