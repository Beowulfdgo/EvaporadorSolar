#include <Wire.h>
#include <DHT.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
#include <MPU6050.h>
#include <HX711.h>
#include <SPI.h>
#include <SD.h>
#include <math.h>

// ---- Pines ----
#define DHTPIN 4
#define DHTTYPE DHT11
#define LDR_PIN 34
#define HX711_DT 32
#define HX711_SCK 33
#define SD_CS 5

#define SD_MISO 2
#define SD_MOSI 23
#define SD_SCK  18

// ---- Objetos sensores ----
DHT dht(DHTPIN, DHTTYPE);
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);  // RX=16, TX=17

MPU6050 mpu1(0x68);
MPU6050 mpu2(0x69);
HX711 scale;
File dataFile;

unsigned long previousMillis = 0;
const unsigned long interval = 5000;  // 5 segundos

void setup() {
  Serial.begin(115200);
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
  scale.set_scale();
  scale.tare();

  // SD
  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS)) {
    Serial.println("⚠️ Error al iniciar SD.");
  } else {
    Serial.println("✅ SD lista.");
  }

  // Crear archivo si no existe
  if (!SD.exists("/datos.csv")) {
    dataFile = SD.open("/datos.csv", FILE_WRITE);
    if (dataFile) {
      dataFile.println("Fecha,Hora,Temperatura,Humedad,Luz,Latitud,Longitud,Peso,AngX1,AngY1,AngX2,AngY2");
      dataFile.close();
    }
  }
}

void loop() {
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

    // Fecha y hora GPS
    String fecha = "00/00/0000";
    String hora  = "00:00:00";

    if (gps.date.isValid()) {
      char f[11];
      sprintf(f, "%02d/%02d/%04d", gps.date.day(), gps.date.month(), gps.date.year());
      fecha = String(f);
    }

    if (gps.time.isValid()) {
      int h = (gps.time.hour() - 6 + 24) % 24; // UTC-6
      char t[9];
      sprintf(t, "%02d:%02d:%02d", h, gps.time.minute(), gps.time.second());
      hora = String(t);
    }

    // Serial output
    Serial.println("----- REGISTRO -----");
    Serial.print("Fecha: "); Serial.print(fecha);
    Serial.print(" | Hora: "); Serial.println(hora);
    Serial.printf("Temp: %.1f°C, Hum: %.1f%%, Luz: %d, Peso: %.2f g\n", temp, hum, luz, peso);
    if (gps.location.isValid()) {
      Serial.printf("GPS: Lat %.6f, Lon %.6f\n", gps.location.lat(), gps.location.lng());
    } else {
      Serial.println("Esperando señal GPS...");
    }
    Serial.printf("MPU1: X=%.2f°, Y=%.2f°\n", angleX1, angleY1);
    Serial.printf("MPU2: X=%.2f°, Y=%.2f°\n", angleX2, angleY2);
    Serial.println("--------------------\n");

    // Guardar en SD
    dataFile = SD.open("/datos.csv", FILE_APPEND);
    if (dataFile) {
      dataFile.print(fecha + "," + hora + ",");
      dataFile.printf("%.1f,%.1f,%d,", temp, hum, luz);
      if (gps.location.isValid()) {
        dataFile.printf("%.6f,%.6f,", gps.location.lat(), gps.location.lng());
      } else {
        dataFile.print("0,0,");
      }
      dataFile.printf("%.2f,%.2f,%.2f,%.2f,%.2f\n", peso, angleX1, angleY1, angleX2, angleY2);
      dataFile.close();
    } else {
      Serial.println("⚠️ Error al escribir en SD");
    }
  }
}
