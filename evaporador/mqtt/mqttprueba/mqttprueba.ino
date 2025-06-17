#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include "mqtt_handler.h"  // Make sure this matches your header filename

// Configuración de la red WiFi
const char* ssid = "Informatica2024";
const char* password = "iinf2024";

// Credenciales del portal cautivo
const char* portal_username = "Practica";
const char* portal_password = "practica";

// Variables para el manejo de conexión
bool wifi_connected = false;
bool portal_authenticated = false;
String portal_url = "http://login.itszo.net/login";

// Pin del LED integrado (GPIO 2 en la mayoría de ESP32)
const int LED_PIN = 2;
bool led_state = false;
unsigned long last_blink = 0;

// Variables para MQTT
PubSubClient mqtt_client(secureClient);
bool mqtt_connected = false;
unsigned long last_mqtt_reconnect = 0;
unsigned long last_status_send = 0;
const unsigned long MQTT_RECONNECT_INTERVAL = 30000;  // 30 segundos
const unsigned long STATUS_SEND_INTERVAL = 60000;     // 1 minuto

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Configurar LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);  // LED apagado al inicio
  
  Serial.println("=== ESP32 Portal Cautivo ITSZO.NET + MQTT ===");
  
  // Conectar a WiFi
  connectToWiFi();
  
  if (wifi_connected) {
    // Detectar y autenticar en el portal cautivo
    handleCaptivePortal();
    
    if (portal_authenticated) {
      Serial.println("¡Conexión exitosa a internet!");
      Serial.print("Dirección MAC del ESP32: ");
      Serial.println(WiFi.macAddress());
      digitalWrite(LED_PIN, HIGH);  // LED encendido fijo cuando está conectado
      testInternetConnection();
      
      // Inicializar MQTT después de tener internet
      initializeMQTT();
    } else {
      // Si no se pudo autenticar, mantener parpadeo lento
      blinkLED(1000);
    }
  } else {
    // Si no hay WiFi, parpadeo rápido
    blinkLED(200);
  }
}

void loop() {
  // Manejar parpadeo del LED según el estado
  if (!portal_authenticated) {
    if (!wifi_connected) {
      blinkLED(200);  // Parpadeo rápido - sin WiFi
    } else {
      blinkLED(500);  // Parpadeo medio - conectando al portal
    }
  }
  
  // Verificar conexión WiFi periódicamente
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Conexión WiFi perdida. Reconectando...");
    wifi_connected = false;
    portal_authenticated = false;
    mqtt_connected = false;
    digitalWrite(LED_PIN, LOW);  // Apagar LED
    connectToWiFi();
    
    if (wifi_connected) {
      handleCaptivePortal();
      
      if (portal_authenticated) {
        Serial.println("¡Reconexión exitosa a internet!");
        Serial.print("Dirección MAC del ESP32: ");
        Serial.println(WiFi.macAddress());
        digitalWrite(LED_PIN, HIGH);  // LED encendido fijo cuando está conectado
        
        // Reinicializar MQTT después de reconectar
        initializeMQTT();
      }
    }
  }
  
  // Si hay conexión a internet, manejar MQTT y pruebas
  if (portal_authenticated) {
    // Manejar MQTT
    handleMQTT();
    
    // Hacer una prueba de internet cada 30 segundos
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 30000) {
      testInternetConnection();
      lastCheck = millis();
    }
    
    // Enviar estado por MQTT cada minuto
    if (millis() - last_status_send > STATUS_SEND_INTERVAL) {
      sendMQTTStatus();
      last_status_send = millis();
    }
  }
  
  delay(50);  // Delay muy pequeño para mejor respuesta del LED
}

void connectToWiFi() {
  Serial.print("Conectando a WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifi_connected = true;
    Serial.println();
    Serial.println("WiFi conectado!");
    Serial.print("Dirección IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("Error: No se pudo conectar a WiFi");
    wifi_connected = false;
  }
}

void handleCaptivePortal() {
  Serial.println("Verificando portal cautivo...");
  
  HTTPClient http;
  http.begin("http://www.google.com");
  http.setTimeout(5000);
  
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    
    if (payload.indexOf("login.itszo.net") != -1 || payload.indexOf("captive") != -1) {
      Serial.println("Portal cautivo detectado. Intentando autenticación...");
      authenticatePortal();
    } else {
      Serial.println("No se detectó portal cautivo. Conexión directa a internet.");
      portal_authenticated = true;
    }
  } else if (httpCode == 302 || httpCode == 301) {
    String location = http.header("Location");
    Serial.println("Redirección detectada a: " + location);
    
    if (location.indexOf("login.itszo.net") != -1) {
      Serial.println("Portal cautivo detectado por redirección. Intentando autenticación...");
      authenticatePortal();
    } else {
      portal_authenticated = true;
    }
  } else {
    Serial.println("Error verificando conexión: " + String(httpCode));
    // En caso de error, intentar autenticación por si acaso
    authenticatePortal();
  }
  
  http.end();
}

void authenticatePortal() {
  HTTPClient http;
  http.begin(portal_url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.addHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
  
  String postData = "username=" + String(portal_username) + "&password=" + String(portal_password);
  
  Serial.println("Enviando credenciales al portal...");
  
  int httpCode = http.POST(postData);
  
  if (httpCode > 0) {
    String response = http.getString();
    Serial.println("Respuesta del portal (código " + String(httpCode) + "):");
    
    if (httpCode == 200 && (response.indexOf("success") != -1 || response.indexOf("authenticated") != -1)) {
      portal_authenticated = true;
      Serial.println("✓ Autenticación exitosa en el portal cautivo!");
    } else if (httpCode == 302 || httpCode == 301) {
      portal_authenticated = true;
      Serial.println("✓ Autenticación exitosa (redirección detectada)!");
    } else {
      Serial.println("✗ Error en la autenticación del portal");
      Serial.println("Respuesta: " + response.substring(0, 200));
    }
  } else {
    Serial.println("✗ Error en la solicitud HTTP: " + String(httpCode));
  }
  
  http.end();
  
  // Verificar conectividad después de intentar autenticar
  delay(2000);
  testInternetConnection();
}

void testInternetConnection() {
  Serial.println("Probando conectividad a internet...");
  
  HTTPClient http;
  http.begin("http://www.google.com");
  http.setTimeout(5000);
  
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    
    if (payload.indexOf("<title>Google</title>") != -1 || payload.length() > 1000) {
      portal_authenticated = true;
      Serial.println("✓ Conexión a internet confirmada!");
    } else {
      portal_authenticated = false;
      Serial.println("✗ Respuesta inesperada. Posible portal cautivo aún activo.");
    }
  } else {
    Serial.println("✗ No hay conectividad a internet. Código: " + String(httpCode));
    portal_authenticated = false;
  }
  
  http.end();
}

void blinkLED(int interval) {
  if (millis() - last_blink > interval) {
    led_state = !led_state;
    digitalWrite(LED_PIN, led_state);
    last_blink = millis();
  }
}