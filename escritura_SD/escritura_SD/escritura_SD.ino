#include <SPI.h>
#include <SD.h>

#define SD_CS 5  // Pin CS que hayas conectado

void setup() {
  Serial.begin(115200);
  if (!SD.begin(SD_CS)) {
    Serial.println("¡Error al inicializar la tarjeta SD!");
    return;
  }
  Serial.println("Tarjeta SD inicializada correctamente.");

  File archivo = SD.open("/ejemplo.txt", FILE_WRITE);
  if (archivo) {
    archivo.println("Hola desde ESP32");
    archivo.close();
    Serial.println("Archivo escrito con éxito.");
  } else {
    Serial.println("Error al abrir archivo.");
  }
}

void loop() {
  // No hace nada aquí
}
