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
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
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

      <label for="telephone">Numéro de téléphone:</label><br>
      <input type="text" name="telephone"><br>

      <label for="numSecu">Numéro de sécurité sociale:</label><br>
      <input type="text" name="numSecu" required><br>

      <label for="gs">Groupe sanguin:</label><br>
      <input type="text" name="gs" required><br><br>

      <input type="submit" value="Enregistrer">
    </form>
    <br>
    <div class="button-container">
      <a href='/imprimer'><button>Imprimer la fiche patient</button></a>
    </div>
    <br>
    <div class="button-container">
  <a href='/historique'><button>Voir l'historique</button></a>
  </div>
<br>
<div class="button-container">
  <a href='/modifier_patient'><button>Modifier les Informations</button></a>
</div>
<br>
<div class="button-container">
  <a href='/conditions'><button>Conditions d'Utilisation</button></a>
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
    
    // Créer le fichier patient.txt s'il n'existe pas encore
    File file = SPIFFS.open("/patient.txt", FILE_READ);
    if (!file) {
      file = SPIFFS.open("/patient.txt", FILE_WRITE);  // Si le fichier n'existe pas, on le crée
      if (!file) {
        server.send(500, "text/html", "<h3 style='text-align:center;color:red;'>Erreur d'initialisation du fichier patient.txt !</h3>");
        return;
      }
      file.close();
    }

    // Enregistrer les données du patient
    file = SPIFFS.open("/patient.txt", FILE_APPEND);  // Ajout de nouvelles données
    if (file) {
      file.println("Nom: " + nom);
      file.println("Téléphone: " + telephone);
      file.println("Numéro de sécurité sociale: " + numSecu);
      file.println("Groupe sanguin: " + gs);
      file.println("--------------------");
      file.close();
      
      String reponse = "<html><head><meta charset='UTF-8'>";
      reponse += "<style>body { font-family: Arial, sans-serif; text-align: center; font-size: 24px; background-color: #f4f4f9; }</style></head><body>";
      reponse += "<div class='container'><h3>✅ Données enregistrées !</h3><p>Les informations du patient ont été correctement enregistrées.</p>";
      reponse += "<button onclick=\"window.location.href='/historique'\">📜 Voir l'historique</button>";
      reponse += "<br><a href='/'>⬅ Retour au formulaire</a></div></body></html>";

      server.send(200, "text/html; charset=UTF-8", reponse);
      Serial.println("Données enregistrées dans patient.txt");
    } else {
      server.send(500, "text/html", "<h3 style='color:red;text-align:center;'>Erreur d'enregistrement! Vérifiez l'accès au système ou le stockage de l'appareil</h3>");
    }
  }
}
void printPatientData() { // Fonction pour afficher la fiche patient bien formatée
  File file = SPIFFS.open("/patient.txt", FILE_READ);
  if (file) {
    String reponse = "<html><head><meta charset='UTF-8'>";
    reponse += "<style>";
    reponse += "body { font-family: Arial, sans-serif; text-align: center; font-size: 24px; background-color: #f4f4f9; }";
    reponse += ".container { max-width: 600px; margin: auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1); }";
    reponse += "h2 { color: #333; font-size: 32px; }";
    reponse += "pre { text-align: left; background: #fff; padding: 15px; font-size: 26px; border-radius: 5px; }";
    reponse += "button { padding: 10px 20px; font-size: 20px; background: #4CAF50; color: white; border: none; border-radius: 5px; cursor: pointer; }";
    reponse += "button:hover { background: #45a049; }";
    reponse += "a { display: block; margin-top: 15px; font-size: 20px; color: #007bff; text-decoration: none; }";
    reponse += "a:hover { text-decoration: underline; }";
    reponse += "</style></head><body>";

    reponse += "<div class='container'>";
    reponse += "<h2>🩺 Fiche Patient</h2>";
    reponse += "<pre>";

    while (file.available()) {
      reponse += (char)file.read();
    }
    file.close();
 
    reponse += "</pre>";
    reponse += "<button onclick='window.print()'>Imprimer</button>"; 
    reponse += "<br><a href='/'>⬅ Retour</a>";

    reponse += "</div></body></html>";

    server.send(200, "text/html; charset=UTF-8", reponse);
  } else {
    server.send(500, "text/html", "<html><head><meta charset='UTF-8'></head><body><h3 style='color:red; text-align:center;'>Erreur: Aucune donnée patient trouvée.</h3></body></html>");
  }
}
// Afficher l'historique des patients
void afficherHistorique() {
  File file = SPIFFS.open("/patient.txt", FILE_READ);
  if (file) {
    String reponse = "<html><head><meta charset='UTF-8'><style>";
    reponse += "body { font-family: Arial, sans-serif; text-align: center; background-color: #f4f4f9; }";
    reponse += ".container { max-width: 600px; margin: auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1); }";
    reponse += "h2 { color: #333; } pre { text-align: left; background: white; padding: 15px; border-radius: 5px; font-size: 18px; }";
    reponse += "input { padding: 10px; width: 80%; margin-top: 10px; font-size: 16px; }";
    reponse += "button { padding: 10px 20px; background: red; color: white; border: none; cursor: pointer; margin-top: 10px; font-size: 16px; }";
    reponse += "button:hover { background: darkred; }";
    reponse += ".delete-all { background: black; }";
    reponse += ".delete-all:hover { background: darkgray; }";
    reponse += "</style></head><body><div class='container'>";

    reponse += "<h2>📜 Historique des patients</h2><pre>";

    // Lire et afficher tout le fichier patient.txt
    while (file.available()) {
      reponse += (char)file.read();
    }
    file.close();

    reponse += "</pre>";

    // Formulaire pour supprimer un patient spécifique par son NOM
    reponse += "<h3>🔍 Supprimer un patient</h3>";
    reponse += "<form action='/supprimer_patient' method='GET'>";
    reponse += "<input type='text' name='nom' placeholder='Nom du patient' required>";
    reponse += "<button type='submit'>❌ Supprimer</button>";
    reponse += "</form>";

    // Bouton pour effacer tout l'historique
    reponse += "<br><br><form action='/supprimer_historique_complet' method='GET'>";
    reponse += "<button class='delete-all' type='submit'>🗑 Supprimer tout l'historique</button>";
    reponse += "</form>";

    reponse += "<br><a href='/'>⬅ Retour</a>";
    reponse += "</div></body></html>";

    server.send(200, "text/html", reponse);
  } else {
    server.send(500, "text/html", "<html><head><meta charset='UTF-8'></head><body><h3 style='color:red;text-align:center;'>Erreur: Aucune donnée patient trouvée.</h3></body></html>");
  }
}
void setup() {
  Serial.begin(115200);
  delay(1000);

  if (!SPIFFS.begin(true)) {
    Serial.println("Erreur d'initialisation de SPIFFS !");
    return;
  }
  Serial.println("Tapez 'delete' dans le moniteur série pour supprimer l'historique.");

  WiFi.softAP(ssid, password);
  Serial.println("Tentative de démarrer l'AP...");
  if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
    Serial.println("⚠️ Échec de la configuration de l'IP !");
  } else {
    Serial.println("✅ Configuration de l'IP réussie !");
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
    // Route pour afficher la fiche du patient (impression)
  server.on("/imprimer", HTTP_GET, printPatientData);
  // Route pour enregistrer un patient
  server.on("/enregistrer", HTTP_POST, handleForm);
    // Route pour afficher l'historique des patients
  server.on("/historique", HTTP_GET, afficherHistorique);
  server.begin();
}

void loop() {
  server.handleClient();
  delay(1000);  // Attends 1 seconde avant de réessayer

}
