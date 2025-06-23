#include <HX711.h>
#include <SPI.h>
#include <SD.h>

#define DT 13
#define SCK 12
#define SD_CS 5

HX711 balanza;

// Reemplaza este número con tu factor calibrado
float factor = -835.0;

void setup() {
  Serial.begin(115200);

  // Inicializar SD
  if (!SD.begin(SD_CS)) {
    Serial.println("Error al iniciar la tarjeta SD.");
    return;
  }
  Serial.println("Tarjeta SD lista.");

  // Iniciar balanza
  balanza.begin(DT, SCK);
  balanza.set_scale(factor);
  balanza.tare();

  Serial.println("Balanza lista. Comenzando lecturas...");
}

void loop() {
  if (balanza.is_ready()) {
    float peso = balanza.get_units(5); // promedio 5 lecturas

    Serial.print("Peso (g): ");
    Serial.println(peso, 2);

    File archivo = SD.open("/pesos.csv", FILE_APPEND);
    if (archivo) {
      archivo.print("Peso (g): ");
      archivo.println(peso, 2);
      archivo.close();
      Serial.println("Dato guardado en SD.");
    } else {
      Serial.println("Error al abrir archivo.");
    }
  } else {
    Serial.println("Balanza no está lista.");
  }

  delay(2000); // espera 2 segundos entre lecturas
}
