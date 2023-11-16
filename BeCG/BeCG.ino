/****************************************************************************/
/*                                                                          */
/* Copyright (C) 2023-2023 Gauthier Brière (gauthier.briere "at" gmail.com) */
/*                                                                          */
/* This file: BeCG.ino is part of BeCG                                      */
/*                                                                          */
/* BeCG is free software: you can redistribute it and/or modify it          */
/* under the terms of the GNU General Public License as published by        */
/* the Free Software Foundation, either version 3 of the License, or        */
/* (at your option) any later version.                                      */
/*                                                                          */
/* BeCG is distributed in the hope that it will be useful, but              */
/* WITHOUT ANY WARRANTY; without even the implied warranty of               */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            */
/* GNU General Public License for more details.                             */
/*                                                                          */
/* You should have received a copy of the GNU General Public License        */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>.    */
/*                                                                          */
/****************************************************************************/

#include "BeCG.h"


//----------------------------------------------------------------------------
// Variables globales
//----------------------------------------------------------------------------
char nameVersion[16];
// Capteurs de masse
HX711 hx711_ba;
HX711 hx711_bf;
// WiFi
char cli_ssid[32] = DEFAULT_CLI_SSID;
char cli_pwd[63]  = DEFAULT_CLI_PWD;
char ap_ssid[32]  = DEFAULT_AP_SSID;
char ap_pwd[63]   = DEFAULT_AP_PWD;
// mDNS
const char *myHostname = APP_NAME;
// DNS server
const byte DNS_PORT = 53;
DNSServer dnsServer;
// Web server
ESP8266WebServer server(80);
// Soft AP network parameters
IPAddress apIP(10, 10, 10, 10);
IPAddress netMsk(255, 255, 255, 0);
// Masses et Centre de Gravité
float valeurMoy_ba = 0.0;
float valeurMoy_bf = 0.0;
float masseTotale = 0.0;
float positionCG = 0.0;


//#define DT_COVARIANCE_RK 4.7e-3 // Estimation of the noise covariances (process)
//#define DT_COVARIANCE_QK 1e-5   // Estimation of the noise covariances (observation)
//#define DT_COVARIANCE_RK 0.00468921411002 // Estimation of the noise covariances (process)
//#define DT_COVARIANCE_QK 0.00468921411002   // Estimation of the noise covariances (observation)
#define DT_COVARIANCE_RK 0.002   // Estimation of the noise covariances (process)
#define DT_COVARIANCE_QK 0.00025 // Estimation of the noise covariances (observation)
TrivialKalmanFilter<float> filter_ba(DT_COVARIANCE_RK, DT_COVARIANCE_QK);
TrivialKalmanFilter<float> filter_bf(DT_COVARIANCE_RK, DT_COVARIANCE_QK);

void setup() {
  float scaleBA;
  float scaleBF;
  char charTmp;
  unsigned long debut;

  #if defined(DEBUG) || defined(DEBUG2)
    // Init port série pour debug
    Serial.begin(115200);
    delay(500);
    Serial.println("");
  #endif

  // Hardware init
  pinMode(PIN_BOUTON, INPUT_PULLUP);

  // Init affichage
  affichage_init();
  afficheSplash();

  // Récupération des paramètres dans la Flash ou de leur valeur par défaut
  EEPROM.begin(EEPROM_LENGTH);
  charTmp = char(EEPROM.read(ADDR_NAME_VERSION));
  if (charTmp != 0xFF) {
    nameVersion[0] = charTmp;
    for (int i=1; i<16; i++) {
      nameVersion[i] = char(EEPROM.read(ADDR_NAME_VERSION + i));
    }
  }
  EEPROM.get(ADDR_SCALE_BA, scaleBA);  if (scaleBA != scaleBA) scaleBA = 0.0;
  EEPROM.get(ADDR_SCALE_BF, scaleBF);  if (scaleBF != scaleBF) scaleBF = 0.0;
  charTmp = char(EEPROM.read(ADDR_CLI_SSID));
  if (charTmp != 0xFF) {
    cli_ssid[0] = charTmp;
    for (int i=1; i<32; i++) {
      cli_ssid[i] = char(EEPROM.read(ADDR_CLI_SSID + i));
    }
  }
  charTmp = char(EEPROM.read(ADDR_CLI_PWD));
  if (charTmp != 0xFF) {
    cli_pwd[0] = charTmp;
    for (int i=1; i<63; i++) {
      cli_pwd[i] = char(EEPROM.read(ADDR_CLI_PWD + i));
    }
  }
  charTmp = char(EEPROM.read(ADDR_AP_SSID));
  if (charTmp != 0xFF) {
    ap_ssid[0] = charTmp;
    for (int i=1; i<32; i++) {
      ap_ssid[i] = char(EEPROM.read(ADDR_AP_SSID + i));
    }
  } else {
    String SSID_MAC = String(DEFAULT_AP_SSID + WiFi.softAPmacAddress().substring(9));
    SSID_MAC.toCharArray(ap_ssid, 32);
  }
  charTmp = char(EEPROM.read(ADDR_AP_PWD));
  if (charTmp != 0xFF) {
    ap_pwd[0] = charTmp;
    for (int i=1; i<63; i++) {
      ap_pwd[i] = char(EEPROM.read(ADDR_AP_PWD + i));
    }
  }
  #ifdef DEBUG
    Serial.print("nameVersion............ = "); Serial.println(nameVersion);
    Serial.print("scaleBA................ = "); Serial.printf("%f\n", scaleBA);
    Serial.print("scaleBF................ = "); Serial.printf("%f\n", scaleBF);
    Serial.print("cli_ssid............... = "); Serial.println(cli_ssid);
    Serial.print("cli_pwd................ = "); Serial.println(cli_pwd);
    Serial.print("ap_ssid................ = "); Serial.println(ap_ssid);
    Serial.print("ap_pwd................. = "); Serial.println(ap_pwd);
  #endif

  // Init des balances
  #ifdef DEBUG
    Serial.println("HX711 begin...");
  #endif
  hx711_ba.begin(LOADCELL_BA_DOUT_PIN, LOADCELL_BA_SCK_PIN, 64);
  hx711_bf.begin(LOADCELL_BF_DOUT_PIN, LOADCELL_BF_SCK_PIN, 64);

  hx711_ba.set_scale();
  hx711_ba.tare();
  hx711_bf.set_scale();
  hx711_bf.tare();

  // Etalonnage des Balance :
  // Valeur des capteurs utilisés pour le dev avec un gain de 128 :
  // Etalonnage terminé, scale_ba = 727.656006, scale_bf = 803.713989
  ////hx711_ba.set_scale(722.898010);
  ////hx711_bf.set_scale(797.702026);
  // avec un gain de 64 :
  // Etalonnage terminé, scale_ba = 363.738007, scale_bf = 401.615997
  //hx711_ba.set_scale(363.738007);
  //hx711_bf.set_scale(401.615997);
  if (scaleBA != 0.0) hx711_ba.set_scale(scaleBA);
  if (scaleBF != 0.0) hx711_bf.set_scale(scaleBF);

  // Connection à un Acces Point si SSID défini
  if (cli_ssid[0] != '\0') {
    #ifdef DEBUG
      Serial.print("Connexion à "); Serial.print(cli_ssid);
      Serial.flush();
    #endif
    debut = millis();
    WiFi.begin(cli_ssid, cli_pwd);
    while (WiFi.status() != WL_CONNECTED) {
      delay(250);
      #ifdef DEBUG
        Serial.print(".");
        Serial.flush();
      #endif
      if (millis() - debut > 10000) {
        break; // Timeout = 10 secondes
      }
    }
    if(WiFi.status() == WL_CONNECTED) {
      #ifdef DEBUG
          Serial.println("OK");
          Serial.print("IP = ");
          Serial.println(WiFi.localIP());
      #endif
      //Start mDNS with APP_NAME
      if (MDNS.begin(myHostname, WiFi.localIP())) {
        ;
        #ifdef DEBUG
          Serial.println("MDNS started");
        #endif
      } else {
        ;
        #ifdef DEBUG
          Serial.println("MDNS failed");
        #endif
      }
    } else {
      ;
      #ifdef DEBUG
        Serial.println("FAIL");
      #endif
    } 
  }

  // Ouverture access point de la balance
  #ifdef DEBUG
    Serial.print("\nConfiguring access point, SSID = ");
    Serial.print(ap_ssid);
    Serial.println("...");
    Serial.flush();
  #endif
  WiFi.softAPConfig(apIP, apIP, netMsk);
  if (ap_pwd[0] == '\0') {
    WiFi.softAP(ap_ssid); // AP ouverte si pas de mot de passe
  } else {
    WiFi.softAP(ap_ssid, ap_pwd);
  }
  delay(500); // Without delay I've seen the IP address blank
  #ifdef DEBUG
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
    Serial.flush();
  #endif

  // Setup DNS server pour redirection de tous les domaines 
  // sur l'IP de la balance
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);

  // Montage du système de fichier ou sont stockés les éléments web
  if (!LittleFS.begin()) {
    ;
    #ifdef DEBUG
      Serial.println("Erreur lors du montage du système de fichier LittleFS");
      Serial.flush();
    #endif    
  }

  // Setup web pages
  server.on("/", handleRoot);
  server.on(ROOT_FILE, handleRoot); // index.html
  server.on("/getvalues", handleGetValues);
  server.onNotFound(handleNotFound);
  server.begin(); // Start web server

  delay(1000);

  // Effacement splash et affichage du pied de page
  clearDisplay();
  affichePiedPage();

}


void loop() {

  unsigned long debut = 0;
  unsigned long now = 0;
  float masse_ba = 0.0;
  float masse_bf = 0.0;

  //DNS
  dnsServer.processNextRequest();
  if (WiFi.status() == WL_CONNECTED) {
    MDNS.update();
  }
  //HTTP
  server.handleClient();

  // Etat du bouton de tarrage
  int etat_bouton = digitalRead(PIN_BOUTON);
  if (etat_bouton == BOUTON_ON) {
    debut = millis();
    do {
      etat_bouton = digitalRead(PIN_BOUTON);
      delay(1);
    } while (etat_bouton != BOUTON_OFF);
    now = millis();
    if ((now - debut) > (APPUIS_LONG * 1000)) {
      #ifdef DEBUG
        Serial.printf("Etalonnages balances...\n");
      #endif
      etalonnage(); // Etalonnages avec une masse connue...
    } else {
      afficheMessage("\n   Remise \x85 zero\n   balances...");
      #ifdef DEBUG
        Serial.printf("Remise à zero balances.\n");
      #endif
      delay(500);
      if (hx711_ba.wait_ready_timeout(1000)) {
        afficheMessage("\n   Remise \x85 zero\n   balance BA...");
        hx711_ba.tare(20); // Remise à zero de balance bord d'attaque
      }
      if (hx711_ba.wait_ready_timeout(1000)) {
        afficheMessage("\n   Remise \x85 zero\n   balance BF...");
        hx711_bf.tare(20); // Remise à zero de balance bord de fuite
      }

      filter_ba.reset(); // Reset filtre de Kalman BA
      filter_bf.reset(); // Reset filtre de Kalman BF

      delay(1500);
      clearDisplay();
      affichePiedPage();
    }
  }

  // Mesure des masses
  if (hx711_ba.wait_ready_timeout(1000) && hx711_bf.wait_ready_timeout(1000)) {
    debut = millis();
    masse_ba = hx711_ba.get_units();
    masse_bf = hx711_bf.get_units();
    now = millis();

    // On applique le filtre de Kalman
    valeurMoy_ba = filter_ba.update(masse_ba);
    valeurMoy_bf = filter_bf.update(masse_bf);

    #ifdef DEBUG2
      /*
      Serial.printf("%0.5f %0.5f\n", masse_ba, valeurMoy_ba);
      Serial.printf("%0.5f %0.5f\n", masse_bf, valeurMoy_bf);
      */
      Serial.printf("HX711 masses BA = %+0.1f,\tBF = %+0.1f,\tfiltrées BA = %+0.1f,\tBF = %+0.1f\n", \
                    masse_ba, \
                    masse_bf, \
                    valeurMoy_ba, \
                    valeurMoy_bf \
      );
    #endif // DEBUG2
  } else {
    afficheMessage("\n   Erreur capteur\n   de masse !");
    #ifdef DEBUG2
      Serial.printf("HX711 missing.\t");
    #endif
  }

  // On force l'arrondi à une décimale de la somme, ce qui évite l'affichage de -0.0
  masseTotale = roundf((valeurMoy_ba + valeurMoy_bf) * 10) / 10;
  // Puis, on arrondi les masse BA et BF après en avoir fait la somme
  valeurMoy_ba = roundf(valeurMoy_ba * 10) /10;
  valeurMoy_bf = roundf(valeurMoy_bf * 10) /10;

  // Affichage :
  afficheMasse_BA_BF(valeurMoy_ba, valeurMoy_bf);
  afficheMasseTotale(masseTotale);
  if ((abs(valeurMoy_ba) < 3) && (abs(valeurMoy_bf) < 3)) {
    // Il faut une masse mini de 3 grammes sur l'un des capteurs
    // pour déclencher le calcul et afficher le centrage. 
    positionCG = 0.0;
  } else {
    positionCG = PORTE_A_FAUX_BA + ((ENTAXE_APPUIS * valeurMoy_bf)/(valeurMoy_ba + valeurMoy_bf));
  }
  afficheCG(positionCG);

}
