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


// Redirection vers le portail captif en cas de requette vers un autre domaine
// retourne true dans ce cas pour éviter que la page ne renvoie plusieurs fois
// la requette.
bool captivePortal(void) {
  if (!isIp(server.hostHeader()) && server.hostHeader() != (String(myHostname) + ".local")) {
    #ifdef DEBUG
      Serial.println("Redirection vers le portail captif");
    #endif
    server.sendHeader("Location", String("http://") + toStringIp(server.client().localIP()), true);
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

  #ifdef DEBUG
    Serial.println("Entrée dans handleRoot()");
  #endif

  handleFileRead("/index.html");
/*
  file = LittleFS.open(ROOT_FILE, "r");
  if (!file) {
    #ifdef DEBUG
      Serial.print("Erreur d'ouverture du fichier : ");
      Serial.println(ROOT_FILE);
    #endif
    return;
  } else {
    ;
    #ifdef DEBUG
      Serial.print("Fichier ");
      Serial.print(ROOT_FILE);
      Serial.print(" ouvert, taille = ");
      Serial.println(file.size());
    #endif
  }
  // Lecture du fichier
  while(file.available()) {
    buffer += file.readString();
  }
  server.send(200, "text/html", buffer);
*/
}


void handleGetValues(void) {
  float valeur = 0.0;
  String XML;

  #ifdef DEBUG
    Serial.println("Entrée dans handleGetValues()");
  #endif

  // Renvoi la réponse au client http
  XML =F("<?xml version='1.0'?>\n");
  XML+=F("<valeurs>\n");
  XML+=F("  <ba>");
  XML+=String(valeurMoy_ba);
  XML+=F("  </ba>\n");
  XML+=F("  <bf>");
  XML+=String(valeurMoy_bf);
  XML+=F("  </bf>\n");
  XML+=F("  <total>");
  XML+=String(masseTotale);
  XML+=F("  </total>\n");
  XML+=F("  <cg>");
  XML+=String(positionCG);
  XML+=F("  </cg>\n");
  XML+=F("</valeurs>\n");
  server.send(200,"text/xml",XML);

}


bool handleFileRead(String path) {
  String contentType;
  File file;

  #ifdef DEBUG
    Serial.println("Entrée dans handleFileRead()");
  #endif

  contentType = mime::getContentType(path);

  if (LittleFS.exists(path)) {
    file = LittleFS.open(path, "r");
    if (server.streamFile(file, contentType) != file.size()) {
      ;
      #ifdef DEBUG
      Serial.println("Sent less data than expected!");
      #endif
    }
    file.close();
    return true;
  }

  return false;

}

void handleNotFound(void) {

  #ifdef DEBUG
    Serial.print("Entrée dans handleNotFound() ");
  #endif

  String uri = ESP8266WebServer::urlDecode(server.uri());  // required to read paths with blanks

  #ifdef DEBUG
    Serial.printf("- uri = %s\n", uri);
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
  message += server.uri();
  message += F("\n");

  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(404, "text/plain", message);

}


