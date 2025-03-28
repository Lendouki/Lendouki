#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID        "0000ffe0-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_UUID "0000ffe1-0000-1000-8000-00805f9b34fb"

BLECharacteristic *pCharacteristic;
BLEServer *pServer;

bool deviceConnected = false;

// Définir la classe MyServerCallbacks avant la fonction setup
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true; // Un périphérique s'est connecté
    Serial.println("Périphérique connecté !");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false; // Un périphérique s'est déconnecté
    Serial.println("Périphérique déconnecté.");
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Démarrage du Bluetooth...");

  // Réinitialiser la pile BLE
  BLEDevice::deinit();
  BLEDevice::init("ESP32_Test");

  // Créer un serveur BLE
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Créer un service BLE
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Créer une caractéristique BLE
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE
  );

  pCharacteristic->setValue("Hello from ESP32"); // Valeur initiale

  // Démarrer le service
  pService->start();

  // Configurer la publicité BLE
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->setScanResponse(true);  // Réponse de scan
  pAdvertising->start(); // Démarrer la publicité

  Serial.println("Publicité BLE démarrée, à la recherche de périphériques...");
}

void loop() {
  // Vérifie si un périphérique est connecté
  Serial.println("En attente de connexions BLE...");
  delay(1000);
}