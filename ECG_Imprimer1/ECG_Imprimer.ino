#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

const int ecgPin = 36;
float alpha = 0.1;
float filteredValue = 0;
const int seuil = 1800;
unsigned long lastPeakTime = 0;
unsigned long currentTime = 0;

// Moyenne glissante BPM
const int bpmWindowSize = 5;
float bpmWindow[bpmWindowSize] = {0};
int bpmIndex = 0;
float bpmAvg = 0;

// BLE setup
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  }
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

void setup() {
  Serial.begin(115200);
  filteredValue = analogRead(ecgPin); // Initialise le filtre avec la première valeur

  // BLE setup
  BLEDevice::init("ECG_ESP32");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
  Serial.println("BLE prêt !");
}

void loop() {
  int rawValue = analogRead(ecgPin);
  filteredValue = alpha * rawValue + (1 - alpha) * filteredValue;

  // Détection des pics R
  currentTime = millis();
  if (filteredValue > seuil && (currentTime - lastPeakTime) > 300) {
    unsigned long interval = currentTime - lastPeakTime;
    float bpm = 60000.0 / interval;
    lastPeakTime = currentTime;

    // Moyenne glissante du BPM
    bpmWindow[bpmIndex] = bpm;
    bpmIndex = (bpmIndex + 1) % bpmWindowSize;
    float sum = 0;
    for (int i = 0; i < bpmWindowSize; i++) sum += bpmWindow[i];
    bpmAvg = sum / bpmWindowSize;
  }

  // Envoi BLE si connecté
  if (deviceConnected) {
    String data = "ECG: " + String(filteredValue, 2) + ", BPM: " + String(bpmAvg, 2);
    pCharacteristic->setValue(data.c_str());
    pCharacteristic->notify();
  }
  Serial.print(millis());
  Serial.print(",");
  Serial.print(filteredValue, 2);
  Serial.print(",");
  Serial.println(bpmAvg, 2);
  delay(25); // 40Hz
}
