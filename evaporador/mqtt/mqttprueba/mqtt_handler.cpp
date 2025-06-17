#include "mqtt_handler.h"
#include <WiFi.h>

// Configuración MQTT de HiveMQ Cloud
const char* mqtt_server = "b6d522ef66224d36a55e598722e7338d.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "hivemq.webclient.1749844320500";
const char* mqtt_password = "bcDm5j>2lRvUKA,%6.3Q";

// Certificado raíz corregido
const char* root_ca = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n" \
"MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
"MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
"b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG\n" \
"9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI\n" \
"2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx\n" \
"1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ\n" \
"q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz\n" \
"tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ\n" \
"vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP\n" \
"BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBBjAdBgNVHQ4EFgQUTiJUIBiV\n" \
"5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY\n" \
"1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4\n" \
"NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG\n" \
"Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91\n" \
"8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe\n" \
"pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl\n" \
"MrY=\n" \
"-----END CERTIFICATE-----\n";

// Instancia del cliente seguro
WiFiClientSecure secureClient;

// Variables externas para MQTT
extern PubSubClient mqtt_client;
extern bool mqtt_connected;
extern unsigned long last_mqtt_reconnect;
extern const unsigned long MQTT_RECONNECT_INTERVAL;

void mqtt_setup(PubSubClient& client) {
  // Configurar el certificado CA
  secureClient.setCACert(root_ca);
  
  // Configurar servidor MQTT
  client.setServer(mqtt_server, mqtt_port);
  
  // Configurar callback para mensajes recibidos
  client.setCallback([](char* topic, byte* payload, unsigned int length) {
    Serial.print("Mensaje recibido en topic: ");
    Serial.println(topic);
    
    String message = "";
    for (int i = 0; i < length; i++) {
      message += (char)payload[i];
    }
    Serial.print("Mensaje: ");
    Serial.println(message);
  });
  
  Serial.println("MQTT configurado correctamente");
}

bool mqtt_reconnect_safe(PubSubClient& client) {
  if (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    
    // Crear un ID único para el cliente
    String clientId = "ESP32-" + WiFi.macAddress();
    clientId.replace(":", "");
    
    // Intentar conectar
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("conectado");
      
      // Suscribirse a un topic
      String topic = "itszo/esp32/" + clientId + "/commands";
      client.subscribe(topic.c_str());
      Serial.println("Suscrito a: " + topic);
      
      return true;
    } else {
      Serial.print("falló, rc=");
      Serial.print(client.state());
      Serial.println(" reintentando en 30 segundos");
      return false;
    }
  }
  return true;
}

void mqtt_loop(PubSubClient& client) {
  client.loop();
}

void initializeMQTT() {
  Serial.println("Inicializando MQTT...");
  mqtt_setup(mqtt_client);
}

void handleMQTT() {
  // Verificar conexión MQTT cada 30 segundos
  if (!mqtt_client.connected()) {
    if (millis() - last_mqtt_reconnect > MQTT_RECONNECT_INTERVAL) {
      mqtt_connected = mqtt_reconnect_safe(mqtt_client);
      last_mqtt_reconnect = millis();
    }
  } else {
    mqtt_connected = true;
    mqtt_loop(mqtt_client);
  }
}

void sendMQTTStatus() {
  if (mqtt_client.connected()) {
    String clientId = "ESP32-" + WiFi.macAddress();
    clientId.replace(":", "");
    
    String topic = "itszo/esp32/" + clientId + "/status";
    String message = "{\"status\":\"online\",\"ip\":\"" + WiFi.localIP().toString() + 
                    "\",\"mac\":\"" + WiFi.macAddress() + 
                    "\",\"uptime\":" + String(millis()) + "}";
    
    if (mqtt_client.publish(topic.c_str(), message.c_str())) {
      Serial.println("Estado enviado por MQTT: " + message);
    } else {
      Serial.println("Error enviando estado por MQTT");
    }
  }
}