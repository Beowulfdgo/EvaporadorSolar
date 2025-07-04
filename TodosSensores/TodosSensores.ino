#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <DHT.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
#include <MPU6050.h>
#include <HX711.h>
#include <SPI.h>
#include <SD.h>
#include <math.h>
#include <ArduinoJson.h>
#include <time.h>

// ================== CONFIGURACIÓN WIFI Y MQTT ==================
const char* ssid = "Informatica2024";
const char* password = "iinf2024";
const char* mqtt_server = "b6d522ef66224d36a55e598722e7338d.s1.eu.hivemq.cloud";
const char* mqtt_username = "evaporador";
const char* mqtt_password = "Evaporador2025";
const int mqtt_port = 8883;
#define LED_WIFI 2

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

// ================== PINES Y OBJETOS DE SENSORES ==================
#define DHTPIN 4
#define DHTTYPE DHT11
#define LDR_PIN 34
#define HX711_DT 32
#define HX711_SCK 33

DHT dht(DHTPIN, DHTTYPE);
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);  // RX=16, TX=17

MPU6050 mpu1(0x68);
MPU6050 mpu2(0x69);
HX711 scale;

unsigned long previousMillis = 0;
const unsigned long interval = 5000;  // 5 segundos

// ================== MQTT TOPICS ==================
const char* topic_temp = "evaporador/dht11/temperatura";
const char* topic_hum = "evaporador/dht11/humedad";
const char* topic_luz = "evaporador/luminosidad";
const char* topic_bascula = "evaporador/bascula/mg";
const char* topic_giro_json = "evaporador/giroscopio/json";
const char* topic_gps_lat = "evaporador/gps/latitud";
const char* topic_gps_lon = "evaporador/gps/longitud";
const char* topic_status = "evaporador/status";

// ================== FUNCIONES ==================
void setup() {
  Serial.begin(115200);
  pinMode(LED_WIFI, OUTPUT);
  dht.begin();
  analogReadResolution(10);
  gpsSerial.begin(9600, SERIAL_8N1, 16, 17);

  // MPU6050 #1
  Wire.begin(21, 22);
  mpu1.initialize();

  // MPU6050 #2
  Wire1.begin(25, 26);
  mpu2.initialize();

  // HX711
  scale.begin(HX711_DT, HX711_SCK);
  scale.set_scale(); // Asegúrate de calibrar tu báscula
  scale.tare();

  // WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  int intentosWifi = 0;
  while (WiFi.status() != WL_CONNECTED && intentosWifi < 15) {
    digitalWrite(LED_WIFI, !digitalRead(LED_WIFI));
    delay(1000);
    Serial.print(".");
    intentosWifi++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(LED_WIFI, HIGH);
    Serial.println("\nWiFi conectado\nIP address: ");
    Serial.println(WiFi.localIP());
    espClient.setCACert(root_ca);
    client.setServer(mqtt_server, mqtt_port);
    delay(1000);
  } else {
    digitalWrite(LED_WIFI, LOW);
    Serial.println("\nNo se pudo conectar a WiFi después de 15 intentos.");
  }
}

void reconnect() {
  int intentosMqtt = 0;
  while (!client.connected() && intentosMqtt < 5) {
    Serial.print("Intentando conectar MQTT...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Conectado a MQTT");
      return;
    } else {
      Serial.print("Fallo, rc=");
      Serial.print(client.state());
      Serial.println(" intentando en 2 segundos");
      delay(2000);
      intentosMqtt++;
    }
  }
  if (!client.connected()) {
    Serial.println("No se pudo conectar a MQTT después de 5 intentos.");
  }
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // Leer datos de GPS
  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    float temp = dht.readTemperature();
    float hum = dht.readHumidity();
    int luz = analogRead(LDR_PIN);
    float peso = scale.get_units();

    // MPU6050 #1
    int16_t ax1, ay1, az1;
    mpu1.getAcceleration(&ax1, &ay1, &az1);
    float ax1g = ax1 / 16384.0, ay1g = ay1 / 16384.0, az1g = az1 / 16384.0;
    float angleX1 = atan2(ay1g, az1g) * 180 / PI;
    float angleY1 = atan2(-ax1g, sqrt(ay1g * ay1g + az1g * az1g)) * 180 / PI;

    // MPU6050 #2
    int16_t ax2, ay2, az2;
    mpu2.getAcceleration(&ax2, &ay2, &az2);
    float ax2g = ax2 / 16384.0, ay2g = ay2 / 16384.0, az2g = az2 / 16384.0;
    float angleX2 = atan2(ay2g, az2g) * 180 / PI;
    float angleY2 = atan2(-ax2g, sqrt(ay2g * ay2g + az2g * az2g)) * 180 / PI;

    // GPS lat/lon
    float lat = gps.location.isValid() ? gps.location.lat() : 0.0;
    float lon = gps.location.isValid() ? gps.location.lng() : 0.0;

    // Publicar datos por MQTT
    client.publish(topic_temp, String(temp,1).c_str(), true);
    client.publish(topic_hum, String(hum,1).c_str(), true);
    client.publish(topic_luz, String(luz).c_str(), true);
    // Enviar peso en mg (si tu báscula está calibrada en gramos)
    client.publish(topic_bascula, String(peso * 1000.0, 0).c_str(), true); 

    // Giroscopio JSON (ambos MPU6050)
    StaticJsonDocument<256> docGiro;
    docGiro["MPU1_X"] = angleX1;
    docGiro["MPU1_Y"] = angleY1;
    docGiro["MPU2_X"] = angleX2;
    docGiro["MPU2_Y"] = angleY2;
    docGiro["hora"] = millis()/1000;
    String giroPayload;
    serializeJson(docGiro, giroPayload);
    client.publish(topic_giro_json, giroPayload.c_str(), true);

    // GPS lat/lon
    client.publish(topic_gps_lat, String(lat, 6).c_str(), true);
    client.publish(topic_gps_lon, String(lon, 6).c_str(), true);

    // Heartbeat/status
    client.publish(topic_status, "online", true);

    // Debug por Serial
    Serial.println("----- REGISTRO (MQTT) -----");
    Serial.printf("Temp: %.1f°C, Hum: %.1f%%, Luz: %d, Peso: %.2f g\n", temp, hum, luz, peso);
    Serial.printf("GPS: Lat %.6f, Lon %.6f\n", lat, lon);
    Serial.printf("MPU1: X=%.2f°, Y=%.2f°\n", angleX1, angleY1);
    Serial.printf("MPU2: X=%.2f°, Y=%.2f°\n", angleX2, angleY2);
    Serial.println("---------------------------\n");
  }
}