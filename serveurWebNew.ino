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
  <a href='/rechercher_patient'><button>Modifier les informations des patients</button></a>
</div>
<div class="button-container">
  <a href='/conditions'><button>Conditions d'Utilisation</button></a>
</div>
</div>
</body>
</html>
)";

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

String pageModificationPatient = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Modifier Patient</title>
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
      width: 90%;
      max-width: 500px;
    }
    h2 {
      text-align: center;
      color: #333;
    }
    label, input {
      display: block;
      width: 100%;
      font-size: 16px;
    }
    input[type="text"] {
      padding: 10px;
      margin: 10px 0;
      border: 1px solid #ccc;
      border-radius: 4px;
    }
    input[type="submit"] {
      padding: 12px;
      background-color: #4CAF50;
      color: white;
      border: none;
      border-radius: 4px;
      cursor: pointer;
      font-size: 16px;
      margin-top: 15px;
    }
    input[type="submit"]:hover {
      background-color: #45a049;
    }
  </style>
</head>
<body>
  <div class="form-container">
    <h2>Modifier les Informations du Patient</h2>
    <form action="/modifier_patient" method="POST">
      <label>Nom:</label>
      <input type="text" name="nom" value="%nom%" required>
      <label>Téléphone:</label>
      <input type="text" name="telephone" value="%telephone%">
      <label>Numéro de sécurité sociale:</label>
      <input type="text" name="numSecu" value="%numSecu%" required>
      <label>Groupe sanguin:</label>
      <input type="text" name="gs" value="%gs%" required>
      <input type="submit" value="Enregistrer les Modifications">
    </form>
  </div>
</body>
</html>
)rawliteral";


void afficherConditions() {
  server.send(200, "text/html", conditionsUtilisation);
}
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


// Fonction pour afficher la page de modification d'un patient
void afficherPageModificationPatient() {
  if (!server.hasArg("nom")) {
    server.send(400, "text/html", "<h3 style='color:red;text-align:center;'>Erreur: Nom du patient manquant !</h3>");
    return;
  }

  String nomASearch = server.arg("nom");
  nomASearch.trim();

  File file = SPIFFS.open("/patient.txt", FILE_READ);
  if (!file) {
    server.send(500, "text/html", "<h3 style='color:red;text-align:center;'>Erreur: Impossible d'accéder au fichier.</h3>");
    return;
  }

  String nom, telephone, numSecu, gs;
  bool patientTrouve = false;

  while (file.available()) {
    String ligne = file.readStringUntil('\n');
    if (ligne.startsWith("Nom: ")) {
      String nomLu = ligne.substring(5); // "Nom: " = 5 caractères
      if (nomLu.equalsIgnoreCase(nomASearch)) {
        patientTrouve = true;
        nom = nomLu;

        ligne = file.readStringUntil('\n');  // Téléphone
        if (ligne.startsWith("Téléphone: ")) {
          telephone = ligne.substring(11);
        }

        ligne = file.readStringUntil('\n');  // Numéro sécu
        if (ligne.startsWith("Numéro de sécurité sociale: ")) {
          numSecu = ligne.substring(30);
        }

        ligne = file.readStringUntil('\n');  // Groupe sanguin
        if (ligne.startsWith("Groupe sanguin: ")) {
          gs = ligne.substring(17);
        }

        break;
      }
    }
  }
  file.close();
  if (patientTrouve) {
    String page = pageModificationPatient;
    page.replace("%nom%", nom);
    page.replace("%telephone%", telephone);
    page.replace("%numSecu%", numSecu);
    page.replace("%gs%", gs);

    server.send(200, "text/html", page);
  } else {
    server.send(404, "text/html", "<h3 style='color:red;text-align:center;'>⚠️ Patient non trouvé !</h3>");
  }
}

// Fonction pour modifier un patient dans le fichier
void modifierPatient() {
  if (!server.hasArg("nom")) {
    server.send(400, "text/html", "<h3 style='color:red;text-align:center;'>Erreur: Nom du patient manquant !</h3>");
    return;
  }

  String nom = server.arg("nom");
  String telephone = server.arg("telephone");
  String numSecu = server.arg("numSecu");
  String gs = server.arg("gs");

  File file = SPIFFS.open("/patient.txt", FILE_READ);
  if (!file) {
    server.send(500, "text/html", "<h3 style='color:red;text-align:center;'>Erreur: Impossible d'accéder au fichier.</h3>");
    return;
  }

  String nouveauContenu = "";
  bool patientTrouve = false;

  while (file.available()) {
    String ligne = file.readStringUntil('\n');

    if (ligne.startsWith("Nom: ") && ligne.substring(5).equalsIgnoreCase(nom)) {
      patientTrouve = true;

      // Sauter les anciennes infos
      file.readStringUntil('\n');
      file.readStringUntil('\n');
      file.readStringUntil('\n');
      file.readStringUntil('\n');

      // Ajouter les nouvelles
      nouveauContenu += "Nom: " + nom + "\n";
      nouveauContenu += "Téléphone: " + telephone + "\n";
      nouveauContenu += "Numéro de sécurité sociale: " + numSecu + "\n";
      nouveauContenu += "Groupe sanguin: " + gs + "\n";
      nouveauContenu += "--------------------\n";
    } else {
      nouveauContenu += ligne + "\n";
    }
  }

  file.close();

  File newFile = SPIFFS.open("/patient.txt", FILE_WRITE);
  if (newFile) {
    newFile.print(nouveauContenu);
    newFile.close();

    if (patientTrouve) {
      server.send(200, "text/html", "<h3 style='color:green;text-align:center;'>✅ Informations modifiées avec succès !</h3><br><a href='/'>Retour</a>");
    } else {
      server.send(404, "text/html", "<h3 style='color:red;text-align:center;'>⚠️ Patient non trouvé !</h3><br><a href='/'>Retour</a>");
    }
  } else {
    server.send(500, "text/html", "<h3 style='color:red;text-align:center;'>Erreur: Impossible d'écrire dans le fichier.</h3>");
  }
}
// Page de suppression d'un patient
void afficherPageSuppressionPatient() {
  String reponse = "<html><head><meta charset='UTF-8'><style>";
  reponse += "body { font-family: Arial, sans-serif; text-align: center; background-color: #f4f4f9; }";
  reponse += ".container { max-width: 600px; margin: auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1); }";
  reponse += "h2 { color: #333; }";
  reponse += "input { padding: 10px; width: 80%; margin-top: 10px; }";
  reponse += "button { padding: 10px 20px; background: red; color: white; border: none; cursor: pointer; margin-top: 10px; }";
  reponse += "</style></head><body>";
  reponse += "<div class='container'>";
  reponse += "<h2>🗑 Supprimer un patient</h2>";
  reponse += "<form action='/supprimer_patients' method='POST'>";
  reponse += "<input type='text' name='nom' placeholder='Nom du patient' required>";
  reponse += "<form action='/supprimer_patient_action' method='POST'>";
  reponse += "<input type='text' name='nom' placeholder='Nom du patient' required>";
  reponse += "<br><br><button type='submit'>Supprimer</button>";;
  reponse += "</form>";
  reponse += "<br><a href='/'>⬅ Retour</a>";
  reponse += "</div></body></html>";

  server.send(200, "text/html", reponse);
}

// Fonction pour supprimer un patient par son NOM
void supprimerPatient() {
  // Vérifier si le nom est bien fourni dans la requête
  if (!server.hasArg("nom")) {  
    server.send(400, "text/html", "<h3 style='color:red;text-align:center;'>Erreur: Nom du patient manquant !</h3><br><a href='/'>⬅ Retour</a>");
    return;
  }

  String nomASupprimer = server.arg("nom");  // Récupération du nom
  nomASupprimer.trim(); // évite les espaces
  File file = SPIFFS.open("/patient.txt", FILE_READ);
  
  if (!file) {
    server.send(500, "text/html", "<html><head><meta charset='UTF-8'></head><body><h3 style='color:red;text-align:center;'>Erreur: Impossible d'accéder au fichier.</h3><br><a href='/'>⬅ Retour</a></body></html>");
    return;
  }

  String nouveauContenu = "";
  bool patientTrouve = false;
  bool suppressionEnCours = false;

  // Lire chaque ligne du fichier
  while (file.available()) {
    String ligne = file.readStringUntil('\n');

    // Si on trouve le nom dans une ligne commençant par "Nom: ", on déclenche la suppression
    if (ligne.startsWith("Nom: ") && ligne.indexOf(nomASupprimer) != -1) {
      suppressionEnCours = true;
      patientTrouve = true;
      continue;  // Ne pas ajouter cette ligne dans le nouveau fichier
    }

    // Si on détecte un nouveau patient (nouveau "Nom: "), on arrête la suppression
    if (suppressionEnCours && ligne.startsWith("Nom: ")) {
      suppressionEnCours = false;
    }

    // Ajouter la ligne dans le fichier SI elle ne fait pas partie du patient à supprimer
    if (!suppressionEnCours) {
      nouveauContenu += ligne + "\n";
    }
  }
  file.close();

  // Réécriture du fichier sans le patient supprimé
  File newFile = SPIFFS.open("/patient.txt", FILE_WRITE);
  newFile.print(nouveauContenu);
  newFile.close();

  // Réponse au client
  if (patientTrouve) {
    server.send(200, "text/html", "<html><head><meta charset='UTF-8'></head><body><h3 style='color:green;text-align:center;'>✅ Patient supprimé avec succès !</h3><br><a href='/'>⬅ Retour</a></body></html>");
  } else {
    server.send(404, "text/html", "<html><head><meta charset='UTF-8'></head><body><h3 style='color:red;text-align:center;'>⚠ Patient non trouvé !</h3><br><a href='/'>⬅ Retour</a></body></html>");
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
void supprimerHistorique() {
  // Vérifier que SPIFFS est bien initialisé
  if (!SPIFFS.begin()) {
    Serial.println("⚠️ Erreur d'initialisation de SPIFFS !");
    server.send(500, "text/html", "<html><head><meta charset='UTF-8'></head><body><h1>Erreur SPIFFS !</h1><p><a href='/'>Retour</a></p></body></html>");
    return;
  }

  // Formatage complet du SPIFFS (Supprime tous les fichiers)
  Serial.println("🧹 Formatage de SPIFFS en cours...");
  if (SPIFFS.format()) {
    Serial.println("✅ SPIFFS formaté avec succès !");
    server.send(200, "text/html", "<html><head><meta charset='UTF-8'></head><body><h1>Historique supprimé avec succès !</h1><p><a href='/'>Retour</a></p></body></html>");
  } else {
    Serial.println("❌ Échec du formatage de SPIFFS !");
    server.send(500, "text/html", "<html><head><meta charset='UTF-8'></head><body><h1>Échec de la suppression.</h1><p><a href='/'>Retour</a></p></body></html>");
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
  
  // Route pour afficher l'historique des patients
  server.on("/historique", HTTP_GET, afficherHistorique);
  
  // Route pour afficher la page de suppression de patient
  server.on("/supprimer_patients", HTTP_GET, afficherPageSuppressionPatient);
  
  // Route pour supprimer pour l'historique complet
  server.on("/supprimer_historique_complet", HTTP_GET, supprimerHistorique);
  
  // Route pour supprimer un patient (Action de suppression)
  server.on("/supprimer_patient", HTTP_GET, supprimerPatient);
  
  // Route pour enregistrer un patient
  server.on("/enregistrer", HTTP_POST, handleForm);
  
  // Route pour afficher le formulaire de modification d'un patient
  server.on("/rechercher_patient", HTTP_GET, afficherPageModificationPatient);
  server.on("/modifier_patient", HTTP_POST, modifierPatient);
  
  // Route pour la page de conditions d'utilisation
  server.on("/conditions", HTTP_GET, afficherConditions); 
  // Route pour afficher la fiche du patient (impression)
  server.on("/imprimer", HTTP_GET, printPatientData);
  server.begin();
}
void loop() {
  server.handleClient();
  delay(1000);  // Attends 1 seconde avant de réessayer
}    