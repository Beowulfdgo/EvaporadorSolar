#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <SPI.h>
#include <ArduinoJson.h> // Instala esta librería

// ==== WiFi settings
const char* ssid = "Informatica2024";
const char* password = "iinf2024";

// ==== MQTT Broker settings
const char* mqtt_server = "b6d522ef66224d36a55e598722e7338d.s1.eu.hivemq.cloud";
const char* mqtt_username = "evaporador";
const char* mqtt_password = "Evaporador2025";
const int mqtt_port = 8883;

// ==== LED SETTINGS
#define LED_WIFI 2  // Cambia el pin si tu LED está en otro

WiFiClientSecure espClient;
PubSubClient client(espClient);

static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

// MQTT Topics
const char* topic_dht11_temp = "evaporador/dht11/temperatura";
const char* topic_dht11_hum = "evaporador/dht11/humedad";
const char* topic_gyro_lat = "evaporador/giroscopio/latitud";
const char* topic_gyro_lon = "evaporador/giroscopio/longitud";
const char* topic_lux = "evaporador/luminosidad";
const char* topic_motorX = "evaporador/motorX/estatus";
const char* topic_motorY = "evaporador/motorY/estatus";
const char* topic_gps_lat = "evaporador/gps/latitud";
const char* topic_gps_lon = "evaporador/gps/longitud";
const char* topic_bascula = "evaporador/bascula/mg";
const char* topic_motorX_cmd = "evaporador/motorX/cmd";
const char* topic_motorY_cmd = "evaporador/motorY/cmd";

float last_gps_lat = 19.4326;
float last_gps_lon = -99.1332;

// Estado motores y grados
String motorX_status = "OFF";
String motorY_status = "OFF";
int motorX_grados = 0;
int motorY_grados = 0;

// ===================== MQTT CALLBACK ==========================
void callback(char* topic, byte* payload, unsigned int length) {
  String incoming = "";
  for (int i = 0; i < length; i++) incoming += (char)payload[i];

  // Control de motores y grados por MQTT
  if (String(topic) == topic_motorX_cmd) {
    if (incoming == "ON" || incoming == "OFF") motorX_status = incoming;
    if (incoming.startsWith("GRADOS:")) {
      motorX_grados = incoming.substring(7).toInt();
      Serial.printf("MotorX a %d grados\n", motorX_grados);
    }
  }
  if (String(topic) == topic_motorY_cmd) {
    if (incoming == "ON" || incoming == "OFF") motorY_status = incoming;
    if (incoming.startsWith("GRADOS:")) {
      motorY_grados = incoming.substring(7).toInt();
      Serial.printf("MotorY a %d grados\n", motorY_grados);
    }
  }
}

// ===================== WIFI Y MQTT ============================
void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conectar MQTT...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Conectado a MQTT");
      client.subscribe(topic_motorX_cmd);
      client.subscribe(topic_motorY_cmd);
    } else {
      Serial.print("Fallo, rc=");
      Serial.print(client.state());
      Serial.println(" intentando en 5 segundos");
      delay(5000);
    }
  }
}

// ===================== SETUP ============================
void setup() {
  Serial.begin(115200);
  pinMode(LED_WIFI, OUTPUT);

  Serial.print("\nConectando a ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  client.setCallback(callback);

  // LED parpadea mientras conecta
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_WIFI, !digitalRead(LED_WIFI));
    delay(300);
    Serial.print(".");
  }

  // LED fijo cuando conecta
  digitalWrite(LED_WIFI, HIGH);
  Serial.println("\nWiFi conectado\nIP address: ");
  Serial.println(WiFi.localIP());
  espClient.setCACert(root_ca);
  client.setServer(mqtt_server, mqtt_port);
  delay(1000);
}

// ===================== LOOP PRINCIPAL ============================
void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // ==== SIMULACION DE SENSORES ====
  float temp = random(200, 350) / 10.0;
  float hum = random(300, 700) / 10.0;
  float gyro_lat = random(-9000, 9000) / 100.0;
  float gyro_lon = random(-18000, 18000) / 100.0;
  int lux = random(0, 10000);
  int bascula = random(50000, 500000); // 50,000 a 500,000 mg
  last_gps_lat += random(-10, 10) / 10000.0;
  last_gps_lon += random(-10, 10) / 10000.0;

  // ==== JSON PARA MOSTRAR Y (SIMULAR) GUARDAR EN SD ====
  StaticJsonDocument<512> doc;
  doc["dht11"]["temp"] = temp;
  doc["dht11"]["hum"] = hum;
  doc["giroscopio"]["latitud"] = gyro_lat;
  doc["giroscopio"]["longitud"] = gyro_lon;
  doc["luminosidad"] = lux;
  doc["bascula_mg"] = bascula;
  doc["motorX"]["estatus"] = motorX_status;
  doc["motorX"]["grados"] = motorX_grados;
  doc["motorY"]["estatus"] = motorY_status;
  doc["motorY"]["grados"] = motorY_grados;
  doc["gps"]["latitud"] = last_gps_lat;
  doc["gps"]["longitud"] = last_gps_lon;

  String jsonOutput;
  serializeJson(doc, jsonOutput);

  // ==== PUBLICAR MQTT ====
  publishMessage(topic_dht11_temp, String(temp, 1), true);
  publishMessage(topic_dht11_hum, String(hum, 1), true);
  publishMessage(topic_gyro_lat, String(gyro_lat, 2), true);
  publishMessage(topic_gyro_lon, String(gyro_lon, 2), true);
  publishMessage(topic_lux, String(lux), true);
  publishMessage(topic_bascula, String(bascula), true);
  publishMessage(topic_motorX, String(motorX_status) + ",GRADOS:" + String(motorX_grados), true);
  publishMessage(topic_motorY, String(motorY_status) + ",GRADOS:" + String(motorY_grados), true);
  publishMessage(topic_gps_lat, String(last_gps_lat, 6), true);
  publishMessage(topic_gps_lon, String(last_gps_lon, 6), true);

  // ==== MOSTRAR Y (SIMULAR) GUARDAR EN SD ====
  Serial.println("\nJSON publicado:");
  Serial.println(jsonOutput);

  // ----------- Simular guardado en SD -----------
  // File file = SD.open("/valores.json", FILE_WRITE);
  // if(file) {
  //   file.println(jsonOutput);
  //   file.close();
  // }
  // ----------------------------------------------

  delay(5000);
}

// ===================== MQTT PUBLISH ============================
void publishMessage(const char* topic, String payload, boolean retained) {
  if (client.publish(topic, payload.c_str(), retained))
    Serial.println("Mensaje publicado [" + String(topic) + "]: " + payload);
}