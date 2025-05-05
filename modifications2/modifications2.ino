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
  <a href='/rechercher_patient'><button>Modifier les Informations</button></a>
</div>
<br>
<div class="button-container">
  <a href='/conditions'><button>Conditions d'Utilisation</button></a>
</div>
</div>
</body>
</html>
)";

String pageRecherchePatient = R"(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
  <title>Rechercher un Patient</title>
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
    button {
      padding: 10px 20px;
      background-color: #007bff;
      color: white;
      border: none;
      border-radius: 4px;
      font-size: 16px;
      cursor: pointer;
      margin: 5px;
    }
    button:hover {
      background-color: #0056b3;
    }
  </style>
</head>
<body>
  <div class="form-container">
    <h2>Rechercher un Patient</h2>
    <form action="/modifier_patient" method="GET">
      <label for="nom">Nom du patient:</label><br>
      <input type="text" name="nom" required><br><br>
      <input type="submit" value="Rechercher">
    </form>
    <br>
    <div class="button-container">
      <a href='/'><button>Retour</button></a>
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
    button {
      padding: 10px 20px;
      background-color: #007bff;
      color: white;
      border: none;
      border-radius: 4px;
      font-size: 16px;
      cursor: pointer;
      margin: 5px;
    }
    button:hover {
      background-color: #0056b3;
    }
  </style>
</head>
<body>
<div class="form-container">
  <h2>Modifier les Informations du Patient</h2>
  <form action="/modifier_patient" method="POST">
    <!-- Champs pour modifier uniquement les informations du patient -->
    
    <label for="nom">Nom:</label><br>
    <input type="text" name="nom" value="%nom%" required><br>

    <label for="telephone">Numéro de téléphone:</label><br>
    <input type="text" name="telephone" value="%telephone%"><br>

    <label for="numSecu">Numéro de sécurité sociale:</label><br>
    <input type="text" name="numSecu" value="%numSecu%" required><br>

    <label for="gs">Groupe sanguin:</label><br>
    <input type="text" name="gs" value="%gs%" required><br><br>

    <!-- Bouton pour envoyer les modifications -->
    <input type="submit" value="Enregistrer les Modifications">
  </form>
  <br>
  <div class="button-container">
    <a href='/'><button>Retour</button></a>
  </div>
  </div>
</body>
</html>
)rawliteral";
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
      file.println("Numéro de sécurité sociale: " + numSecu);
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

// Fonction pour afficher la page de modification d'un patient
void afficherPageModificationPatient() {
  String reponse = "<html><head><meta charset='UTF-8'>";
  if (!server.hasArg("nom")) {
    server.send(400, "text/html", "<h3 style='color:red;text-align:center;'>Nom manquant</h3>");
    return;
  }

  String nomRecherche = server.arg("nom");
  nomRecherche.trim();

  File file = SPIFFS.open("/patient.txt", FILE_READ);
  if (!file) {
    server.send(500, "text/html", "<h3>Erreur d'accès au fichier</h3>");
    return;
  }

  bool patientTrouve = false;
  String ligne, nom = "", telephone = "", secu = "", groupe = "";

  while (file.available()) {
    ligne = file.readStringUntil('\n');
    ligne.trim();
    
    if (ligne.startsWith("Nom: ")) {
      String nomLigne = ligne.substring(5);
      nomLigne.trim();
      
      if (nomLigne.equalsIgnoreCase(nomRecherche)) {
        patientTrouve = true;
        nom = nomLigne;
        
        // Lire les lignes suivantes
        if (file.available()) {
          telephone = file.readStringUntil('\n');
          telephone.trim();
        }
        if (file.available()) {
          secu = file.readStringUntil('\n');
          secu.trim();
        }
        if (file.available()) {
          groupe = file.readStringUntil('\n');
          groupe.trim();
        }
        
        // Enlever les préfixes
        if (telephone.startsWith("Téléphone: ")) telephone = telephone.substring(11);
        if (secu.startsWith("Numéro de sécurité sociale: ")) secu = secu.substring(29);
        if (groupe.startsWith("Groupe sanguin: ")) groupe = groupe.substring(16);
        
        break;
      }
    }
  }
  file.close();

  if (patientTrouve) {
    String pageModifiee = pageModificationPatient;
    // Remplacer les placeholders par les valeurs trouvées
    pageModifiee.replace("%nom%", nom);
    pageModifiee.replace("%telephone%", telephone);
    pageModifiee.replace("%numSecu%", secu);
    pageModifiee.replace("%gs%", groupe);
    
    // Ajouter un champ caché pour l'ancien nom (important pour retrouver le patient lors de la modification)
    pageModifiee.replace("<form action=\"/modifier_patient\" method=\"POST\">", 
                         "<form action=\"/modifier_patient\" method=\"POST\"><input type=\"hidden\" name=\"ancien_nom\" value=\"" + nom + "\">");
    
    server.send(200, "text/html; charset=UTF-8", pageModifiee);
  } else {
    server.send(404, "text/html; charset=UTF-8", "<h3 style='color:red;text-align:center;'>Patient non trouvé. Veuillez vérifier le nom.</h3>");
  }
}

// Modifiez votre fonction modifierPatient() pour utiliser l'ancien nom
void modifierPatient() {
  // Vérifier si c'est une requête GET pour la recherche ou POST pour la modification
  if (server.method() == HTTP_GET) {
    afficherPageModificationPatient();
    return;
  }

  // Vérifier si tous les champs nécessaires sont présents
  if (!server.hasArg("ancien_nom") || !server.hasArg("nom") || !server.hasArg("telephone") || !server.hasArg("numSecu") || !server.hasArg("gs")) {
    server.send(400, "text/html", "<h3 style='color:red;text-align:center;'>Erreur: Données incomplètes pour la modification !</h3>");
    return;
  }

  String ancienNom = server.arg("ancien_nom");
  String nouveauNom = server.arg("nom");
  String telephone = server.arg("telephone");
  String numSecu = server.arg("numSecu");
  String gs = server.arg("gs");

  File file = SPIFFS.open("/patient.txt", FILE_READ);
  if (!file) {
    server.send(500, "text/html", "<h3 style='color:red;text-align:center;'>Erreur: Impossible d'accéder au fichier.</h3>");
    return;
  }

  String contenuModifie = "";
  bool patientModifie = false;
  bool enTrainDeSauter = false;
  int lignesASauter = 0;

  while (file.available()) {
    String ligne = file.readStringUntil('\n');

    if (enTrainDeSauter) {
      lignesASauter--;
      if (lignesASauter <= 0) {
        enTrainDeSauter = false;
      }
      continue;
    }

    if (ligne.startsWith("Nom: ")) {
      String nomLigne = ligne.substring(5);
      nomLigne.trim();

      String nomRecherche = ancienNom;
      nomRecherche.trim();

      nomLigne.toLowerCase();
      nomRecherche.toLowerCase();

      if (nomLigne == nomRecherche) {
        // Patient trouvé, on remplace les données
        patientModifie = true;
        contenuModifie += "Nom: " + nouveauNom + "\n";
        contenuModifie += "Téléphone: " + telephone + "\n";
        contenuModifie += "Numéro de sécurité sociale: " + numSecu + "\n";
        contenuModifie += "Groupe sanguin: " + gs + "\n";
        contenuModifie += "--------------------\n";

        // Sauter les lignes suivantes (infos anciennes + ligne de séparation)
        enTrainDeSauter = true;
        lignesASauter = 4;
        continue;
      }
    }

    contenuModifie += ligne + "\n";
  }
  file.close();

  if (patientModifie) {
    File newFile = SPIFFS.open("/patient.txt", FILE_WRITE);
    if (!newFile) {
      server.send(500, "text/html", "<h3 style='color:red;text-align:center;'>Erreur: Écriture impossible dans le fichier.</h3>");
      return;
    }
    newFile.print(contenuModifie);
    newFile.close();

    String message = "<html><head><meta charset='UTF-8'>";
    message += "<style>body { font-family: Arial, sans-serif; text-align: center; font-size: 24px; background-color: #f4f4f9; }</style></head><body>";
    message += "<div style='max-width: 600px; margin: 50px auto; padding: 20px; background-color: white; border-radius: 10px; box-shadow: 0 4px 8px rgba(0,0,0,0.1);'>";
    message += "<h3 style='color:green;'>✅ Informations mises à jour avec succès !</h3>";
    message += "<p>Les données du patient ont été modifiées.</p>";
    message += "<a href='/historique'><button style='padding: 10px 20px; background-color: #4CAF50; color: white; border: none; border-radius: 4px; cursor: pointer;'>Voir l'historique</button></a> ";
    message += "<a href='/'><button style='padding: 10px 20px; background-color: #007bff; color: white; border: none; border-radius: 4px; cursor: pointer;'>Retour à l'accueil</button></a>";
    message += "</div></body></html>";

    server.send(200, "text/html; charset=UTF-8", message);
  } else {
    server.send(404, "text/html; charset=UTF-8", "<h3 style='color:red;text-align:center;'>⚠️ Patient non trouvé. Veuillez vérifier le nom.</h3>");
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

    server.send(200, "text/html; charset=UTF-8", reponse);
 
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
  // Route pour afficher le formulaire de modification d'un patient
  server.on("/modifier_patient", HTTP_ANY, modifierPatient);
  
  server.on("/rechercher_patient", HTTP_GET, []() {
    server.send(200, "text/html", pageRecherchePatient);
  });

  server.on("/historique", HTTP_GET, afficherHistorique);
  // Route pour enregistrer un patient 
  server.on("/enregistrer", HTTP_POST, handleForm);
  server.begin();
}

void loop() {
  server.handleClient();
  delay(10);  // Attends 10 millisecondes avant de réessayer

}
