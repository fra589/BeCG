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

  #include <EEPROM.h>
  #include <HX711.h>
  #include <ESP8266WiFi.h>
  #include <ESP8266mDNS.h>
  #include <ESP8266WebServer.h>
  #include <DNSServer.h>
  #include <LittleFS.h>

  #include "TrivialKalmanFilter.h"
  #include "bitmaps.h"
  #include "affichage.h"
  #include "tools.h"
  #include "webserver.h"

  // Pour debug sur port série
  #define DEBUG
  #define DEBUG2 // Pour debugs dans la boucle...

  #define ORG_NAME            "fra589\0"
  #define APP_NAME            "BeCG\0"
  #define APP_VERSION_STRING  "v0.0\0"
  #define APP_VERSION_DATE    "20230325\0"
  #define COPYRIGHT           "G.Bri\x8Are 2023-2023\0"

  // Adresses EEProm pour sauvegarde des paramètres
  #define EEPROM_LENGTH 512
  #define ADDR_NAME_VERSION  0 //       16 caractères de long
  #define ADDR_SCALE_BA     16 //   0 + 16 longueur  4 octets
  #define ADDR_SCALE_BF     20 //  16 +  4 longueur  4 octets
  #define ADDR_CLI_SSID     24 //  20 +  4 longueur 32 octets
  #define ADDR_CLI_PWD      56 //  18 + 32 longueur 63 octets
  #define ADDR_AP_SSID     119 //  56 + 63 longueur 32 octets
  #define ADDR_AP_PWD      151 // 119 + 32 longueur 63 octets


  // Données mécanique de la balance
  #define ENTAXE_APPUIS   139.5 // (mm) Distance entre las appuis BA et BF
  #define PORTE_A_FAUX_BA  15.0 // (mm) Distance entre appuis BA et Cale bord attaque

  // Câblages HX711 
  #define LOADCELL_BA_DOUT_PIN 12 // D6/GPIO12
  #define LOADCELL_BA_SCK_PIN  13 // D7/GPIO13
  extern HX711 hx711_ba;
  #define LOADCELL_BF_DOUT_PIN 16 // D0/GPIO16
  #define LOADCELL_BF_SCK_PIN  14 // D5/GPIO14
  extern HX711 hx711_bf;

  // Bouton tarage
  #define PIN_BOUTON        2 // Pin D4/GPIO2 - Bouton de tarage
  #define BOUTON_ON         0
  #define BOUTON_OFF        1
  #define APPUIS_LONG       2 // 2 secondes

  // Valeurs des paramètres par défauts
  #define DEFAULT_CLI_SSID "DomoPassaduy\0"       // SSID client (la minuterie se connecte si défini)
  #define DEFAULT_CLI_PWD  "C'est1secret\0"       // WPA-PSK/WPA2-PSK client
  #define DEFAULT_AP_SSID  "BeCG_\0"  // SSID de l'AP minuterie
  #define DEFAULT_AP_PWD   "\0"       // WPA-PSK/WPA2-PSK AP

  // Variable globales pour le WiFi
  extern char cli_ssid[32];
  extern char cli_pwd[63];
  extern char ap_ssid[32];
  extern char ap_pwd[63];
  // Web server
  extern ESP8266WebServer server;
  // hostname pour mDNS. devrait fonctionner au moins avec windows :
  // http://BeCG.local
  extern const char *myHostname;

  // Fichiers web
  #define ROOT_FILE "/index.html"

  // Masses et Centre de Gravité
  extern float valeurMoy_ba;
  extern float valeurMoy_bf;
  extern float masseTotale;
  extern float positionCG;

#endif // BeCG_h
