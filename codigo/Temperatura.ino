#include <WiFiS3.h>
#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT22
#define LEDPIN 8

const char* ssid = "Mega_2.4G_96A0";
const char* password = "6GFHzQZt";

const char* webhookHost = "hook.us2.make.com";
const char* webhookPath = "/owu4jrrr8dk1ll2y8l9b61nqe21n82lo";

DHT dht(DHTPIN, DHTTYPE);

WiFiSSLClient client;

bool avisoEnviado = false;

void setup() {

  Serial.begin(9600);

  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, LOW);

  dht.begin();

  Serial.println("Iniciando...");

  // -----------------------------
  // CONECTAR AL WIFI
  // -----------------------------

  WiFi.begin(ssid, password);

  Serial.print("Conectando a WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi conectado!");

  // -----------------------------
  // ESPERAR IP
  // -----------------------------

  Serial.println("Esperando IP...");

  IPAddress ip = WiFi.localIP();

  while (ip == IPAddress(0, 0, 0, 0)) {

    delay(1000);

    ip = WiFi.localIP();

    Serial.println("Esperando DHCP...");
  }

  Serial.print("IP del Arduino: ");
  Serial.println(ip);

  Serial.println("Sistema listo.");
}


void loop() {

  // Leer DHT22

  float temperatura = dht.readTemperature();
  float humedad = dht.readHumidity();


  if (isnan(temperatura) || isnan(humedad)) {

    Serial.println("ERROR: No se pudo leer el DHT22");

    delay(2000);

    return;
  }


  // -----------------------------
  // MOSTRAR DATOS
  // -----------------------------

  Serial.println();
  Serial.println("============================");

  Serial.print("Temperatura: ");
  Serial.print(temperatura, 1);
  Serial.println(" °C");

  Serial.print("Humedad: ");
  Serial.print(humedad, 1);
  Serial.println(" %");


  // -----------------------------
  // SI SUPERA 26 °C
  // -----------------------------

  if (temperatura > 26.0) {

    digitalWrite(LEDPIN, HIGH);

    Serial.println("🔥 TEMPERATURA ALTA");
    Serial.println("LED D8 ENCENDIDO");


    // Solo mandar una vez

    if (!avisoEnviado) {

      Serial.println("Enviando alerta a Make...");

      enviarMake(temperatura, humedad);

      avisoEnviado = true;
    }

  }


  // -----------------------------
  // SI VUELVE A 26 °C O MENOS
  // -----------------------------

  else {

    digitalWrite(LEDPIN, LOW);

    avisoEnviado = false;

    Serial.println("❄️ Temperatura normal");
    Serial.println("LED D8 APAGADO");
  }


  // Leer cada 2 segundos

  delay(2000);
}


// =================================================
// ENVIAR DATOS A MAKE
// =================================================

void enviarMake(float temperatura, float humedad) {

  Serial.println("Conectando con Make...");


  if (!client.connect(webhookHost, 443)) {

    Serial.println("ERROR: No se pudo conectar con Make.");

    return;
  }


  Serial.println("Conectado a Make!");


  // -----------------------------
  // CREAR JSON
  // -----------------------------

  String json = "{";

  json += "\"temperatura\":";
  json += String(temperatura, 1);

  json += ",";

  json += "\"humedad\":";
  json += String(humedad, 1);

  json += "}";


  Serial.println("Enviando:");

  Serial.println(json);


  // -----------------------------
  // PETICION HTTP POST
  // -----------------------------

  client.println("POST " + String(webhookPath) + " HTTP/1.1");

  client.println("Host: " + String(webhookHost));

  client.println("Content-Type: application/json");

  client.println("Content-Length: " + String(json.length()));

  client.println("Connection: close");

  client.println();

  client.println(json);


  Serial.println("Peticion enviada.");


  // -----------------------------
  // LEER RESPUESTA
  // -----------------------------

  unsigned long inicio = millis();

  while (!client.available() && millis() - inicio < 5000) {

    delay(100);
  }


  if (client.available()) {

    String respuesta = client.readStringUntil('\n');

    Serial.print("Respuesta de Make: ");

    Serial.println(respuesta);

  }

  else {

    Serial.println("Make no envio respuesta.");
  }


  client.stop();

  Serial.println("Conexion cerrada.");
}