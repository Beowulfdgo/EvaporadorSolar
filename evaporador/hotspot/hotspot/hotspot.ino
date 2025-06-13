#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>

// Configuración de la red WiFi
const char* ssid = "Informatica2024";
const char* password = "iinf2024";  // Sin contraseña

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

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Configurar LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);  // LED apagado al inicio
  
  Serial.println("=== ESP32 Portal Cautivo ITSZO.NET ===");
  
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
  
  // Verificar conexión periódicamente
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Conexión WiFi perdida. Reconectando...");
    wifi_connected = false;
    portal_authenticated = false;
    digitalWrite(LED_PIN, LOW);  // Apagar LED
    connectToWiFi();
    
    if (wifi_connected) {
      handleCaptivePortal();
      
      if (portal_authenticated) {
        Serial.println("¡Reconexión exitosa a internet!");
        Serial.print("Dirección MAC del ESP32: ");
        Serial.println(WiFi.macAddress());
        digitalWrite(LED_PIN, HIGH);  // LED encendido fijo cuando está conectado
      }
    }
  }
  
  // Si hay conexión, hacer una prueba de internet cada 30 segundos
  if (portal_authenticated) {
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 30000) {
      testInternetConnection();
      lastCheck = millis();
    }
  }
  
  delay(50);  // Delay muy pequeño para mejor respuesta del LED
}

void connectToWiFi() {
  Serial.print("Conectando a ");
  Serial.print(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    blinkLED(300);  // Parpadeo durante la conexión
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifi_connected = true;
    Serial.println();
    Serial.println("WiFi conectado!");
    Serial.print("Dirección IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("Dirección MAC: ");
    Serial.println(WiFi.macAddress());
    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());
  } else {
    Serial.println();
    Serial.println("Error: No se pudo conectar al WiFi");
    wifi_connected = false;
  }
}

void handleCaptivePortal() {
  Serial.println("Detectando portal cautivo...");
  
  HTTPClient http;
  
  // Probar acceso directo a Google para detectar portal
  http.begin("http://www.google.com");
  http.setTimeout(8000);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    String response = http.getString();
    
    Serial.print("Código HTTP: ");
    Serial.println(httpCode);
    Serial.print("Respuesta Google (primeros 100 chars): ");
    Serial.println(response.substring(0, 100));
    
    // Verificar si hay acceso directo a internet
    if (httpCode == 200 && (response.indexOf("google") != -1 || response.indexOf("Google") != -1)) {
      Serial.println("No hay portal cautivo. Acceso directo a internet.");
      portal_authenticated = true;
      digitalWrite(LED_PIN, HIGH);  // LED encendido fijo
    } else if (httpCode == 302 || httpCode == 301 || 
               response.indexOf("login") != -1 ||
               response.indexOf("portal") != -1 ||
               response.indexOf("itszo") != -1) {
      Serial.println("Portal cautivo detectado!");
      authenticateItszoPortal();
    } else {
      Serial.println("Estado incierto. Intentando autenticación...");
      authenticateItszoPortal();
    }
  } else {
    Serial.print("Error en la solicitud HTTP: ");
    Serial.println(httpCode);
    // Si hay error, asumir que hay portal y intentar autenticación
    authenticateItszoPortal();
  }
  
  http.end();
}

void authenticateItszoPortal() {
  Serial.println("=== AUTENTICACIÓN PORTAL ITSZO.NET ===");
  
  // Datos de autenticación optimizados para el portal ITSZO
  String authData[] = {
    // Formato principal basado en el formulario HTML proporcionado
    "username=" + String(portal_username) + "&password=" + String(portal_password),
    
    // Con submit vacío (común en formularios con JavaScript)
    "username=" + String(portal_username) + "&password=" + String(portal_password) + "&submit=",
    
    // Con submit como espacio
    "username=" + String(portal_username) + "&password=" + String(portal_password) + "&submit=%20",
    
    // Con parámetros adicionales comunes
    "username=" + String(portal_username) + "&password=" + String(portal_password) + "&dst=",
    
    // Con referrer
    "username=" + String(portal_username) + "&password=" + String(portal_password) + "&submit=&dst=http%3A//www.google.com/"
  };
  
  for (int i = 0; i < 5; i++) {
    Serial.print("Intento de autenticación ");
    Serial.print(i + 1);
    Serial.print("/5: ");
    
    HTTPClient http;
    http.begin(portal_url);
    
    // Headers específicos para el portal ITSZO
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("User-Agent", "Mozilla/5.0 (ESP32) AppleWebKit/537.36 Chrome/91.0.4472.124");
    http.addHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    http.addHeader("Accept-Language", "es-ES,es;q=0.9,en;q=0.8");
    http.addHeader("Accept-Encoding", "identity");  // Sin compresión para ESP32
    http.addHeader("Connection", "keep-alive");
    http.addHeader("Cache-Control", "no-cache");
    http.addHeader("Referer", portal_url);
    http.setTimeout(15000);
    
    Serial.println(authData[i]);
    
    int httpCode = http.POST(authData[i]);
    
    Serial.print("Código respuesta: ");
    Serial.println(httpCode);
    
    if (httpCode > 0) {
      String response = http.getString();
      
      // Debug de la respuesta
      Serial.print("Respuesta (200 chars): ");
      Serial.println(response.substring(0, 200));
      
      // Verificar indicadores de éxito
      bool possibleSuccess = false;
      
      if (httpCode == 302 || httpCode == 301) {
        Serial.println("→ Redirección detectada");
        possibleSuccess = true;
      } else if (httpCode == 200) {
        // Verificar contenido de la respuesta
        String lowerResponse = response;
        lowerResponse.toLowerCase();
        
        if (lowerResponse.indexOf("success") != -1 || 
            lowerResponse.indexOf("exitoso") != -1 ||
            lowerResponse.indexOf("bienvenido") != -1 ||
            lowerResponse.indexOf("welcome") != -1) {
          Serial.println("→ Mensaje de éxito detectado");
          possibleSuccess = true;
        } else if (lowerResponse.indexOf("error") != -1 || 
                   lowerResponse.indexOf("invalid") != -1 ||
                   lowerResponse.indexOf("incorrect") != -1 ||
                   lowerResponse.indexOf("usuario") != -1 && lowerResponse.indexOf("incorrecto") != -1) {
          Serial.println("→ Mensaje de error detectado");
        } else {
          // Si no hay mensaje claro, podría ser éxito silencioso
          possibleSuccess = true;
        }
      }
      
      http.end();
      
      // Esperar para que el portal procese la autenticación
      delay(3000);
      
      // Probar conectividad
      if (testBasicConnectivity()) {
        portal_authenticated = true;
        digitalWrite(LED_PIN, HIGH);
        Serial.println("🎉 ¡AUTENTICACIÓN EXITOSA!");
        return;
      } else if (possibleSuccess) {
        Serial.println("⚠ Posible éxito - esperando más tiempo...");
        delay(5000);
        if (testBasicConnectivity()) {
          portal_authenticated = true;
          digitalWrite(LED_PIN, HIGH);
          Serial.println("🎉 ¡AUTENTICACIÓN EXITOSA (después de esperar)!");
          return;
        }
      }
      
    } else {
      Serial.print("Error HTTP: ");
      Serial.println(httpCode);
      http.end();
    }
    
    delay(2000);  // Pausa entre intentos
  }
  
  Serial.println("❌ Todos los intentos de autenticación fallaron");
  tryAlternativeAuthentication();
}

void tryAlternativeAuthentication() {
  Serial.println("Probando métodos alternativos...");
  
  // URLs alternativas del portal ITSZO
  String alternativeUrls[] = {
    "http://login.itszo.net/auth",
    "http://login.itszo.net/",
    "http://login.itszo.net/index.php",
    "http://" + WiFi.gatewayIP().toString() + "/login"
  };
  
  for (int i = 0; i < 4; i++) {
    HTTPClient http;
    http.begin(alternativeUrls[i]);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.addHeader("User-Agent", "Mozilla/5.0 (ESP32)");
    
    String postData = "username=" + String(portal_username) + 
                     "&password=" + String(portal_password);
    
    Serial.print("Probando URL: ");
    Serial.println(alternativeUrls[i]);
    
    int httpCode = http.POST(postData);
    
    if (httpCode > 0) {
      Serial.print("Código: ");
      Serial.println(httpCode);
      delay(3000);
      if (testBasicConnectivity()) {
        portal_authenticated = true;
        digitalWrite(LED_PIN, HIGH);
        Serial.println("¡Autenticación alternativa exitosa!");
        http.end();
        return;
      }
    }
    
    http.end();
    delay(2000);
  }
  
  Serial.println("Todos los métodos alternativos fallaron");
  Serial.println("Verificar credenciales: username=" + String(portal_username) + 
                 ", password=" + String(portal_password));
}

bool testBasicConnectivity() {
  Serial.println("Verificando conectividad...");
  
  HTTPClient http;
  http.begin("http://www.google.com");
  http.setTimeout(8000);
  
  int httpCode = http.GET();
  bool success = false;
  
  if (httpCode == 200) {
    String response = http.getString();
    
    if (response.indexOf("google") != -1 || response.indexOf("Google") != -1) {
      success = true;
      Serial.println("✓ Conectividad a internet confirmada");
    } else {
      Serial.println("✗ Respuesta no es de Google");
      Serial.print("Contenido (100 chars): ");
      Serial.println(response.substring(0, 100));
    }
  } else {
    Serial.print("✗ Error HTTP: ");
    Serial.println(httpCode);
  }
  
  http.end();
  return success;
}

void testInternetConnection() {
  HTTPClient http;
  http.begin("http://www.google.com");
  http.setTimeout(10000);
  
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String response = http.getString();
    if (response.indexOf("google") != -1 || response.indexOf("Google") != -1) {
      Serial.println("✓ Conexión a internet OK");
    } else {
      Serial.println("⚠ Respuesta inesperada de Google");
      portal_authenticated = false;
      digitalWrite(LED_PIN, LOW);
    }
  } else {
    Serial.print("✗ Error de conexión: ");
    Serial.println(httpCode);
    portal_authenticated = false;
    digitalWrite(LED_PIN, LOW);
  }
  
  http.end();
}

// Función para manejar el parpadeo del LED
void blinkLED(int interval) {
  if (millis() - last_blink >= interval) {
    led_state = !led_state;
    digitalWrite(LED_PIN, led_state);
    last_blink = millis();
  }
}