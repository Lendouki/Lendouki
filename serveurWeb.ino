#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
// Configuration Wi-Fi
const char* ssid = "ECG_ESP32";
const char* password = "azerty123";
// Configuration IP statique pour le mode AP
IPAddress local_IP(192, 168, 10, 1); // IP de l'ESP32
IPAddress gateway(192, 168, 10, 1);  // La passerelle est la même que l'ESP32
IPAddress subnet(255, 255, 255, 0); // Masque de sous-réseau
WebServer server(80);
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
  delay(1000);

  if (!SPIFFS.begin(true)) {
    Serial.println("Erreur d'initialisation de SPIFFS !");
    return;
}
  WiFi.begin(ssid, password);
  if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
      Serial.println("⚠️ Échec de la configuration de l'IP !");
  }

    // Lancer le mode Access Point
  WiFi.softAP(ssid, password);
  if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
    Serial.println("⚠️ Échec de la configuration de l'IP !");
  }
  Serial.println("✅ Point d'accès WiFi activé !");
  Serial.print("📡 Connectez-vous au réseau : ");
  Serial.println(ssid);
    
  Serial.print("🌐 Accédez à l'ESP32 via : ");
  Serial.println(WiFi.softAPIP());

    // Démarrer le serveur Web
  server.on("/", []() { server.send(200, "text/html", "<h1>Bienvenue sur ESP32</h1>"); });
  server.begin();

}
void loop() {
  
  server.handleClient();

}
