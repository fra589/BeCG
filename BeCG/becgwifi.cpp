/****************************************************************************/
/*                                                                          */
/* Copyright (C) 2023-2024 Gauthier Brière (gauthier.briere "at" gmail.com) */
/*                                                                          */
/* This file: becgwifi.cpp is part of BeCG                                  */
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

#ifdef DEBUG
  // Prise en charge des évennements
  WiFiEventHandler stationConnectedHandler;
  WiFiEventHandler stationDisconnectedHandler;
  WiFiEventHandler probeRequestPrintHandler;
  
  void onStationConnected(const WiFiEventSoftAPModeStationConnected& evt) {
    Serial.print("Station connected: ");
    Serial.print(macToString(evt.mac));
    Serial.print(" aid = ");
    Serial.println(evt.aid);
  }

  void onStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& evt) {
    Serial.print("Station disconnected: ");
    Serial.println(macToString(evt.mac));
  }
  
  void onProbeRequestPrint(const WiFiEventSoftAPModeProbeRequestReceived& evt) {
    /*
    Serial.print("Probe request from: ");
    Serial.print(macToString(evt.mac));
    Serial.print(" RSSI: ");
    Serial.println(evt.rssi);
    */
    ;
  }

#endif

void wifiApInit(void) {
  const byte DNS_PORT = 53;

  // Soft AP network parameters
  IPAddress apIP(10, 10, 10, 10);
  IPAddress netMsk(255, 255, 255, 0);

  // Ouverture access point de la balance
  #ifdef DEBUG
    Serial.print("\nConfiguring access point, SSID = ");
    Serial.print(ap_ssid);
    Serial.println("...");
    Serial.flush();
  #endif
  WiFi.softAPConfig(apIP, apIP, netMsk);
  WiFi.softAP(ap_ssid, ap_pwd, DEFAULT_AP_CHANNEL); // (AP ouverte si de mot de passe vide ou null)

  delay(500); // Without delay I've seen the IP address blank
  #ifdef DEBUG
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
    Serial.printf("AP MAC address = %s\n", WiFi.softAPmacAddress().c_str());
    Serial.printf("Wifi channel = %d\n", WiFi.channel());
    Serial.flush();
    // Register event handlers.
    // Callback functions will be called as long as these handler objects exist.
    // Call "onStationConnected" each time a station connects
    stationConnectedHandler = WiFi.onSoftAPModeStationConnected(&onStationConnected);
    // Call "onStationDisconnected" each time a station disconnects
    stationDisconnectedHandler = WiFi.onSoftAPModeStationDisconnected(&onStationDisconnected);
    // Call "onProbeRequestPrint" and "onProbeRequestBlink" each time
    // a probe request is received.
    // Former will print MAC address of the station and RSSI to Serial,
    // latter will blink an LED.
    probeRequestPrintHandler = WiFi.onSoftAPModeProbeRequestReceived(&onProbeRequestPrint);
  #endif

  // Setup DNS server pour redirection de tous les domaines 
  // sur l'IP de la balance
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);

}

void wifiClientInit(void) {
  unsigned long debut;
  
  // Empeche le wifi client de se connecter avec d'anciens paramètres résiduels en flash.
  WiFi.setAutoConnect(false);
  WiFi.setAutoReconnect (false );
  // Connexion à un Acces Point si SSID défini
  if (cli_ssid[0] != '\0') {
    #ifdef DEBUG
      Serial.println("");
      Serial.print("Connexion à "); Serial.print(cli_ssid);
      Serial.flush();
    #endif
    debut = millis();
    WiFi.begin(cli_ssid, cli_pwd);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
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
      WiFi.setAutoReconnect(true);
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
        switch (WiFi.status()) {
          case WL_IDLE_STATUS:
            Serial.println("Erreur : Wi-Fi is in process of changing between statuses");
          break;
          case WL_NO_SSID_AVAIL:
            Serial.println("Erreur : SSID cannot be reached");
          break;
          case WL_CONNECT_FAILED:
            Serial.println("Erreur : Connexion failed");
          break;
          case WL_WRONG_PASSWORD:
            Serial.println("Erreur : Password is incorrect");
          break;
          case WL_DISCONNECTED:
            Serial.println("Erreur : Module is not configured in station mode");
          break;
        }
      #endif
    } 
  }

}
