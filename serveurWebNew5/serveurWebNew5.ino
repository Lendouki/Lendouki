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

const char* login = "";     // Identifiant de l'administrateur
const char* mdp = "";       // Mot de passe de l'administrateur

bool estAuthentifie() {
  return server.authenticate(login, password);
}

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
      <input type="text" name="nom" pattern="[A-Za-zÀ-ÿ\s]+" title="Lettres uniquement" required><br>

      <label for="telephone">Numéro de téléphone:</label><br>
      <input type="text" name="telephone" pattern="\d{10}" title="10 chiffres requis" required><br>

      <label for="numSecu">Numéro de sécurité sociale:</label><br>
      <input type="text" name="numSecu" pattern="\d{15}" title="15 chiffres requis" required><br>

      <label for="gs">Groupe sanguin:</label><br>
      <input type="text" name="gs" pattern="^(A|B|AB|O)[+-]$" title="Ex: A+, AB-, O+" required><br><br>

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
  <h1>Conditions d'Utilisation et Politique de Confidentialité</h1>
  <p>Merci de lire attentivement ces conditions avant d'utiliser ce système de collecte de données ECG.</p>

  <h2>1. Objet du service</h2>
  <p>Ce dispositif permet la saisie, le traitement et la sauvegarde de données ECG (électrocardiogramme) à des fins de suivi médical ou de recherche clinique. Il ne remplace pas un avis médical professionnel.</p>

  <h2>2. Données collectées</h2>
  <p>Dans le cadre de son fonctionnement, le système peut collecter les données suivantes :</p>
  <ul>
    <li>Données ECG (signaux électriques du cœur)</li>
    <li>Nom</li>
    <li>Numéro de téléphone </li>
    <li>Numéro de sécurité sociale</li>
    <li>Groupe sanguin</li>

  </ul>

  <h2>4. Protection des données (RGPD)</h2>
  <p>Conformément au Règlement Général sur la Protection des Données (RGPD – UE 2016/679), les données collectées respectent les principes suivants :</p>
  <ul>
    <li><strong>Licéité, loyauté et transparence :</strong> les données sont traitées de manière claire, avec le consentement de la personne concernée.</li>
    <li><strong>Minimisation :</strong> seules les données strictement nécessaires sont collectées.</li>
    <li><strong>Sécurité :</strong> les données sont stockées localement (sur l’ESP32) et protégées par des protocoles de chiffrement et des accès restreints.</li>
    <li><strong>Durée de conservation limitée :</strong> les données peuvent être effacées par l’utilisateur via une interface dédiée.</li>
    <li><strong>Droits des utilisateurs :</strong> l'utilisateur peut accéder, modifier ou supprimer ses données à tout moment.</li>
  </ul>

  <h2>5. Vos droits</h2>
  <p>Conformément à la réglementation, vous disposez des droits suivants sur vos données personnelles :</p>
  <ul>
    <li>Droit d'accès</li>
    <li>Droit de rectification</li>
    <li>Droit à l'effacement (« droit à l’oubli »)</li>
    <li>Droit à la portabilité</li>
    <li>Droit d’opposition</li>
  </ul>

  <h2>6. Responsabilité</h2>
  <p>Ce système est fourni à titre expérimental ou comme support de suivi. Il ne remplace en aucun cas un diagnostic ou un avis médical professionnel.</p>

  <div class="footer">
    <a href="/">Retour à l'accueil</a>
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
    
    
    <label for="nom">Nom:</label><br>
    <input type="text" name="nom" pattern="[A-Za-zÀ-ÿ\s]+" title="Lettres uniquement" value="%nom%" required><br>

    <label for="telephone">Numéro de téléphone:</label><br>
    <input type="text" name="telephone" pattern="\d{10}" title="10 chiffres requis" value="%telephone%" required><br>

    <label for="numSecu">Numéro de sécurité sociale:</label><br>
    <input type="text" name="numSecu" pattern="\d{15}" title="15 chiffres requis" value="%numSecu%" required><br>

    <label for="gs">Groupe sanguin:</label><br>
    <input type="text" name="gs" pattern="^(A|B|AB|O)[+-]$" title="Ex: A+, AB-, O+" value="%gs%" required><br><br>

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

// Affiche les conditions d'utilisation en HTML
void afficherConditions() {
  server.send(200, "text/html", conditionsUtilisation);
}
// Gère la soumission du formulaire patient
void enregistrement() { 
  if (server.method() == HTTP_POST) { 
    
    String nom = server.arg("nom");
    String telephone = server.arg("telephone");
    String numSecu = server.arg("numSecu");
    String gs = server.arg("gs");

    // Ouverture du fichier pour ajouter les données
    File file = SPIFFS.open("/patient.txt", FILE_WRITE);
    if (file) {
      file.println("Nom: " + nom);
      file.println("Téléphone: " + telephone);
      file.println("Numéro de sécurité sociale: " + numSecu);
      file.println("Groupe sanguin: " + gs);
      file.close();
      server.send(200, "text/html", "<html><head><meta charset='UTF-8'></head><body><h3>Données enregistrées!</h3></body></html>");
      
      file.close();
      // Réponse HTML après enregistrement
      Serial.println("Données enregistrées dans patient.txt");
    }
    else { // Réponse en cas d'erreur d'écriture
      server.send(500, "text/html", "<html><head><meta charset='UTF-8'></head><body><h3>Erreur d'enregistrement ! Vérifier l'accès au système ou le stockage de l'appareil</h3></body></html>");

    }
  }
}
// Affiche l'historique de tous les patients enregistrés
void afficherHistorique() { // Ouvre le fichier patient en lecture
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

    reponse += "<h2> Historique des patients</h2><pre>";

    // Lire et afficher tout le fichier patient.txt
    while (file.available()) {
      reponse += (char)file.read();
    }
    file.close();

    reponse += "</pre>";

    // Formulaire pour supprimer un patient spécifique par son NOM
    reponse += "<h3> Supprimer un patient</h3>";
    reponse += "<form action='/supprimer_patient' method='GET'>";
    reponse += "<input type='text' name='nom' placeholder='Nom du patient' required>";
    reponse += "<button type='submit'> Supprimer</button>";
    reponse += "</form>";

    // Bouton pour effacer tout l'historique
    reponse += "<br><br><form action='/supprimer_historique_complet' method='GET'>";
    reponse += "<button class='delete-all' type='submit'>🗑 Supprimer tout l'historique</button>";
    reponse += "</form>";

    reponse += "<br><a href='/'>⬅ Retour</a>";
    reponse += "</div></body></html>";
    server.send(200, "text/html", reponse);
  } else {
    server.send(500, "text/html", "<html><head><meta charset='UTF-8'></head><body><h3 style='color:red;text-align:center;'>Erreur: Aucunes données d'un patient n'a été trouvées.</h3></body></html>");
  }
}


// Fonction pour afficher la page de modification d'un patient
void afficherPageModificationPatient() {
  String reponse = "<html><head><meta charset='UTF-8'>";
   // Vérifie si un nom a été fourni dans la requête
  if (!server.hasArg("nom")) {
    server.send(400, "text/html", "<h3 style='color:red;text-align:center;'>Nom manquant</h3>");
    return;
  }
  // Récupère le nom fourni et le nettoie
  String nomRecherche = server.arg("nom");
  nomRecherche.trim();

  File file = SPIFFS.open("/patient.txt", FILE_READ);
  if (!file) {
    server.send(500, "text/html", "<h3>Erreur d'accès au fichier</h3>");
    return;
  }
  // Variables pour stocker les données du patient trouvé
  bool patientTrouve = false;
  String ligne, nom = "", telephone = "", secu = "", groupe = "";

  // Lecture ligne par ligne pour trouver le patient correspondant
  while (file.available()) {
    ligne = file.readStringUntil('\n');
    ligne.trim();
    
    if (ligne.startsWith("Nom: ")) {
      String nomLigne = ligne.substring(5);
      nomLigne.trim();
      
      if (nomLigne.equalsIgnoreCase(nomRecherche)) {
        patientTrouve = true;
        nom = nomLigne;
        
        // Lire et extraire les informations suivantes du patient
        if (file.available()) {
          ligne = file.readStringUntil('\n');
          ligne.trim();
          if (ligne.startsWith("Téléphone: ")) {
            telephone = ligne.substring(12);
          }
        }
        
        if (file.available()) {
          ligne = file.readStringUntil('\n');
          ligne.trim();
          if (ligne.startsWith("Numéro de sécurité sociale: ")) {
            secu = ligne.substring(30);
          }
        }
        
        if (file.available()) {
          ligne = file.readStringUntil('\n');
          ligne.trim();
          if (ligne.startsWith("Groupe sanguin: ")) {
            groupe = ligne.substring(16);
          }
        }
        
        break; // Arrête la lecture une fois le patient trouvé
      }
    }
  }
  file.close();

  // Si patient trouvé, on affiche la page de modification avec les champs remplis
  if (patientTrouve) {
    String pageModifiee = pageModificationPatient;
    
    // Remplace les champs avec les données récupérées
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

// Fonction pour modifier les données d'un patient existant
void modifierPatient() {
 // Si c'est une requête GET, on affiche la page de modification
  if (server.method() == HTTP_GET) {
    afficherPageModificationPatient();
    return;
  }

   // Vérifie que tous les champs requis sont présents dans la requête POST
  if (!server.hasArg("ancien_nom") || !server.hasArg("nom") || !server.hasArg("telephone") || !server.hasArg("numSecu") || !server.hasArg("gs")) {
    server.send(400, "text/html", "<h3 style='color:red;text-align:center;'>Erreur: Données incomplètes pour la modification !</h3>");
    return;
  }

// Récupère les nouvelles données depuis le formulaire
  String ancienNom = server.arg("ancien_nom");
  String nouveauNom = server.arg("nom");
  String telephone = server.arg("telephone");
  String numSecu = server.arg("numSecu");
  String gs = server.arg("gs");

  // Ouvre le fichier des patients en lecture
  File file = SPIFFS.open("/patient.txt", FILE_READ);
  if (!file) { // Erreur si le fichier ne peut pas être ouvert
    server.send(500, "text/html", "<h3 style='color:red;text-align:center;'>Erreur: Impossible d'accéder au fichier.</h3>");
    return;
  }
// Variables pour créer le nouveau contenu modifié
  String contenuModifie = "";
  bool patientModifie = false;  // Pour savoir si un patient a été trouvé et modifié
  bool enTrainDeSauter = false; // Pour ignorer les anciennes lignes du patient modifié
  int lignesASauter = 0;        // Nombre de lignes à ignorer (4 après "Nom: ")

 // Lecture du fichier ligne par ligne
  while (file.available()) {
    String ligne = file.readStringUntil('\n');
    // Si on est en train de sauter les anciennes données du patient, on diminue le compteur
    if (enTrainDeSauter) {
      lignesASauter--;
      if (lignesASauter <= 0) {
        enTrainDeSauter = false; // Fin des lignes à ignorer
      }
      continue;
    }
    // Si la ligne contient un nom, on vérifie s’il correspond à l’ancien nom
    if (ligne.startsWith("Nom: ")) {
      String nomLigne = ligne.substring(5); // Extrait le nom (après "Nom: ")
      nomLigne.trim();

      String nomRecherche = ancienNom;
      nomRecherche.trim();

      // Comparaison insensible à la casse
      nomLigne.toLowerCase();
      nomRecherche.toLowerCase();

      if (nomLigne == nomRecherche) {
        // Si le nom correspond, on remplace les anciennes données par les nouvelles
        patientModifie = true;
        contenuModifie += "Nom: " + nouveauNom + "\n";
        contenuModifie += "Téléphone: " + telephone + "\n";
        contenuModifie += "Numéro de sécurité sociale: " + numSecu + "\n";
        contenuModifie += "Groupe sanguin: " + gs + "\n";
        contenuModifie += "--------------------\n";

        // Active le saut des lignes suivantes de l'ancien patient (4 lignes à ignorer)
        enTrainDeSauter = true;
        lignesASauter = 4;
        continue;
      }
    }
    // Si aucun traitement spécial, on garde la ligne telle quelle
    contenuModifie += ligne + "\n";
  }
  file.close();
    // Si le patient a été modifié, on réécrit le fichier avec les nouvelles données
  if (patientModifie) {
    File newFile = SPIFFS.open("/patient.txt", FILE_WRITE);
    if (!newFile) {
      server.send(500, "text/html", "<h3 style='color:red;text-align:center;'>Erreur: Écriture impossible dans le fichier.</h3>");
      return;
    }
    newFile.print(contenuModifie); // Écrit le nouveau contenu
    newFile.close();

    // Réponse HTML de confirmation
    String message = "<html><head><meta charset='UTF-8'>";
    message += "<style>body { font-family: Arial, sans-serif; text-align: center; font-size: 24px; background-color: #f4f4f9; }</style></head><body>";
    message += "<div style='max-width: 600px; margin: 50px auto; padding: 20px; background-color: white; border-radius: 10px; box-shadow: 0 4px 8px rgba(0,0,0,0.1);'>";
    message += "<h3 style='color:green;'>Informations mises à jour avec succès !</h3>";
    message += "<p>Les données du patient ont été modifiées.</p>";
    message += "<a href='/historique'><button style='padding: 10px 20px; background-color: #4CAF50; color: white; border: none; border-radius: 4px; cursor: pointer;'>Voir l'historique</button></a> ";
    message += "<a href='/'><button style='padding: 10px 20px; background-color: #007bff; color: white; border: none; border-radius: 4px; cursor: pointer;'>Retour à l'accueil</button></a>";
    message += "</div></body></html>";

    server.send(200, "text/html; charset=UTF-8", message);
  } else { // Patient non trouvé
    server.send(404, "text/html; charset=UTF-8", "<h3 style='color:red;text-align:center;'>Patient non trouvé. Veuillez vérifier le nom.</h3>");
  }
}

// Fonction pour supprimer un patient par son NOM
void supprimerPatient() {
  // Vérifie si l'argument "nom" est bien passé dans la requête
  if (!server.hasArg("nom")) {  
    server.send(400, "text/html", "<h3 style='color:red;text-align:center;'>Erreur: Nom du patient manquant !</h3><br><a href='/'> Retour</a>");
    return;
  }
  // Récupère et nettoie le nom à rechercher
  String nomASupprimer = server.arg("nom");
  nomASupprimer.trim();

  // Tente d'ouvrir le fichier patient.txt en lecture et envoie message d'erreur si il n'y arrive pas
  File file = SPIFFS.open("/patient.txt", FILE_READ);
  if (!file) {
    server.send(500, "text/html", "<html><head><meta charset='UTF-8'></head><body><h3 style='color:red;text-align:center;'>Erreur: Impossible d'accéder au fichier.</h3><br><a href='/'>⬅ Retour</a></body></html>");
    return;
  } 
  // Variables pour stocker le nouveau contenu du fichier et suivre l’état de la suppression
  String nouveauContenu = "";
  bool patientTrouve = false;
  bool suppressionEnCours = false;

  // Lecture du fichier ligne par ligne pour reconstruire un nouveau contenu sans le patient ciblé
  while (file.available()) {
    String ligne = file.readStringUntil('\n');  
    if (ligne.startsWith("Nom: ") && ligne.indexOf(nomASupprimer) != -1) {
      suppressionEnCours = true;
      patientTrouve = true;
      continue;  
    } 
     // Dès qu’une nouvelle ligne "Nom: " est rencontrée, arrêter la suppression
    if (suppressionEnCours && ligne.startsWith("Nom: ")) {
      suppressionEnCours = false;
    }
    // Si on n’est pas en train de supprimer, ajouter la ligne au nouveau contenu
    if (!suppressionEnCours) {
      nouveauContenu += ligne + "\n";
    }
  } // Ferme le fichier original
  file.close();

  // Réécrit le fichier patient.txt avec le contenu mis à jour
  File newFile = SPIFFS.open("/patient.txt", FILE_WRITE);
  newFile.print(nouveauContenu);
  newFile.close();
  if (patientTrouve) { // Envoie une réponse selon que le patient a été trouvé et supprimé ou non
    server.send(200, "text/html", "<html><head><meta charset='UTF-8'></head><body><h3 style='color:green;text-align:center;'> Patient supprimé avec succès !</h3><br><a href='/'> Retour</a></body></html>");
  } else {
    server.send(404, "text/html", "<html><head><meta charset='UTF-8'></head><body><h3 style='color:red;text-align:center;'> Patient non trouvé !</h3><br><a href='/'> Retour</a></body></html>");
  }
}
// Fonction pour imprimer les fichiers
void printPatientData() { 
  File file = SPIFFS.open("/patient.txt", FILE_READ); // Ouvre le fichier en lecture
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
    reponse += "<h2> Fiche Patient</h2>";
    reponse += "<pre>";
    // Lit tout le contenu du fichier
    while (file.available()) {
      reponse += (char)file.read();
    }
    file.close();
 
    reponse += "</pre>";
    reponse += "<button onclick='window.print()'>Imprimer</button>"; 
    reponse += "<br><a href='/'> Retour</a>";

    reponse += "</div></body></html>";

    server.send(200, "text/html; charset=UTF-8", reponse);
  } else { // Envoie le contenu au client
    server.send(500, "text/html", "<html><head><meta charset='UTF-8'></head><body><h3 style='color:red; text-align:center;'>Erreur: Aucunes données d'un patient n'a été trouvées.</h3></body></html>");
  }
}
void supprimerHistorique() {
  // Vérifier que SPIFFS est bien initialisé
  if (!SPIFFS.begin()) {
    Serial.println(" Erreur d'initialisation de SPIFFS !");
    server.send(500, "text/html", "<html><head><meta charset='UTF-8'></head><body><h1>Erreur SPIFFS !</h1><p><a href='/'>Retour</a></p></body></html>");
    return;
  }
  Serial.println(" Formatage de SPIFFS en cours...");
  if (SPIFFS.format()) { // Tente de formater SPIFFS, ce qui efface tous les fichiers
    Serial.println(" SPIFFS formaté avec succès !");
    server.send(200, "text/html", "<html><head><meta charset='UTF-8'></head><body><h1>Historique supprimé avec succès !</h1><p><a href='/'>Retour</a></p></body></html>");
  } else {
    Serial.println(" Échec du formatage de SPIFFS !");
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
  
  WiFi.softAP(ssid, password);
  Serial.println("Tentative de démarrer l'AP...");
  if (!WiFi.softAPConfig(local_IP, gateway, subnet)) {
    Serial.println(" Échec de la configuration de l'IP !");
  } else {
    Serial.println(" Configuration de l'IP réussie !");
  }
  Serial.println(" Point d'accès WiFi activé !");
  Serial.print(" Connectez-vous au réseau : ");
  Serial.println(ssid);
  Serial.print(" Accédez à l'ESP32 via : ");
  Serial.println(WiFi.softAPIP());

  // Route pour affciher la page d'accueil 
  
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", formulairePatient);
});
  server.on("/historique", HTTP_GET, []() {
    if (!estAuthentifie()) return server.requestAuthentication();
      afficherHistorique();
});
  server.on("/supprimer_historique_complet", HTTP_GET, []() {
    if (!estAuthentifie()) return server.requestAuthentication();
      supprimerHistorique();
});
  server.on("/supprimer_patient", HTTP_GET, []() {
    if (!estAuthentifie()) return server.requestAuthentication();
      supprimerPatient();
});
  server.on("/enregistrer", HTTP_POST, []() {
    if (!estAuthentifie()) return server.requestAuthentication();
      enregistrement();
});
  server.on("/modifier_patient", HTTP_ANY, []() {
    if (!estAuthentifie()) return server.requestAuthentication();
      modifierPatient();
});
  server.on("/rechercher_patient", HTTP_GET, []() {
    if (!estAuthentifie()) return server.requestAuthentication();
      server.send(200, "text/html", pageRecherchePatient);
});
  server.on("/conditions", HTTP_GET, afficherConditions);

  server.on("/imprimer", HTTP_GET, []() {
    if (!estAuthentifie()) return server.requestAuthentication();
      printPatientData();
});
  server.begin();
}
void loop() {
  server.handleClient();
  delay(1000);  // Attends 1 seconde avant de réessayer
}    