#include "BluetoothSerial.h"

BluetoothSerial SerialBT; // Instance Bluetooth
int number = 0;  // Déclaration globale

void setup() {
  Serial.begin(115200);  // Débogage
  SerialBT.begin("ESP32_BT"); // Nom Bluetooth
  Serial.println("Bluetooth démarré ! Prêt à être appairé.");
}

void loop() {
  if (SerialBT.hasClient()) {  // Vérifie si un appareil est connecté
    number++; // Incrémentation normale
    SerialBT.println(number); // Envoi Bluetooth
    Serial.print("Envoyé : ");
    Serial.println(number);
    if (number > 9){
      number = 0;
    }
  } else {
    Serial.println("En attente d'une connexion...");
  }
  delay(1000); // Pause de 1s
}
