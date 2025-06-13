#include <PubSubClient.h>
#include <WiFi.h>
// Configuración Wi-Fi
const char* ssid = "SoloConocedores";
const char* password = "D=2228n9";

// Configuración MQTT
const char* mqtt_server = "b6d522ef66224d36a55e598722e7338d.s1.eu.hivemq.cloud"; // Puedes usar tu propio broker
const int mqtt_port = 8883;
const char* mqtt_topic = "sensor/ambiente";

WiFiClient espClient;
PubSubClient client(espClient);

void conectarWiFi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println(WiFi.localIP());
}

void reconectarMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando al broker MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("conectado");
    } else {
      Serial.print("falló, rc=");
      Serial.print(client.state());
      Serial.println(" intentando de nuevo en 5 segundos");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  conectarWiFi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconectarMQTT();
  }
  client.loop();

  float temperatura = 25.3; // Simulado
  int luz = 512;            // Simulado

  // Crear mensaje JSON
  String mensaje = "{\"temperatura\":";
  mensaje += temperatura;
  mensaje += ",\"luz\":";
  mensaje += luz;
  mensaje += "}";

  // Publicar en MQTT
  client.publish(mqtt_topic, mensaje.c_str());

  Serial.println("Publicado: " + mensaje);
  delay(2000);
}


