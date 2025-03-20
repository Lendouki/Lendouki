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
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Fiche Patient</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background-color: #f4f4f9;
      display: flex;
      justify-content: center;
      align-items: center;
      height: 100vh;
      margin: 0;
    }
    .form-container {
      background-color: white;
      border-radius: 8px;
      box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
      padding: 30px;
      width: 80%;
      max-width: 500px;
    }
    h2 {
      text-align: center;
      color: #333;
    }
    label {
      font-size: 16px;
      color: #555;
    }
    input[type="text"] {
      width: 100%;
      padding: 12px;
      margin: 10px 0;
      border: 1px solid #ccc;
      border-radius: 4px;
      box-sizing: border-box;
      font-size: 16px;
    }
    input[type="submit"] {
      width: 100%;
      padding: 14px;
      background-color: #4CAF50;
      color: white;
      border: none;
      border-radius: 4px;
      font-size: 16px;
      cursor: pointer;
      margin-top: 20px;
    }
    input[type="submit"]:hover {
      background-color: #45a049;
    }
    .button-container {
      text-align: center;
    }
    .button-container a {
      text-decoration: none;
      font-size: 16px;
      color: #007bff;
    }
    .button-container a:hover {
      text-decoration: underline;
    }
  </style>
</head>
<body>
  <div class="form-container">
    <h2>Fiche Patient ECG</h2>
    <form action="/enregistrer" method="POST">
      <label for="nom">Nom:</label><br>
      <input type="text" name="nom" required><br>

      <label for="telephone">Numero de telephone:</label><br>
      <input type="text" name="telephone"><br>

      <label for="numSecu">Numero de securite sociale:</label><br>
      <input type="text" name="numSecu" required><br>

      <label for="gs">Groupe sanguin:</label><br>
      <input type="text" name="gs" required><br><br>

      <input type="submit" value="Enregistrer">
    </form>
    <br>
    <div class="button-container">
      <a href='/print'><button>Imprimer la fiche patient</button></a>
    </div>
  </div>
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
      file.println("Telephone: " + telephone);
      file.println("Securite sociale: " + numSecu);
      file.println("Groupe sanguin: " + gs);
      file.close();

      // Réponse HTML centrée et avec texte plus gros
      String response = "<html><body style='text-align:center; font-size:24px;'>";
      response += "<h3>Donnees enregistrees!</h3>";
      response += "<p style='font-size:30px;'>Les informations du patient ont été correctement enregistrées.</p>";
      response += "<br><a href='/' style='font-size:20px; color: #007bff; text-decoration: none;'>Retour au formulaire</a>";
      response += "</body></html>";

      // Envoi de la réponse avec du HTML
      server.send(200, "text/html", response);

      Serial.println("Données enregistrées dans patient.txt");
    } else {
      server.send(500, "text/html", "<h3 style='text-align:center; color:red;'>Erreur d'enregistrement! Vérifiez l'accès au système ou le stockage de l'appareil</h3>");
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

void setup() {
  Serial.begin(115200);
  delay(1000);

  if (!SPIFFS.begin(true)) {
    Serial.println("Erreur d'initialisation de SPIFFS !");
    return;
  }

  WiFi.softAP(ssid, password);
  if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
    Serial.println("⚠️ Échec de la configuration de l'IP !");
  }

  Serial.println("✅ Point d'accès WiFi activé !");
  Serial.print("📡 Connectez-vous au réseau : ");
  Serial.println(ssid);
  Serial.print("🌐 Accédez à l'ESP32 via : ");
  Serial.println(WiFi.softAPIP());

  // Définir les routes du serveur
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", formulairePatient); // Afficher le formulaire
  });
  server.on("/enregistrer", HTTP_POST, handleForm);   // Route pour enregistrer les données
  server.on("/print", HTTP_GET, printPatientData);    // Route pour afficher la fiche patient
  server.begin();
}

void loop() {
  server.handleClient();
  Serial.println("En attente d'une connexion...");
  delay(1000);
}
