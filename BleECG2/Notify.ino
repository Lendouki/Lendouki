/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updated by chegewara

   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 4fafc201-1fb5-459e-8fcc-c5c9c331914b
   And has a characteristic of: beb5483e-36e1-4688-b7f5-ea07361b26a8

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   A connect handler associated with the server starts a background task that performs notification
   every couple of seconds.
*/
#include <WebServer.h> 
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLE2901.h>
#include <SPIFFS.h>
BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
BLE2901 *descriptor_2901 = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;
// Configuration Wi-Fi
const char* ssid = "Redmi 12";
const char* password = "azerty123";
IPAddress local_ip(192, 168, 1, 100);  // adresse de l'esp32
IPAddress gateway(192, 168, 1, 1);     // Adresse de la passerelle 
IPAddress subnet(255, 255, 255, 0);    // Masque de sous-réseau
WebServer server(80); // le port de l'esp32 qui lira http://192.168.1.100:80 pour acceder au serveur
//stocke le dernier echantillon brut du capteur
int Ecgbrute = 0;
// les valeurs brutes précédentes du signal
float mesure1 = 0, mesure2 = 0, mesure3 = 0; 

// les valeurs filtrées du signal
float mesureF0 = 0, mesureF1 = 0, mesureF2 = 0, mesureF3 = 0;

const int ECG = 3 ;  // port de lecture ESP32



// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
  };

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
  }
};
String formulairePatient = R"(
<!DOCTYPE html>
<html>
<head>
  <title>Fiche Patient</title>
</head>
<body>
  <h2>Fiche Patient ECG</h2>
  <form action="/enregistrer" method="POST">
    <label>Nom:</label><br>
    <input type="text" name="nom" required><br>
    <label>Numéro de téléphone:</label><br>
    <input type="text" name="telephone" ><br>
    <label>Numéro de sécurité sociale:</label><br>
    <input type="text" name="numSecu" required><br>
    <label>Groupe sanguin:</label><br>
    <input type="text" name="gs" required><br><br>
    <input type="submit" value="Enregistrer">
  </form>
  <br>
    <a href='/print'><button>Imprimer la fiche patient</button></a>
</body>
</html>
)";
void handleForm() {
  if (server.method() == HTTP_POST) {
    String nom = server.arg("nom");
    String telephone = server.arg("telephone");
    String numSecu = server.arg("numSecu");
    String gs = server.arg("gs");

    // Stocker dans la fiche patient dans un fichier texte
    File file = SPIFFS.open("/patient.txt", FILE_WRITE);
    if (file) {
      file.println("Nom: " + nom);
      file.println("Téléphone: " + telephone);
      file.println("Sécurité sociale: " + numSecu);
      file.println("Groupe sanguin: " + gs);
      file.close();
      server.send(200, "text/html", "<h3>Données enregistrées!</h3>");
      
      file.close();
      Serial.println("Données enregistrées dans patient.txt");
    }
    else {
      server.send(500, "text/html", "<h3>Erreur d'enregistrement! Vérifier l'accès au système ou le stockage de l'appareil</h3>");
    }
  }
}
void printPatientData() { // fonction pour afficher la fiche patient
  File file = SPIFFS.open("/patient.txt", FILE_READ);
  if (file) {
    String data = "<html><body><h2>Fiche Patient</h2><pre>";
    while (file.available()) {
      data += (char)file.read();
    }
    file.close();
    data += "</pre><br><button onclick='window.print()'>Imprimer</button>"; 
    data += "<br><a href='/'>Retour</a></body></html>";
    server.send(200, "text/html", data);
  } else {
    server.send(500, "text/html", "<h3>Erreur: Aucune donnée patient trouvée.</h3>");
  }
}
void saveECGData(int EcgValue) {
  File file = SPIFFS.open("/cycle_ecg.txt", FILE_APPEND); // Ouvrir en mode append pour ajouter des données sans écraser les anciennes
  if (file) {
    file.println(EcgValue); // Ajouter la valeur de l'ECG
    file.close();           // Fermer le fichier après l'écriture
    Serial.println("Cycle ECG enregistré.");
} 
  else {
    Serial.println("Erreur d'ouverture du fichier.");
  }
}
void showECGData() {
  File file = SPIFFS.open("/cycle_ecg.txt", FILE_READ); // Ouvrir le fichier en mode lecture
  if (file) {
    String data = "";
    while (file.available()) {
      data += (char)file.read(); // Lire chaque caractère
    }
    file.close();
    server.send(200, "text/html", "<h3>Données ECG enregistrées:</h3><pre>" + data + "</pre>");
} else {
    server.send(500, "text/html", "<h3>Erreur lors de la lecture des données ECG.</h3>");
  }
}
   
void setup() {
  Serial.begin(115200);
//definir la broche de l'ECG en entrée
  pinMode(ECG, INPUT);  
    // Initialiser SPIFFS pour le stockage
  if (!SPIFFS.begin()) {
    Serial.println("Erreur SPIFFS!");
    return;
  }

  // Connexion Wi-Fi avec une adresse IP fixe
  WiFi.config(local_ip, gateway, subnet); // Configurer l'adresse IP
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnecté au Wi-Fi avec IP: " + WiFi.localIP().toString());

  // Configuration des routes du serveur web 
  // Définition des routes HTTP
    server.on("/", []() { 
        server.send(200, "text/html", "<h3>Bienvenue sur l'ESP32</h3>"); 
    });
  server.on("/form", HTTP_POST, handleForm);
  server.on("/", []() { server.send(200, "text/html", formulairePatient); });
  server.on("/enregistrer", handleForm);
  server.on("/show_data", showECGData);
  server.on("/print", printPatientData);
  server.begin();

  // Create the BLE Device
  BLEDevice::init("ESP32_BT");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE
  );

  // Creates BLE Descriptor 0x2902: Client Characteristic Configuration Descriptor (CCCD)
  pCharacteristic->addDescriptor(new BLE2902());
  // Adds also the Characteristic User Description - 0x2901 descriptor
  descriptor_2901 = new BLE2901();
  descriptor_2901->setDescription("My own description for this characteristic.");
  descriptor_2901->setAccessPermissions(ESP_GATT_PERM_READ);  // enforce read only - default is Read|Write
  pCharacteristic->addDescriptor(descriptor_2901);

  // Start the service
  pService->start();
  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
  delay(1000);
}
void loop() {

  server.handleClient(); // Gère les requêtes HTTP entrantes
  Serial.println("En attente d'une connexion...");
  delay(1000);
// on fait glisser chaque valeur afin de stocker l'historique du signal et c'est la meme pour les valeurs filtrées
  mesure3 = mesure2;
  mesure2 = mesure1;
  mesure1 = Ecgbrute;

  mesureF3 = mesureF2;
  mesureF2 = mesureF1;
  mesureF1 = mesureF0;

  // Lecture de la valeur brute du capteur ECG
  Ecgbrute = analogRead(ECG);

  // lecture de la valeur filtré du capteur ECG
  // mesure_f0 =(0.2066 * EcgRawValue) - (0.2066 * mesure2) - (1.373 * mesureF1) + (0.6216 * mesureF2); // il faut que je vérifie les coef avec Josy
  Ecgbrute = mesureF0;
  saveECGData(Ecgbrute);
  //Serial.println(EcgRawValue);
  Serial.println(mesureF0);
  server.handleClient();
  delay(500);

  // notify changed value
  if (deviceConnected) {
    pCharacteristic->setValue((uint8_t *)&mesureF0, 4);
    pCharacteristic->notify();
    value++;
    delay(25);
  }
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);                   // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising();  // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }
}

