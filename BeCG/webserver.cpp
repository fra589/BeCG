/****************************************************************************/
/*                                                                          */
/* Copyright (C) 2023-2024 Gauthier Brière (gauthier.briere "at" gmail.com) */
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

void webServerInit(void) {

  // Montage du système de fichier ou sont stockés les éléments web
  if (!LittleFS.begin()) {
    ;
    #ifdef DEBUG
      Serial.println("Erreur lors du montage du système de fichier LittleFS");
      Serial.flush();
    #endif    
  }

  // Setup web pages
  server.enableCORS(true);
  server.on(ROOT_FILE,           handleRoot); // index.html => /<en/fr>/index.html
  server.on("/",                 handleRoot);
  server.on("/connecttest.txt",  handleRoot);
  server.on("/generate_204",     handleRoot); //Android captive portal. Maybe not needed. Might be handled By notFound handler.
  server.on("/favicon.ico",      handleRoot); //Another Android captive portal. Maybe not needed. Might be handled By notFound handler. Checked on Sony Handy
  server.on("/fwlink",           handleRoot); //Microsoft captive portal. Maybe not needed. Might be handled By notFound handler.
  server.on("/getvalues",        handleGetValues);
  server.on("/getversion",       handleGetVersion);
  server.on("/getwifi",          handleGetWifi);
  server.on("/getnetworks",      handleGetNetworks);
  server.on("/affichage",        handleAffichage);
  server.on("/tare",             handleTare);
  server.on("/resetscale",       handleResetScale);
  server.on("/etalonba",         handleEtalon);
  server.on("/etalonbf",         handleEtalon);
  server.on("/stopmesure",       handleStopMesure);
  server.on("/startmesure",      handleStartMesure);
  server.on("/reboot",           handleReboot);
  server.on("/getsettings",      handleGetSettings);
  server.on("/setsettings",      handleSetSettings);
  server.on("/wificonnect",      handleWifiConnect);
  server.on("/deconnexion",      handleDeconnection);
  server.on("/setlang",          handleSetLang);
  server.on("/resetfactory",     handleFactory);
  server.on("/update",           handleUpdate);
  server.on("/fsinfo",           handleFSInfo);
  server.onNotFound(handleNotFound);
  
  httpUpdater.setup(&server); // Pour mise a jour par le réseau
  server.begin(); // Start web server

  ////MDNS.addService("http", "tcp", 80);
  
  delay(1000);

  #ifdef DEBUG
    Serial.println("Serveur web démarré.");
    Serial.flush();
  #endif    

}

// Redirection vers le portail captif en cas de requette vers un autre domaine
// retourne true dans ce cas pour éviter que la page ne renvoie plusieurs fois
// la requette.
bool captivePortal(void) {

  if (!isIp(server.hostHeader()) && server.hostHeader() != (String(myHostname) + ".local")) {
    #ifdef DEBUG_WEB
      Serial.println("Redirection vers le portail captif");
    #endif
    server.sendHeader("Location", String("http://") + IPtoString(server.client().localIP()) + "/" + String(lang) + "/index.html", true);
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
    Serial.print("server.hostHeader() = ");
    Serial.println(server.hostHeader());
    String uri = ESP8266WebServer::urlDecode(server.uri()); 
    Serial.print("server.uri() = ");
    Serial.println(uri);
  #endif

  if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
    return;
  }

  #ifdef DEBUG_WEB
    Serial.println("captivePortal() returned false, sending index.html");
  #endif

  /*
  handleFileRead("/" + String(lang) + "/index.html");
  */
  // Redirige vers la page index en fonction de la langue
  server.sendHeader("Location", String("http://") + IPtoString(server.client().localIP()) + "/" + String(lang) + "/index.html", true);
  server.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.client().stop(); // Stop is needed because we sent no content length

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
    switch (WiFi.encryptionType(i)) {
      case (ENC_TYPE_NONE):
        XML += "none";
        break;
      case (ENC_TYPE_WEP):
        XML += "WEP";
        break;
      case (ENC_TYPE_TKIP):
        XML += "WPA-PSK";
        break;
      case (ENC_TYPE_CCMP):
        XML += "WPA2-PSK";
        break;
      case (ENC_TYPE_AUTO):
        XML += "Auto";
        break;
      default:
        XML += "Inconnue";
    }
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
  String tmpString = "";
  int i;

  String uri = ESP8266WebServer::urlDecode(server.uri());  // required to read paths with blanks

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
      /* conversion effectuée dans afficheMessage()
      tmpString = server.arg(i);
      tmpString.replace("à", "\x85");
      tmpString.replace("é", "\x82");
      tmpString.replace("è", "\x8A");
      afficheMessage(tmpString.c_str());
      */
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
    #ifdef DEBUG_WEB
      Serial.printf("LittleFS.exists(\"%s\") OK\n", path.c_str());
    #endif
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

  // Cherche le fichier sur LittleFS
  if (handleFileRead(uri)) {
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

  #ifdef DEBUG_WEB
    Serial.printf("Entrée dans handleSetSettings() --- %d\n", millis()/1000);
    Serial.printf("  Nb arguments = %d\n", server.args());
  #endif

  //Traitement des données POST
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

void handleWifiConnect(void) {
  String XML;
  String ssid        = "";
  String password    = "";
  int8_t channel     = 0;
  bool settingChange = false;
  bool newConnectOK  = false;
  String result      = "";
  unsigned long debut = 0;
  
  #ifdef DEBUG_WEB
    Serial.printf("Entrée dans handleWifiConnect() --- %d\n", millis()/1000);
  #endif

  //Traitement des données POST
  if (server.args() > 0) {
    for (int i = 0; i < server.args(); i++) {
      #ifdef DEBUG_WEB
        Serial.printf("  handleWifiConnect() - Arg n°%d –> %s = [%s]\n", i, server.argName(i), server.arg(i).c_str());
      #endif
      if (strncasecmp(server.argName(i).c_str(), "ssid", (size_t)4) == 0) {
        #ifdef DEBUG_WEB
          Serial.printf("  handleWifiConnect() - SSID = %s\n", server.arg(i).c_str());
        #endif
        if (strncmp(cli_ssid, server.arg(i).c_str(), MAX_SSID_LEN) != 0) {
          strncpy(cli_ssid, server.arg(i).c_str(), MAX_SSID_LEN);
          settingChange = true;
        }
      } else if (strncasecmp(server.argName(i).c_str(), "pwd", (size_t)3) == 0) {
        #ifdef DEBUG_WEB
          Serial.printf("  handleWifiConnect() - pwd  = %s\n", server.arg(i).c_str());
        #endif
        if (strncmp(cli_pwd, server.arg(i).c_str(), MAX_PWD_LEN) != 0) {
          strncpy(cli_pwd, server.arg(i).c_str(), MAX_PWD_LEN);
          settingChange = true;
        }
      } else if (strncasecmp(server.argName(i).c_str(), "channel", (size_t)7) == 0) {
        if (server.arg(i).toInt() > 0) {
          channel = server.arg(i).toInt();
        }
      }
    }
    
    // Connetion au réseau
    if (WiFi.status() == WL_CONNECTED) {
      // Si on est déjà connecté, on coupe...
      #ifdef DEBUG_WEB
        Serial.printf("  handleWifiConnect() - déconnexion du réseau précédent\n");
        Serial.flush();
      #endif
      WiFi.disconnect();
    }
    debut = millis();
    if (channel > 0) {
      #ifdef DEBUG_WEB
        Serial.printf("  handleWifiConnect() - connexion au réseau SSID = [%s] pwd=[%s] channel=%d\n", cli_ssid, cli_pwd, channel);
        Serial.flush();
      #endif
      WiFi.begin(cli_ssid, cli_pwd, channel);
    } else {
      #ifdef DEBUG_WEB
        Serial.printf("  handleWifiConnect() - connexion au réseau SSID = [%s] pwd=[%s]\n", cli_ssid, cli_pwd);
        Serial.flush();
      #endif
      WiFi.begin(cli_ssid, cli_pwd);
    }
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      #ifdef DEBUG_WEB
        Serial.print(".");
        Serial.flush();
      #endif
      if (millis() - debut > 10000) {
        break; // Timeout = 10 secondes
      }
    }
    switch (WiFi.status()) {
      case WL_CONNECTED:
        // Connexion OK
        result = "OK";
        newConnectOK = true;
        #ifdef DEBUG_WEB
            Serial.println(result);
            Serial.printf("IP = %s\n",IPtoString(WiFi.localIP()).c_str());
        #endif
        WiFi.setAutoReconnect(true);
        //Start mDNS with APP_NAME
        if (MDNS.begin(myHostname, WiFi.localIP())) {
          ;
          #ifdef DEBUG_WEB
            Serial.println("MDNS started");
          #endif
        } else {
          ;
          #ifdef DEBUG_WEB
            Serial.println("MDNS failed");
          #endif
        }
        break;
      case WL_IDLE_STATUS:
        result = "Erreur : Wi-Fi is in process of changing between statuses";
        newConnectOK = false;
        #ifdef DEBUG_WEB
          Serial.println(result);
        #endif
      break;
      case WL_NO_SSID_AVAIL:
        result = "Erreur : SSID cannot be reached";
        newConnectOK = false;
        #ifdef DEBUG_WEB
          Serial.println(result);
        #endif
      break;
      case WL_CONNECT_FAILED:
        result = "Erreur : Connexion failed";
        newConnectOK = false;
        #ifdef DEBUG_WEB
          Serial.println(result);
        #endif
      break;
      case WL_WRONG_PASSWORD:
        result = "Erreur : Password is incorrect";
        newConnectOK = false;
        #ifdef DEBUG_WEB
          Serial.println(result);
        #endif
      break;
      case WL_DISCONNECTED:
        result = "Erreur : Module is not configured in station mode";
        newConnectOK = false;
        #ifdef DEBUG_WEB
          Serial.println(result);
        #endif
      break;
    }
    
    if (newConnectOK){
      #ifdef DEBUG_WEB
        Serial.printf("Sauvegarde paramètres WiFi en EEPROM : SSID=[%s], pwd=[%s]\n", cli_ssid, cli_pwd);
      #endif
      // Sauvegarde les nouveaux paramètres dans l'EEPROM
      EEPROM_writeStr(ADDR_CLI_SSID, cli_ssid, MAX_SSID_LEN);
      EEPROM_writeStr(ADDR_CLI_PWD, cli_pwd, MAX_PWD_LEN);
      EEPROM.commit();
    }

    // Réponse au client
    XML  = F("<?xml version=\"1.0\" encoding=\"UTF-8\"\?>\n");
    //XML +=F("<?xml-stylesheet href=\"style.css\" type=\"text/css\"?>\n");
    XML += F("<wificonnect>\n");
    XML += F("  <result>") + result + F("</result>\n");
    XML += F("</wificonnect>\n");
  } else {
    // Réponse au client
    XML  = F("<?xml version=\"1.0\" encoding=\"UTF-8\"\?>\n");
    //XML +=F("<?xml-stylesheet href=\"style.css\" type=\"text/css\"?>\n");
    XML += F("<wificonnect>\n");
    XML += F("  <result>wificonnect: Manque de paramètres</result>\n");
    XML += F("</wificonnect>\n");
  }
  
  // Renvoi la réponse au client http
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200,"text/xml",XML);

  #ifdef DEBUG_WEB
    Serial.println("Réponse XML envoyée.");
  #endif

}

void handleDeconnection(void) {
  String XML;
  int i;
  
  #ifdef DEBUG_WEB
    Serial.printf("Entrée dans handleDeconnection() --- %d\n", millis()/1000);
  #endif

  // Deconnexion du réseau
  if (WiFi.status() == WL_CONNECTED) {
    // Si on est déjà connecté, on coupe, sinoni il n'y a rien a faire...
    #ifdef DEBUG_WEB
      Serial.printf("  handleWifiConnect() - déconnexion du réseau précédent\n");
      Serial.flush();
    #endif
    WiFi.disconnect();
  }

  //Effacement des données réseau
  for (i=0; i<MAX_SSID_LEN; i++) {
    cli_ssid[i] = '\xFF';
  }
  for (i=0; i<MAX_PWD_LEN; i++) {
    cli_pwd[i] = '\xFF';
  }
  // Sauvegarde les nouveaux paramètres dans l'EEPROM
  EEPROM_writeStr(ADDR_CLI_SSID, cli_ssid, MAX_SSID_LEN);
  EEPROM_writeStr(ADDR_CLI_PWD, cli_pwd, MAX_PWD_LEN);
  EEPROM.commit();
  
  // Réponse au client
  XML  = F("<?xml version=\"1.0\" encoding=\"UTF-8\"\?>\n");
  //XML +=F("<?xml-stylesheet href=\"style.css\" type=\"text/css\"?>\n");
  XML += F("<deconnexion>\n");
  XML += F("  <result>OK</result>\n");
  XML += F("</deconnexion>\n");

  // Renvoi la réponse au client http
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200,"text/xml",XML);

}

void handleSetLang(void) {
  // Met à jour et mémorise la langue choisie dans la flash

  bool settingChange = false;
  
  String XML;

  #ifdef DEBUG_WEB
    Serial.printf("Entrée dans handleSetLang() --- %d\n", millis()/1000);
    Serial.printf("  Nb arguments = %d\n", server.args());
  #endif

  //Traitement des données POST
  for (int i = 0; i < server.args(); i++) {
    #ifdef DEBUG_WEB
      Serial.printf("  handleSetLang() - Arg n°%d –> %s = [%s]\n", i, server.argName(i), server.arg(i).c_str());
    #endif
    if (strncasecmp(server.argName(i).c_str(), "lang", (size_t)4) == 0) {
      if ((server.arg(i) == "fr") || (server.arg(i) == "en")) {
        if (strncmp(server.arg(i).c_str(), lang, (size_t)LANG_LEN) != 0) {
          // changement de langue
          #ifdef DEBUG_WEB
            Serial.printf("  Activation de la langue : %s\n", server.arg(i).c_str());
          #endif
          strncpy(lang, server.arg(i).c_str(), LANG_LEN);
          settingChange = true;
        }
      }
    }
  }

  if (settingChange) {
    // Sauvegarde dans l'EEPROM
    EEPROM_writeStr(ADDR_LANG, lang, LANG_LEN);
    EEPROM.commit();
  }

  // Redirige vers la page index en fonction de la langue
  server.sendHeader("Location", String("http://") + IPtoString(server.client().localIP()) + "/" + String(lang) + "/index.html", true);
  server.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
  server.client().stop(); // Stop is needed because we sent no content length

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

void handleUpdate(void) {
  #ifdef DEBUG_WEB
    Serial.println("Entrée dans handleUpdate()");
  #endif
  handleFileRead("/update.html");
}

void handleFSInfo(void) {
  String XML;
  FSInfo fs_info;
  
  #ifdef DEBUG_WEB
    Serial.printf("Entrée dans handleGetAPconfig()\n");
  #endif

  LittleFS.info(fs_info);

  // Renvoi la réponse au client http
  XML  = F("<?xml version=\"1.0\" encoding=\"UTF-8\"\?>\n");
  XML += F("<FSInfo>\n");
  XML += F("  <totalBytes>");
  XML += String(fs_info.totalBytes);
  XML += F("</totalBytes>\n");
  XML += F("  <usedBytes>");
  XML += String(fs_info.usedBytes);
  XML += F("</usedBytes>\n");
  XML += F("  <blockSize>");
  XML += String(fs_info.blockSize);
  XML += F("</blockSize>\n");
  XML += F("  <pageSize>");
  XML += String(fs_info.pageSize);
  XML += F("</pageSize>\n");
  XML += F("  <maxOpenFiles>");
  XML += String(fs_info.maxOpenFiles);
  XML += F("</maxOpenFiles>\n");
  XML += F("  <maxPathLength>");
  XML += String(fs_info.maxPathLength);
  XML += F("</maxPathLength>\n");
  // Liste les fichiers
  XML += F("  <Files>\n");
  XML += getFileList("/");
  XML += F("  </Files>\n");
  XML += F("</FSInfo>\n");

  server.send(200,"text/xml",XML);

}

String getFileList(String path) {
  // Fonction recursive listage des fichiers
  String liste = "";
  Dir dir;
  File f;

  #ifdef DEBUG_WEB
    Serial.print("getFileList(\"");
    Serial.print(path);
    Serial.println("\")");
  #endif
  
  dir = LittleFS.openDir(path);
  while(dir.next()){
    #ifdef DEBUG_WEB
      Serial.println(dir.fileName());
    #endif
    liste += F("    <File><name>");
    liste += String(path + dir.fileName());
    liste += F("</name><size>");
    liste += String(dir.fileSize());
    liste += F("</size></File>\n");
    if (dir.isDirectory() == true) {
      liste += getFileList(path + dir.fileName() + "/");
    }
    yield();
  }

  return liste;
  
}
