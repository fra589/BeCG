/****************************************************************************/
/*                                                                          */
/* Copyright (C) 2023-2023 Gauthier Brière (gauthier.briere "at" gmail.com) */
/*                                                                          */
/* This file: webserver.cpp is part of BeCG                                 */
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
#include <stdlib.h>

// Redirection vers le portail captif en cas de requette vers un autre domaine
// retourne true dans ce cas pour éviter que la page ne renvoie plusieurs fois
// la requette.
bool captivePortal(void) {
  if (!isIp(server.hostHeader()) && server.hostHeader() != (String(myHostname) + ".local")) {
    #ifdef DEBUG_WEB
      Serial.println("Redirection vers le portail captif");
    #endif
    server.sendHeader("Location", String("http://") + IPtoString(server.client().localIP()), true);
    server.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
    server.client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}

//Handles http request 
void handleRoot(void) {
  String buffer = "";
  File file;

  #ifdef DEBUG_WEB
    Serial.println("Entrée dans handleRoot()");
  #endif

  handleFileRead("/index.html");
}

void handleGetValues(void) {
  float valeur = 0.0;
  String XML;

  #ifdef DEBUG_WEB_VALUE
    Serial.printf("Entrée dans handleGetValues() --- %d\n", millis()/1000);
  #endif

  // Renvoi la réponse au client http
  XML  = F("<?xml version=\"1.0\" encoding=\"UTF-8\"\?>\n");
  //XML += F("<?xml-stylesheet href=\"style.css\" type=\"text/css\"?>\n");
  XML += F("<valeurs>\n");
  XML += F("  <ba>");
  XML += String(valeurMoy_ba);
  XML += F("  </ba>\n");
  XML += F("  <bf>");
  XML += String(valeurMoy_bf);
  XML += F("  </bf>\n");
  XML += F("  <total>");
  XML += String(masseTotale);
  XML += F("  </total>\n");
  XML += F("  <cg>");
  if (positionCG != 0.0) {
    XML += String(positionCG);
  } else {
    XML += F("~~~~");
  }
  XML += F("  </cg>\n");
  XML += F("</valeurs>\n");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200,"text/xml",XML);

}

void handleGetVersion(void) {
  String XML;

  #ifdef DEBUG_WEB
    Serial.printf("Entrée dans handleGetVersion() --- %d\n", millis()/1000);
  #endif

  // Renvoi la réponse au client http
  XML  = F("<?xml version=\"1.0\" encoding=\"UTF-8\"\?>\n");
  //XML += F("<?xml-stylesheet href=\"style.css\" type=\"text/css\"?>\n");
  XML += F("<version>\n");
  XML += F("  <app>");
  XML += String(APP_NAME);
  XML += F("  </app>\n");
  XML += F("  <major>");
  XML += String(APP_VERSION_MAJOR);
  XML += F("  </major>\n");
  XML += F("  <minor>");
  XML += String(APP_VERSION_MINOR);
  XML += F("  </minor>\n");
  XML += F("  <date>");
  XML += String(APP_VERSION_DATE);
  XML += F("  </date>\n");
  XML += F("  <string>");
  XML += String(APP_VERSION_STRING);
  XML += F("  </string>\n");
  XML += F("</version>\n");

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200,"text/xml",XML);

}

void handleGetWifi(void) {
  String XML;

  #ifdef DEBUG_WEB
    Serial.printf("Entrée dans handleGetWifi() --- %d\n", millis()/1000);
  #endif

  // Renvoi la réponse au client http
  XML  =F("<?xml version=\"1.0\" encoding=\"UTF-8\"\?>\n");
  //XML +=F("<?xml-stylesheet href=\"style.css\" type=\"text/css\"?>\n");
  XML +=F("<wifi>\n");
  XML +=F("  <ap_ssid>");
  XML += String(ap_ssid);
  XML +=F("  </ap_ssid>");
  XML +=F("  <ap_ip>");
  XML += IPtoString(WiFi.softAPIP());
  XML +=F("  </ap_ip>");
  XML +=F("  <cli_ssid>");
  if (WiFi.status() == WL_CONNECTED) {
    XML += String(cli_ssid);
  } else {
    XML += F("cli non connecté");
  }
  XML +=F("  </cli_ssid>");
  XML +=F("  <cli_ip>");
  if (WiFi.status() == WL_CONNECTED) {
    XML += IPtoString(WiFi.localIP());
  } else {
    XML += F("No local IP");
  }
  XML +=F("  </cli_ip>");
  XML +=F("</wifi>\n");

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200,"text/xml",XML);

}

void handleTare(void) {
  String XML;

  #ifdef DEBUG_WEB
    Serial.printf("Entrée dans handleTare() --- %d\n", millis()/1000);
  #endif

  tare_en_cours = true;

  tare();
  
  // On attends que ça soit fini pour renvoyer au client web
  while (tare_en_cours) {
    #ifdef DEBUG_WEB
      Serial.printf("Attente tarage en cours --- %d\n", millis()/1000);
    #endif
    delay(500);
    // https://forum.arduino.cc/t/yield-utilite-fonctionnement/883558
    yield();
  }
  
  #ifdef DEBUG_WEB
    Serial.printf("handleTare() Tarage terminé --- %d\n", millis()/1000);
  #endif

  // Renvoi la réponse au client http
  XML  = F("<?xml version=\"1.0\" encoding=\"UTF-8\"\?>\n");
  //XML +=F("<?xml-stylesheet href=\"style.css\" type=\"text/css\"?>\n");
  XML += F("<tare>\n");
  XML += F("  <result>OK</result>\n");
  XML += F("</tare>\n");

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200,"text/xml",XML);

}

void handleResetScale(void) {
  // Preparation de l'étalonnage (reset des échelles des balances)
  String XML;
  
  #ifdef DEBUG_WEB
    Serial.printf("Entrée dans handleResetScale() --- %d\n", millis()/1000);
  #endif
  
  reset_scale_en_cours = true;
  resetScale();

  // On attends que ça soit fini pour renvoyer au client web
  while (reset_scale_en_cours) {
    #ifdef DEBUG_WEB
      Serial.printf("Attente reset scale en cours --- %d\n", millis()/1000);
    #endif
    delay(500);
    // https://forum.arduino.cc/t/yield-utilite-fonctionnement/883558
    yield();
  }
  
  #ifdef DEBUG_WEB
    Serial.printf("handleResetScale() reset scale terminé --- %d\n", millis()/1000);
  #endif

  // Renvoi la réponse au client http
  XML  = F("<?xml version=\"1.0\" encoding=\"UTF-8\"\?>\n");
  //XML +=F("<?xml-stylesheet href=\"style.css\" type=\"text/css\"?>\n");
  XML += F("<resetscale>\n");
  XML += F("  <result>OK</result>\n");
  XML += F("</resetscale>\n");

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200,"text/xml",XML);

}

void handleEtalon(void) {
  String XML;
  float scale = 0.0;
  
  #ifdef DEBUG_WEB
    Serial.printf("Entrée dans handleEtalon(%s) --- %d\n", server.uri(), millis()/1000);
  #endif

  etalonnage_en_cours = true;
  
  scale = etalon(server.uri());
  
  while (etalonnage_en_cours) {
    #ifdef DEBUG_WEB
      Serial.printf("Attente etalonnage en cours --- %d\n", millis()/1000);
    #endif
    delay(500);
    // https://forum.arduino.cc/t/yield-utilite-fonctionnement/883558
    yield();
  }
  
  #ifdef DEBUG_WEB
    Serial.printf("handleEtalon() etalonnage terminé --- %d\n", millis()/1000);
  #endif

  // Renvoi la réponse au client http
  XML  = F("<?xml version=\"1.0\" encoding=\"UTF-8\"\?>\n");
  //XML +=F("<?xml-stylesheet href=\"style.css\" type=\"text/css\"?>\n");
  XML += F("<etalonnage>\n");
  XML += F("  <scale>");
  XML += String(scale);
  XML += F("</scale>\n");
  XML += F("  <result>OK</result>\n");
  XML += F("</etalonnage>\n");

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200,"text/xml",XML);

}

// renvoie en format XML la liste des SSID scannés 
String getWifiNetworks() {
  String XML = "";
  int nReseaux;
  int i;

  #ifdef DEBUG_WEB
    Serial.printf("Entrée dans getWifiNetworks()\n");
  #endif

// Voir https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/scan-examples.html#scan
//WiFi.mode(WIFI_STA);
//WiFi.disconnect();
// https://randomnerdtutorials.com/esp8266-nodemcu-wi-fi-manager-asyncwebserver/

  nReseaux = WiFi.scanNetworks();
  for (i = 0; i < nReseaux; i++) {
    XML += "  <network>\n";
    XML += "    <SSID>";
    XML += WiFi.SSID(i);
    XML += "</SSID>\n";
    XML += "    <channel>";
    XML += WiFi.channel(i);
    XML += "</channel>\n";
    XML += "    <RSSI>";
    XML += WiFi.RSSI(i);
    XML += "</RSSI>\n";
    XML += "    <encryption>";
    XML += WiFi.encryptionType(i);
    XML += "</encryption>\n";
    XML += "  </network>\n";
  }
  WiFi.scanDelete();

  return XML;

}

void handleGetNetworks(void) {
  String XML;

  #ifdef DEBUG_WEB
    Serial.printf("Entrée dans handleGetNetworks() --- %d\n", millis()/1000);
  #endif

  // Renvoi la réponse au client http
  XML  =F("<?xml version=\"1.0\" encoding=\"UTF-8\"\?>\n");
  //XML +=F("<?xml-stylesheet href=\"style.css\" type=\"text/css\"?>\n");
  XML += "<networks>\n";
  XML += getWifiNetworks();
  XML += "</networks>\n";

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200,"text/xml",XML);
  
}

void handleAffichage(void) {
  String XML;
  int i;

  String uri = ESP8266WebServer::urlDecode(server.uri());  // required to read paths with blanks

// A voir ici pour conversion UTF8 -> GFX Latin 1 (CP437)
// https://www.sigmdel.ca/michel/program/misc/gfxfont_8bit_fr.html


  #ifdef DEBUG_WEB
    Serial.printf("Entrée dans handleAffichage() --- %d\n", millis()/1000);
    Serial.printf("uri encodée = [%s]\n", server.uri());
    Serial.printf("uri décodée = [%s]\n", uri);
    Serial.printf("  Nb arguments URI = %d\n", server.args());
  #endif

  for (int i = 0; i < server.args(); i++) {
    #ifdef DEBUG_WEB
      Serial.printf("  handleAffichage() - Arg n°%d –> %s = [%s]\n", i, server.argName(i), server.arg(i).c_str());
    #endif
    if (strncasecmp(server.argName(i).c_str(), "text", (size_t)4) == 0) {
      #ifdef DEBUG_WEB
        Serial.printf("  handleAffichage() - envoi du texte sur l'écran\n");
      #endif
      afficheMessage(server.arg(i).c_str());
    } else if (strncasecmp(server.argName(i).c_str(), "reset", (size_t)5) == 0) {
      #ifdef DEBUG_WEB
        Serial.printf("  handleAffichage() - Reset écran\n");
      #endif
      clearDisplay();
      affichePiedPage();
    }
  }

  // Renvoi la réponse au client http
  XML  = F("<?xml version=\"1.0\" encoding=\"UTF-8\"\?>\n");
  //XML +=F("<?xml-stylesheet href=\"style.css\" type=\"text/css\"?>\n");
  XML += F("<affichage>\n");
  XML += F("  <result>OK</result>\n");
  XML += F("</affichage>\n");

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200,"text/xml",XML);

}

void handleStopMesure(void) {
  String XML;

  #ifdef DEBUG_WEB
    Serial.printf("Entrée dans handleStopMesure() --- %d\n", millis()/1000);
  #endif

  disableBouton=true;
  disableMesure = true;

  // Renvoi la réponse au client http
  XML  = F("<?xml version=\"1.0\" encoding=\"UTF-8\"\?>\n");
  //XML +=F("<?xml-stylesheet href=\"style.css\" type=\"text/css\"?>\n");
  XML += F("<stopmesure>\n");
  XML += F("  <result>OK</result>\n");
  XML += F("</stopmesure>\n");

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200,"text/xml",XML);

}

void handleStartMesure(void) {
  String XML;

  #ifdef DEBUG_WEB
    Serial.printf("Entrée dans handleStartMesure() --- %d\n", millis()/1000);
  #endif

  disableBouton=false;
  disableMesure = false;

  // Renvoi la réponse au client http
  XML  = F("<?xml version=\"1.0\" encoding=\"UTF-8\"\?>\n");
  //XML +=F("<?xml-stylesheet href=\"style.css\" type=\"text/css\"?>\n");
  XML += F("<startmesure>\n");
  XML += F("  <result>OK</result>\n");
  XML += F("</startmesure>\n");

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200,"text/xml",XML);

}

bool handleFileRead(String path) {
  String contentType;
  File file;

  #ifdef DEBUG_WEB
    Serial.printf("Entrée dans handleFileRead(\"%s\")\n", path.c_str());
  #endif

  contentType = mime::getContentType(path);

  if (LittleFS.exists(path)) {
    file = LittleFS.open(path, "r");
    if (server.streamFile(file, contentType) != file.size()) {
      ;
      #ifdef DEBUG_WEB
      Serial.println("Sent less data than expected!");
      #endif
    }
    file.close();
    return true;
  }

  return false;

}

void handleNotFound(void) {

  #ifdef DEBUG_WEB
    Serial.print("Entrée dans handleNotFound() ");
  #endif

  String uri = ESP8266WebServer::urlDecode(server.uri());  // required to read paths with blanks

  #ifdef DEBUG_WEB
    Serial.printf("- uri = %s\n", uri.c_str());
  #endif

  // 
  if (handleFileRead(uri)) {
    return;
  }

  // If captive portal redirect instead of displaying the error page.
  if (captivePortal()) {
    return;
  }

  String message = F("Page non trouvée : ");
  message += server.uri().c_str();
  message += F("\n");

  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(404, "text/plain", message);

}

void handleReboot(void) {
  String XML;

  #ifdef DEBUG_WEB
    Serial.printf("Entrée dans handleReboot() --- %d\n", millis()/1000);
  #endif

  // Retour à la page principale après traitements
  server.sendHeader("Location", "/", true);
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(302, "text/plain", "");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.client().stop(); // Stop is needed because we sent no content length

  // Et on reboot
  ESP.restart();
  
}

void handleGetSettings(void) {
  String XML;
  
  #ifdef DEBUG_WEB
    Serial.printf("Entrée dans handleGetSettings() --- %d\n", millis()/1000);
  #endif

  // Renvoi la réponse au client http
  XML  = F("<?xml version=\"1.0\" encoding=\"UTF-8\"\?>\n");
  //XML +=F("<?xml-stylesheet href=\"style.css\" type=\"text/css\"?>\n");
  XML += F("<settings>\n");
  XML += F("  <paf_ba>");
  XML += String(pafBA);
  XML += F("</paf_ba>\n");
  XML += F("  <entraxe>");
  XML += String(entraxe);
  XML += F("</entraxe>\n");
  XML += F("  <tare>");
  XML += String(masseEtalon);
  XML += F("</tare>\n");
  XML += F("</settings>\n");

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200,"text/xml",XML);

}

void handleSetSettings(void) {
  char *endPtr;
  float fvalue = 0.0;
  int16_t ivalue = 0;
  bool settingChange = false;
  
  String XML;

  String uri = ESP8266WebServer::urlDecode(server.uri());  // required to read paths with blanks

  #ifdef DEBUG_WEB
    Serial.printf("Entrée dans handleSetSettings() --- %d\n", millis()/1000);
    Serial.printf("uri encodée = [%s]\n", server.uri());
    Serial.printf("uri décodée = [%s]\n", uri);
    Serial.printf("  Nb arguments URI = %d\n", server.args());
  #endif

  //Traitement des données
  for (int i = 0; i < server.args(); i++) {
    #ifdef DEBUG_WEB
      Serial.printf("  handleSetSettings() - Arg n°%d –> %s = [%s]\n", i, server.argName(i), server.arg(i).c_str());
    #endif
    if (strncasecmp(server.argName(i).c_str(), "paf_ba", (size_t)6) == 0) {
      fvalue = strtof(server.arg(i).c_str(), &endPtr);
      #ifdef DEBUG_WEB
        Serial.printf("  handleSetSettings() - mise à jour porte à faux BA = %f\n", fvalue);
      #endif
      if (fvalue != pafBA) {
        pafBA = fvalue;
        settingChange = true;
      }
    } else if (strncasecmp(server.argName(i).c_str(), "entraxe", (size_t)7) == 0) {
      fvalue = strtof(server.arg(i).c_str(), &endPtr);
      #ifdef DEBUG_WEB
        Serial.printf("  handleSetSettings() - mise à jour porte à faux BA = %f\n", fvalue);
      #endif
      if (fvalue != entraxe) {
        entraxe = fvalue;
        settingChange = true;
      }
    } else if (strncasecmp(server.argName(i).c_str(), "tare", (size_t)4) == 0) {
      ivalue = strtol(server.arg(i).c_str(), &endPtr, 10);
      #ifdef DEBUG_WEB
        Serial.printf("  handleSetSettings() - mise à jour masse étalon = %d\n", ivalue);
      #endif
      if (ivalue != masseEtalon) {
        masseEtalon = ivalue;
        settingChange = true;
      }
    }
  }

  if (settingChange) {
    // Sauvegarde dans l'EEPROM
    EEPROM.put(ADDR_ENTRAXE, entraxe);
    EEPROM.put(ADDR_PAF_BA, pafBA);
    EEPROM.put(ADDR_MASSE_ETALON, masseEtalon);
    EEPROM.commit();
  }

  // Renvoi la réponse au client http
  XML  = F("<?xml version=\"1.0\" encoding=\"UTF-8\"\?>\n");
  //XML +=F("<?xml-stylesheet href=\"style.css\" type=\"text/css\"?>\n");
  XML += F("<settings>\n");
  XML += F("  <paf_ba>");
  XML += String(pafBA);
  XML += F("</paf_ba>\n");
  XML += F("  <entraxe>");
  XML += String(entraxe);
  XML += F("</entraxe>\n");
  XML += F("  <tare>");
  XML += String(masseEtalon);
  XML += F("</tare>\n");
  XML += F("</settings>\n");

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200,"text/xml",XML);

}

void handleFactory(void) {
  String XML;
  
  #ifdef DEBUG_WEB
    Serial.printf("Entrée dans handleFactory() --- %d\n", millis()/1000);
  #endif
  
  resetFactory();

  // Renvoi la réponse au client http
  XML  = F("<?xml version=\"1.0\" encoding=\"UTF-8\"\?>\n");
  //XML +=F("<?xml-stylesheet href=\"style.css\" type=\"text/css\"?>\n");
  XML += F("<factory>\n");

  XML += F("  <version>\n");
  XML += F("    <string>") + String(APP_VERSION_STRING) + F("</string>\n");
  XML += F("  </version>\n");
  
  XML += F("  <settings>\n");
  XML += F("    <paf_ba>") + String(pafBA) + F("</paf_ba>\n");
  XML += F("    <entraxe>") + String(entraxe) + F("</entraxe>\n");
  XML += F("    <tare>") + String(masseEtalon) + F("</tare>\n");
  XML += F("  </settings>\n");

  XML += F("  <wifi>\n");
  XML += F("    <ap_ssid>") + String(ap_ssid) + F("</ap_ssid>");
  XML += F("    <ap_pwd>") + String(ap_pwd) + F("</ap_pwd>");
  XML += F("    <ap_ip>") + IPtoString(WiFi.softAPIP()) + F("</ap_ip>");
  XML += F("    <cli_ssid>") + String(cli_ssid) + F("</cli_ssid>");
  XML += F("    <cli_pwd>") + String(cli_pwd) + F("</cli_pwd>");
  XML += F("  </wifi>\n");
  XML += F("</factory>\n");

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200,"text/xml",XML);

}
