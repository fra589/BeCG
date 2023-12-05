/****************************************************************************/
/*                                                                          */
/* Copyright (C) 2023-2023 Gauthier Brière (gauthier.briere "at" gmail.com) */
/*                                                                          */
/* This file: BeCG.h is part of BeCG                                        */
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

#ifndef BeCG_h
  #define BeCG_h

  #include <stdlib.h>
  #include <Adafruit_SSD1306.h>
  #include <EEPROM.h>
  #include <HX711.h>
  #include <ESP8266WiFi.h>
  #include <ESP8266mDNS.h>
  #include <ESP8266WebServer.h>
  #include <DNSServer.h>
  #include <LittleFS.h>
  #include <ESP8266HTTPUpdateServer.h>

  #include "TrivialKalmanFilter.h"
  #include "bitmaps.h"
  #include "affichage.h"
  #include "tools.h"
  #include "webserver.h"
  #include "becgwifi.h"

  // Pour debug sur port série
  //#define DEBUG
  //#define DEBUG2          // Pour debugs dans la boucle...
  //#define DEBUG_WEB       // debug des interractions web
  //#define DEBUG_WEB_VALUE // debug des appels de valeurs web

  #define COPYRIGHT             "G.Bri\x8Are 2023-2023"
  #define ORG_NAME              "fra589"
  #define APP_NAME              "BeCG"
  #define APP_VERSION_MAJOR     "0"
  #define APP_VERSION_MINOR     "9"
  #define APP_VERSION_DATE      "20231201"
  #define APP_VERSION_STRING    "v" APP_VERSION_MAJOR "." APP_VERSION_MINOR "." APP_VERSION_DATE
  #define APP_NAME_VERSION      APP_NAME " - " APP_VERSION_STRING "\0"
  #define EEPROM_VERSION_MAJOR  "0"
  #define EEPROM_VERSION_MINOR  "9"
  #define EEPROM_VERSION_REVIS  "0"
  #define EEPROM_VERSION        APP_NAME " EEPROM v" EEPROM_VERSION_MAJOR "." EEPROM_VERSION_MINOR "." EEPROM_VERSION_REVIS

  #define EEPROM_VERSION_LEN 32
  extern char EEPROM_Version[EEPROM_VERSION_LEN];

  // Adresses EEProm pour sauvegarde des paramètres
  #define EEPROM_LENGTH 512
  #define ADDR_EEPROM_VERSION  0 // =      32 caractères de long
  #define ADDR_SCALE_BA       32 // =   0 + 32 longueur  4 octets
  #define ADDR_SCALE_BF       36 // =  32 +  4 longueur  4 octets
  #define ADDR_CLI_SSID       40 // =  36 +  4 longueur 32 octets
  #define ADDR_CLI_PWD        56 // =  40 + 32 longueur 63 octets
  #define ADDR_AP_SSID       119 // =  56 + 63 longueur 32 octets
  #define ADDR_AP_PWD        151 // = 119 + 32 longueur 63 octets
  #define ADDR_AP_CHANNEL    214 // = 151 + 63 longueur  2 octets
  #define ADDR_ENTRAXE       216 // = 214 +  2 longueur  4 octets
  #define ADDR_PAF_BA        220 // = 216 +  4 longueur  4 octets
  #define ADDR_MASSE_ETALON  224 // = 220 +  4 longueur  2 octets

  // Données mécanique de la balance
  #define DEFAULT_ENTAXE          125.0 // (mm) Distance entre las appuis BA et BF
  #define DEFAULT_PAF_BA           15.0 // (mm) Distance entre appuis BA et Cale bord attaque
  #define DEFAULT_MASSE_ETALON    500   // (g) Masse utilisée pour l'étalonnage
  extern float entraxe;
  extern float pafBA;
  extern int16_t masseEtalon;

  // Câblages HX711 
  #define LOADCELL_BA_DOUT_PIN 12 // D6/GPIO12
  #define LOADCELL_BA_SCK_PIN  13 // D7/GPIO13
  extern HX711 hx711_ba;
  #define LOADCELL_BF_DOUT_PIN 16 // D0/GPIO16
  #define LOADCELL_BF_SCK_PIN  14 // D5/GPIO14
  extern HX711 hx711_bf;

  // Echelle par défaut des balances 
  #define DEFAULT_SCALE_BA 385.0
  #define DEFAULT_SCALE_BF 385.0
  extern float scaleBA;
  extern float scaleBF;

  // Bouton tarage
  #define PIN_BOUTON        2 // Pin D4/GPIO2 - Bouton de tarage
  #define BOUTON_ON         0
  #define BOUTON_OFF        1
  #define APPUIS_LONG       2 // 2 secondes

  // Valeurs des paramètres par défauts
  #define DEFAULT_CLI_SSID   ""       // SSID client (la balance se connecte si défini)
  #define DEFAULT_CLI_PWD    ""       // WPA-PSK/WPA2-PSK client
  #define DEFAULT_AP_SSID    "BeCG_"  // SSID de l'AP balance
  #define DEFAULT_AP_PWD     ""       // WPA-PSK/WPA2-PSK AP
  #define DEFAULT_AP_CHANNEL 3

  // Variable globales pour le WiFi
  #define MAX_SSID_LEN 32 // Longueur maxi d'un SSID
  #define MAX_PWD_LEN  63 // Longueur maxi des mots de passe WiFi
  extern char cli_ssid[MAX_SSID_LEN];
  extern char cli_pwd[MAX_PWD_LEN];
  extern char ap_ssid[MAX_SSID_LEN];
  extern char ap_pwd[MAX_PWD_LEN];
  
  // Web server
  extern ESP8266WebServer server;
  extern ESP8266HTTPUpdateServer httpUpdater;
  
  // DNS server
  extern DNSServer dnsServer;
  
  // hostname pour mDNS. devrait fonctionner au moins avec windows :
  // http://BeCG.local
  extern const char *myHostname;

  // Fichiers web
  #define ROOT_FILE "/index.html"

  // Masses et Centre de Gravité
  extern TrivialKalmanFilter<float> filter_ba;
  extern TrivialKalmanFilter<float> filter_bf;
  extern float valeurMoy_ba;
  extern float valeurMoy_bf;
  extern float masseTotale;
  extern float positionCG;

  // Flags pour désactiver le bouton tare et les mesures
  extern bool disableBouton;
  extern bool disableMesure;
  
#endif // BeCG_h
