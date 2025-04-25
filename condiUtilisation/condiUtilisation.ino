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
String conditionsUtilisation = R"(
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Conditions d'Utilisation</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background-color: #f4f4f9;
      color: #333;
      margin: 0;
      padding: 0;
    }
    .container {
      width: 80%;
      margin: auto;
      padding: 20px;
      background-color: white;
      box-shadow: 0 4px 10px rgba(0, 0, 0, 0.1);
      border-radius: 8px;
    }
    h1 {
      text-align: center;
      color: #333;
    }
    p {
      font-size: 18px;
      line-height: 1.6;
      margin: 10px 0;
    }
    .footer {
      text-align: center;
      margin-top: 20px;
    }
    a {
      color: #007bff;
      text-decoration: none;
    }
    a:hover {
      text-decoration: underline;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Conditions d'Utilisation</h1>
    <p>Merci de lire attentivement ces conditions d'utilisation avant d'utiliser ce système de collecte de données ECG.</p>

    <h2>1. Objet du service</h2>
    <p>Ce service permet la saisie, la sauvegarde et l'analyse de données ECG (électrocardiogramme) à des fins médicales ou de recherche clinique.</p>

    <h2>2. Données collectées</h2>
    <p>Les données suivantes peuvent être collectées :</p>
    <ul>
      <li>Nom</li>
      <li>Numéro de sécurité sociale</li>
      <li>Groupe sanguin</li>
      <li>Données ECG enregistrées</li>
    </ul>

    <h2>3. Consentement</h2>
    <p>En utilisant ce système, vous consentez expressément à la collecte et au traitement des données à caractère personnel nécessaires au bon fonctionnement du service.</p>

    <h2>4. Protection des données (RGPD)</h2>
    <p>Conformément au RGPD (Règlement UE 2016/679), les données collectées sont :</p>
    <ul>
      <li><strong>Limitées</strong> à ce qui est strictement nécessaire (minimisation des données).</li>
      <li><strong>Sécurisées</strong> via des protocoles de chiffrement et un accès restreint.</li>
      <li><strong>Stockées localement</strong> sur l'ESP32 et ne sont pas partagées sans votre accord.</li>
      <li><strong>Non conservées indéfiniment</strong> : l'utilisateur peut supprimer ses données à tout moment via une interface dédiée.</li>
      <li><strong>Accessibles sur demande</strong> : toute personne peut demander la consultation, la modification ou la suppression de ses données.</li>
    </ul>

    <h2>5. Droits de l'utilisateur</h2>
    <p>Vous disposez des droits suivants :</p>
    <ul>
      <li>Droit d'accès à vos données</li>
      <li>Droit de rectification</li>
      <li>Droit à l'effacement (« droit à l'oubli »)</li>
      <li>Droit à la portabilité</li>
      <li>Droit d'opposition</li>
    </ul>
    <p>Pour exercer ces droits, veuillez contacter le responsable du traitement des données, via l'adresse indiquée par l'opérateur de ce dispositif.</p>

    <h2>6. Limitation de responsabilité</h2>
    <p>Ce système est fourni à des fins expérimentales ou de suivi médical et ne saurait remplacer un diagnostic médical professionnel.</p>

    <div class="footer">
      <a href="/">⬅ Retour à l'accueil</a>
    </div>
  </div>
</body>
</html>
)";
void afficherConditions() {
  server.send(200, "text/html", conditionsUtilisation);
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
  // Route pour la page de conditions d'utilisation
  server.on("/conditions", HTTP_GET, afficherConditions);
  server.begin();
}

void loop() {
  server.handleClient();
  delay(1000);  // Attends 1 seconde avant de réessayer

}
