#define DIR_LAT 14
#define STEP_LAT 27

#define DIR_LON 26
#define STEP_LON 25

// Microstepping 1/32 → 6400 pasos por vuelta
const float pasosPorRevolucionMicrostepping = 200.0 * 32.0;
const float gradosPorPaso = 360.0 / pasosPorRevolucionMicrostepping;  // 0.05625°

void setup() {
  pinMode(DIR_LAT, OUTPUT);
  pinMode(STEP_LAT, OUTPUT);
  pinMode(DIR_LON, OUTPUT);
  pinMode(STEP_LON, OUTPUT);

  Serial.begin(115200);
  Serial.println("ESP32 listo. Introduce LATITUD y LONGITUD (separadas por espacio, en grados). Ejemplo:");
  Serial.println("   45 -90");
}

void loop() {
  if (Serial.available()) {
    String linea = Serial.readStringUntil('\n');
    linea.trim();
    
    if (linea.length() == 0) return;

    int espacio = linea.indexOf(' ');
    if (espacio == -1) {
      Serial.println("Error: introduce ambos valores separados por espacio (ej. 45 -90)");
      return;
    }

    float latitud = linea.substring(0, espacio).toFloat();
    float longitud = linea.substring(espacio + 1).toFloat();

    Serial.print("Latitud: ");
    Serial.println(latitud);
    moverMotor(DIR_LAT, STEP_LAT, latitud);

    delay(500);

    Serial.print("Longitud: ");
    Serial.println(longitud);
    moverMotor(DIR_LON, STEP_LON, longitud);

    Serial.println("\nIntroduce nuevos valores de LATITUD y LONGITUD:");
  }
}

void moverMotor(int pinDir, int pinStep, float grados) {
  int pasos = abs(grados / gradosPorPaso);
  digitalWrite(pinDir, grados >= 0 ? HIGH : LOW);

  Serial.print("Moviendo ");
  Serial.print(pasos);
  Serial.println(" micro-pasos...");

  for (int i = 0; i < pasos; i++) {
    digitalWrite(pinStep, HIGH);
    delayMicroseconds(500);  // puedes reducir para más velocidad
    digitalWrite(pinStep, LOW);
    delayMicroseconds(500);
  }
}

