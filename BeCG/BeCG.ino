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

char nameVersion[NAME_VERSION_LEN];

// Capteurs de masse
HX711 hx711_ba;
HX711 hx711_bf;
// WiFi
char cli_ssid[MAX_SSID_LEN] = DEFAULT_CLI_SSID;
char cli_pwd[MAX_PWD_LEN]   = DEFAULT_CLI_PWD;
char ap_ssid[MAX_SSID_LEN]  = DEFAULT_AP_SSID;
char ap_pwd[MAX_PWD_LEN]    = DEFAULT_AP_PWD;
// mDNS
const char *myHostname = APP_NAME;
// DNS server
const byte DNS_PORT = 53;
DNSServer dnsServer;
// Web server
ESP8266WebServer server(80);
// Masses et Centre de Gravité
float valeurMoy_ba = 0.0;
float valeurMoy_bf = 0.0;
float masseTotale = 0.0;
float positionCG = 0.0;

float entraxe       = DEFAULT_ENTAXE;
float pafBA         = DEFAULT_PAF_BA;
int16_t masseEtalon = DEFAULT_MASSE_ETALON;
float scaleBA       = DEFAULT_SCALE_BA;
float scaleBF       = DEFAULT_SCALE_BF;

// Flag pour attente des services web
bool reset_scale_en_cours = false;
bool tare_en_cours        = false;
bool etalonnage_en_cours  = false;
  // Flags pour désactiver le bouton tare et les mesures
bool disableBouton = false;
bool disableMesure = false;

//#define DT_COVARIANCE_RK 4.7e-3 // Estimation of the noise covariances (process)
//#define DT_COVARIANCE_QK 1e-5   // Estimation of the noise covariances (observation)
//#define DT_COVARIANCE_RK 0.00468921411002 // Estimation of the noise covariances (process)
//#define DT_COVARIANCE_QK 0.00468921411002   // Estimation of the noise covariances (observation)
#define DT_COVARIANCE_RK 0.002   // Estimation of the noise covariances (process)
#define DT_COVARIANCE_QK 0.00025 // Estimation of the noise covariances (observation)
TrivialKalmanFilter<float> filter_ba(DT_COVARIANCE_RK, DT_COVARIANCE_QK);
TrivialKalmanFilter<float> filter_bf(DT_COVARIANCE_RK, DT_COVARIANCE_QK);

void setup() {
  bool update_cpu_freq = false;
  // Try pushing frequency to 160MHz.
  update_cpu_freq = system_update_cpu_freq(SYS_CPU_160MHZ);

  #if defined(DEBUG) || defined(DEBUG2) || defined(DEBUG_WEB) || defined(DEBUG_WEB_VALUE)
    // Init port série pour debug
    Serial.begin(115200);
    delay(500);
    Serial.println("");
    Serial.flush();
  #endif

  #ifdef DEBUG
    int cpuFreq;
    cpuFreq = ESP.getCpuFreqMHz();
    Serial.printf("\nStarting %s on ESP8266@%dMHz (update_cpu_freq = %s)...\n\n", APP_NAME_VERSION, cpuFreq, update_cpu_freq?"true":"false");
    Serial.flush();
  #endif

  // Hardware init
  pinMode(PIN_BOUTON, INPUT_PULLUP);

  // Init affichage
  affichage_init();
  afficheSplash();

  // Récupération des paramètres EEPROM
  getEepromStartupData();
  
  // Initialisation du WiFi
  wifiApInit();
  wifiClientInit();

  // Démarrage du serveur web
  webServerInit();

  // Initialisation des balances
  balancesInit();

  // Effacement splash et affichage du pied de page
  clearDisplay();
  affichePiedPage();

}


void loop() {

  unsigned long debut = 0;
  unsigned long now = 0;
  float masse_ba = 0.0;
  float masse_bf = 0.0;
  int digitAffiche = -1;
  int digit = 0;
  
  //DNS
  dnsServer.processNextRequest();
  if (WiFi.status() == WL_CONNECTED) {
    MDNS.update();
  }
  //HTTP
  server.handleClient();
  if (!disableBouton) {
    // Etat du bouton de tarrage
    int etat_bouton = digitalRead(PIN_BOUTON);
    if (etat_bouton == BOUTON_ON) {
      debut = millis();
      do {
        // Affiche les secondes d'appuis sur l'écran
        if (digitAffiche < 0) {
          digitAffiche = afficheDigit(0);
        } else {
          digit = (int)((millis() - debut)/1000);
          if (digit > 9) {
            // Un appuis de plus de 10 secondes fait rebooter la balance
            ESP.restart();
          }
          // met à jour l'affichage du digit à chaque seconde
          if (digit != digitAffiche) {
            digitAffiche = afficheDigit(digit);
          }
        }
        etat_bouton = digitalRead(PIN_BOUTON);
        delay(1);
        // https://forum.arduino.cc/t/yield-utilite-fonctionnement/883558
        yield();
      } while (etat_bouton != BOUTON_OFF);
      now = millis();
      if ((now - debut) > (APPUIS_LONG * 1000)) {
        #ifdef DEBUG
          Serial.printf("Etalonnages balances...\n");
        #endif
        etalonnage(); // Etalonnages avec une masse connue...
      } else {
        afficheMessage("\n   Remise \x85 z\x82ro\n   balances...");
        #ifdef DEBUG
          Serial.printf("Remise à z\x82ro balances.\n");
        #endif
        tare();
      }
    }
  } // if (!disableBouton)

  if (!disableMesure) {
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
      positionCG = pafBA + ((entraxe * valeurMoy_bf)/(valeurMoy_ba + valeurMoy_bf));
    }
    afficheCG(positionCG);
    
  } // if (!disableMesure)

}
