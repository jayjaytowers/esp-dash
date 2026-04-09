/*
  -----------------------------
  ESPDASH Lite - Interactive Example
  -----------------------------

  Skill Level: Intermediate

  In this example we will be creating a interactive dashboard which consists 
  of a button and a slider.

  Github: https://github.com/ayushsharma82/ESP-DASH
  WiKi: https://docs.espdash.pro

  Works with ESP32, RP2040+W and RP2350+W based devices / projects.
  -------------------------------

  Upgrade to ESP-DASH Pro: https://espdash.pro
*/

#include <Arduino.h>
#if defined(ESP8266)
  /* ESP8266 Dependencies */
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
  #include <ESPAsyncWebServer.h>
#elif defined(ESP32)
  /* ESP32 Dependencies */
  #include <WiFi.h>
  #include <AsyncTCP.h>
  #include <ESPAsyncWebServer.h>
#elif defined(TARGET_RP2040) || defined(PICO_RP2040) || defined(TARGET_RP2350) || defined(PICO_RP2350)
  /* RP2040 or RP2350 Dependencies */
  #include <WiFi.h>
  #include <RPAsyncTCP.h>
  #include <ESPAsyncWebServer.h>
#endif
#include <ESPDash.h>
#include <DHTesp.h>
#include <Ticker.h>

// Credenciales WiFi
const char* ssid = "Wokwi-GUEST"; // SSID
const char* password = ""; // Password
const int wifi_channel = 6; // para wokwi

// Pines
const int dhtPin = 13;  // DHT-22
const int led1Pin = 4;  // LED verde
const int led2Pin = 15; // LED amarillo

// Prototipos
void updateSensors ();

// Instancio objetos
DHTesp dht;
Ticker periodicTicker (updateSensors, 5000); // Actualizar sensores cada 5 segundos
AsyncWebServer server (80);

// Attachar ESP-DASH al AsyncWebServer */
ESPDash dashboard (server); 

// Tarjetas para sensores (1 decimal de precision)
dash::TemperatureCard <float, 1> tempCard (dashboard, "Temperatura", "°C");
dash::HumidityCard <float, 1> humCard (dashboard, "Humedad", "%");

// Tarjetas tipo pushbutton para los LEDs
dash::ToggleButtonCard led1Card(dashboard, "LED verde");
dash::ToggleButtonCard led2Card(dashboard, "LED amarillo");

// Fn. para actualizar valores
void updateSensors() {
  TempAndHumidity data = dht.getTempAndHumidity ();
  tempCard.setValue(data.temperature);
  humCard.setValue (data.humidity);
  dashboard.sendUpdates ();
}

void setup() {
  Serial.begin(115200);

  // Inicializar DHT
  dht.setup (dhtPin, DHTesp::DHT22);

  // Configurar LEDs como salidas
  pinMode (led1Pin, OUTPUT);
  pinMode (led2Pin, OUTPUT);

  // Conectar WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password, wifi_channel);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
      Serial.printf("WiFi Failed!\n");
      return;
  }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Inicializo tarjetas del dash
  tempCard.setUnit("°C");
  humCard.setValue(0.0f);
  humCard.setUnit("%");

  // Callbacks para los leds
  led1Card.onChange ([] (bool state) {
    /* Print our new button value received from dashboard */
    Serial.println(String("Led verde: ")+(state?"true":"false"));
    /* actualizar el estado del led */
    digitalWrite (led1Pin, state);
    // actualizar valores en el dash
    led1Card.setValue(state);
    dashboard.sendUpdates();
  });

  led2Card.onChange([](bool state){
    /* Print our new button value received from dashboard */
    Serial.println(String("Led amarillo: ")+(state?"true":"false"));
    /* actualizar el led*/
    digitalWrite (led2Pin, state);
    // actualizar valores en el dash
    led2Card.setValue(state);
    dashboard.sendUpdates();
  });

  // Arrancar web server
  server.begin();

  // Arrancar ticker
  periodicTicker.start ();
}

void loop() {
  // Actualizacion del ticker
  periodicTicker.update ();
  delay (100);
}