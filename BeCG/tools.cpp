/****************************************************************************/
/*                                                                          */
/* Copyright (C) 2023-2024 Gauthier Brière (gauthier.briere "at" gmail.com) */
/*                                                                          */
/* This file: tools.cpp is part of BeCG                                     */
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

void getEepromStartupData(void) {
  char charTmp;
  char buffEepromVersion[EEPROM_VERSION_LEN] = { 0 };

  // Prépare la chaine de version qui permet de vérifier les données de l'EEPROM
  strcpy(EEPROM_Version, EEPROM_VERSION);

  //--------------------------------------------------------------------
  // Récupération des paramètres dans l'EEPROM ou de leur valeur par défaut
  //--------------------------------------------------------------------
  EEPROM.begin(EEPROM_LENGTH);
  charTmp = char(EEPROM.read(ADDR_EEPROM_VERSION));
  if (charTmp != 0xFF) {
    buffEepromVersion[0] = charTmp;
    for (int i=1; i<EEPROM_VERSION_LEN; i++) {
      buffEepromVersion[i] = char(EEPROM.read(ADDR_EEPROM_VERSION + i));
    }
    if (strncmp(EEPROM_Version, buffEepromVersion, EEPROM_VERSION_LEN) != 0) {
      // Les données sauvegardées dans l'EEPROM ne correspondent pas à 
      // la version en cours, il faut recharger les paramètres par défaut
      #ifdef DEBUG
        Serial.printf("Version incorrecte des données EEPROM [%s] != [%s]\n", buffEepromVersion, EEPROM_Version);
      #endif
      resetFactory();
    }
  } else {
    // L'EEPROM est vide !
    #ifdef DEBUG
      Serial.printf("l'EEPROM est vide, chargement des données d'usine.\n");
    #endif
    resetFactory();
  }
  
  EEPROM.get(ADDR_SCALE_BA, scaleBA);  if (scaleBA != scaleBA) scaleBA = DEFAULT_SCALE_BA;
  EEPROM.get(ADDR_SCALE_BF, scaleBF);  if (scaleBF != scaleBF) scaleBF = DEFAULT_SCALE_BF;
  charTmp = char(EEPROM.read(ADDR_CLI_SSID));
  if (charTmp != 0xFF) {
    cli_ssid[0] = charTmp;
    for (int i=1; i<MAX_SSID_LEN; i++) {
      cli_ssid[i] = char(EEPROM.read(ADDR_CLI_SSID + i));
    }
  }
  charTmp = char(EEPROM.read(ADDR_CLI_PWD));
  if (charTmp != 0xFF) {
    cli_pwd[0] = charTmp;
    for (int i=1; i<MAX_PWD_LEN; i++) {
      cli_pwd[i] = char(EEPROM.read(ADDR_CLI_PWD + i));
    }
  }
  charTmp = char(EEPROM.read(ADDR_AP_SSID));
  if (charTmp != 0xFF) {
    ap_ssid[0] = charTmp;
    for (int i=1; i<MAX_SSID_LEN; i++) {
      ap_ssid[i] = char(EEPROM.read(ADDR_AP_SSID + i));
    }
  } else {
    String SSID_MAC = String(DEFAULT_AP_SSID + WiFi.softAPmacAddress().substring(9));
    SSID_MAC.toCharArray(ap_ssid, MAX_SSID_LEN);
  }
  charTmp = char(EEPROM.read(ADDR_AP_PWD));
  if (charTmp != 0xFF) {
    ap_pwd[0] = charTmp;
    for (int i=1; i<MAX_PWD_LEN; i++) {
      ap_pwd[i] = char(EEPROM.read(ADDR_AP_PWD + i));
    }
  }
  EEPROM.get(ADDR_ENTRAXE, entraxe);  if (entraxe != entraxe) entraxe = DEFAULT_ENTAXE;
  EEPROM.get(ADDR_PAF_BA, pafBA);  if (pafBA != pafBA) pafBA = DEFAULT_PAF_BA;
  EEPROM.get(ADDR_MASSE_ETALON, masseEtalon);  if (masseEtalon != masseEtalon) masseEtalon = DEFAULT_MASSE_ETALON;
  charTmp = char(EEPROM.read(ADDR_LANG));
  if (charTmp != 0xFF) {
    lang[0] = charTmp;
    for (int i=1; i<LANG_LEN; i++) {
      lang[i] = char(EEPROM.read(ADDR_LANG + i));
    }
  } else {
    strcpy(lang, DEFAULT_LANG);
  }

  #ifdef DEBUG
    Serial.printf("EEPROM_Version......... = %s\n", EEPROM_Version);
    Serial.printf("LANG................... = %s\n", lang);
    Serial.printf("scaleBA................ = %f\n", scaleBA);
    Serial.printf("scaleBF................ = %f\n", scaleBF);
    Serial.printf("pafBA.................. = %f\n", pafBA);
    Serial.printf("entraxe................ = %f\n", entraxe);
    Serial.printf("masseEtalon............ = %d\n", masseEtalon);
    Serial.printf("cli_ssid............... = %s\n", cli_ssid);
    Serial.printf("cli_pwd................ = %s\n", cli_pwd);
    Serial.printf("ap_ssid................ = %s\n", ap_ssid);
    Serial.printf("ap_pwd................. = %s\n\n", ap_pwd);
  #endif

}

void balancesInit(void) {
  // Init des balances
  #ifdef DEBUG
    Serial.println("HX711 begin...");
  #endif
  hx711_ba.begin(LOADCELL_BA_DOUT_PIN, LOADCELL_BA_SCK_PIN, 64);
  hx711_bf.begin(LOADCELL_BF_DOUT_PIN, LOADCELL_BF_SCK_PIN, 64);
  yield();
  #ifdef DEBUG
    Serial.println("HX711 begin OK.");
  #endif

  if ((hx711_ba.wait_ready_timeout(1000)) && (hx711_bf.wait_ready_timeout(1000))) {
    yield();
    #ifdef DEBUG
      Serial.println("HX711 ready no timeout...");
    #endif
    // Initialise la tare (zéro) et l'échelle des balance
    hx711_ba.set_scale();
    hx711_ba.tare();
    hx711_bf.set_scale();
    hx711_bf.tare();
    delay(250);
    yield();
    #ifdef DEBUG
      Serial.println("HX711 set_scale() + tare() OK...");
    #endif

    // Etalonnage des Balance :
    if (scaleBA != 0.0) {
      hx711_ba.set_scale(scaleBA);
    } else {
      hx711_ba.set_scale(DEFAULT_SCALE_BA);
    }
    hx711_ba.tare(10); // Remise à zero de balance bord d'attaque
    if (scaleBF != 0.0) {
      hx711_bf.set_scale(scaleBF);
    } else {
      hx711_bf.set_scale(DEFAULT_SCALE_BF);
    }
    hx711_bf.tare(10); // Remise à zéro de balance bord de fuite
    delay(250);
    yield();
    #ifdef DEBUG
      Serial.println("HX711 prets.");
    #endif
  } else {
    disableMesure = true;
    #ifdef DEBUG
      Serial.println("Pas de réponse du (des) module(s) HX711, boucle de mesure désactivée.");
    #endif
  }

}

void waitForTareButton() {
  // Attente bouton appuyé puis relâché
  while (digitalRead(PIN_BOUTON) == BOUTON_OFF) {
    delay(1);
  }
  while (digitalRead(PIN_BOUTON) == BOUTON_ON) {
    delay(1);
  }
}

void tare(void) {
  // Tarage (remise à zéro) de la balance

  #ifdef DEBUG
    Serial.printf("Remise à zéro balances...\n");
  #endif

  if (hx711_ba.wait_ready_timeout(1000)) {
    afficheMessage("\n   Remise à zéro\n   balance BA...");
    #ifdef DEBUG
      Serial.printf("Remise à zero balance BA...\n");
    #endif
    hx711_ba.tare(15); // Remise à zero de balance bord d'attaque
  }
  if (hx711_ba.wait_ready_timeout(1000)) {
    afficheMessage("\n   Remise à zéro\n   balance BF...");
    #ifdef DEBUG
      Serial.printf("Remise à zéro balance BF...\n");
    #endif
    hx711_bf.tare(15); // Remise à zéro de balance bord de fuite
  }

  filter_ba.reset(); // Reset filtre de Kalman BA
  filter_bf.reset(); // Reset filtre de Kalman BF

  delay(1500);
  // Restore l'affichage normal de l'écran
  clearDisplay();
  affichePiedPage();

  // On prévient le service web
  tare_en_cours= false;
  
}

void resetScale(void) {
  // Réinitialise l'échelle des balances avant nouvel étalonnage
  #ifdef DEBUG
    Serial.printf("Remise à zero de l'échelle des balances...\n");
  #endif
  hx711_ba.set_scale();
  hx711_bf.set_scale();
  
  // On prévient le service web
  reset_scale_en_cours = false;
}

float etalon(String uri) {
  // Etalonnage BA ou BF selon uri
  float masse_ba = 0.0;
  float masse_bf = 0.0;
  float scale = 0.0;

  if (uri == "/etalonba") {
    afficheMessage("Mesure de la tare\nbord d'attaque...");
    masse_ba = hx711_ba.get_units(15);
    hx711_ba.set_scale(masse_ba/masseEtalon);
    scale = hx711_ba.get_scale();
  } else if (uri == "/etalonbf") {
    afficheMessage("Mesure de la tare\nbord de fuite...");
    masse_bf = hx711_bf.get_units(15);
    hx711_bf.set_scale(masse_bf/masseEtalon);
    scale = hx711_bf.get_scale();
  }
  
  // On prévient le service web que c'est fini
  etalonnage_en_cours = false;

  delay(100);
  clearDisplay();
  affichePiedPage();

  return scale;
}

void etalonnage(void) {
  // Etalonnage de la balance par rappore à une masse connue
  char msgBuffer[169];
  float masse_ba = 0.0;
  float masse_bf = 0.0;
  float scale_ba = 0.0;
  float scale_bf = 0.0;

  afficheMessage("Assurez vous que la\nbalance est vide puis\nappuyez sur \"Tare\"");
  #ifdef DEBUG
    Serial.printf("Assurez vous que la balance est vide, puis appuyez sur \"Tare\"\n");
  #endif
  waitForTareButton();
  
  // Reset valeurs balances
  afficheMessage("Remise à zéro\ndes balances...");
  #ifdef DEBUG
    Serial.printf("Remise à zero balances...\n");
  #endif
  hx711_ba.set_scale();
  hx711_ba.tare();
  hx711_bf.set_scale();
  hx711_bf.tare();

  // Tarrage bord d'attaque
  sprintf(msgBuffer, "Placez une masse de\n%dg sur la balance\ncoté bord d'attaque,\npuis pressez \"Tare\"", masseEtalon);
  afficheMessage(msgBuffer);
  #ifdef DEBUG
    Serial.printf("Placez une masse de %dg sur la balance coté bord d'attaque, puis appuyez sur \"Tare\"\n", masseEtalon);
  #endif
  waitForTareButton();
  afficheMessage("Mesure de la tare\nbord d'attaque...");
  #ifdef DEBUG
    Serial.printf("Mesure de la tare bord d'attaque...\n");
  #endif
  masse_ba = hx711_ba.get_units(15);
  hx711_ba.set_scale(masse_ba/masseEtalon);

  // Tarrage bord de fuite
  sprintf(msgBuffer, "Placez une masse de\n%dg sur la balance\ncoté bord de fuite,\npuis pressez \"Tare\"", masseEtalon);
  afficheMessage(msgBuffer);
  #ifdef DEBUG
    Serial.printf("Placez une masse de %dg sur la balance coté bord de fuite, puis appuyez sur \"Tare\"\n", masseEtalon);
  #endif
  waitForTareButton();
  afficheMessage("Mesure de la tare\nbord de fuite...");
  #ifdef DEBUG
    Serial.printf("Mesure de la tare bord de fuite...\n");
  #endif
  masse_bf = hx711_bf.get_units(15);
  hx711_bf.set_scale(masse_bf/masseEtalon);

  scale_ba = hx711_ba.get_scale();
  scale_bf = hx711_bf.get_scale();

  // Sauvegarde dans l'EEPROM
  EEPROM.put(ADDR_SCALE_BA, scale_ba);
  EEPROM.put(ADDR_SCALE_BF, scale_bf);
  EEPROM.commit();

  sprintf(msgBuffer, "Etalonnage terminé,\nscale_ba = %f\nscale_bf = %f", scale_ba, scale_bf);
  afficheMessage(msgBuffer);
  #ifdef DEBUG
    Serial.printf("Etalonnage terminé, scale_ba = %f, scale_bf = %f\n", scale_ba, scale_bf);
  #endif

  delay(5000);
  clearDisplay();
  affichePiedPage();

}

  void resetFactory(void) {
  // Reset de tous les paramètres à leur valeur par défaut et reinitialisation EEPROM

  #ifdef DEBUG
    Serial.printf("Entrée dans resetFactory()\n");
  #endif

  scaleBA = DEFAULT_SCALE_BA;
  scaleBF = DEFAULT_SCALE_BF;
  strcpy(cli_ssid, DEFAULT_CLI_SSID);
  strcpy(cli_pwd,  DEFAULT_CLI_PWD);
  strcpy(ap_ssid,  DEFAULT_AP_SSID);
  strcpy(ap_pwd,   DEFAULT_AP_PWD);
  strcpy(lang, DEFAULT_LANG);
  String SSID_MAC = String(DEFAULT_AP_SSID + WiFi.softAPmacAddress().substring(9));
  SSID_MAC.toCharArray(ap_ssid, MAX_SSID_LEN);
  entraxe     = DEFAULT_ENTAXE;
  pafBA       = DEFAULT_PAF_BA;
  masseEtalon = DEFAULT_MASSE_ETALON;

  // Sauvegarde en EEPROM
  EEPROM_format(); // On efface tout
  
  EEPROM_writeStr(ADDR_EEPROM_VERSION, EEPROM_Version, EEPROM_VERSION_LEN);
  
  #ifdef DEBUG
    Serial.printf("  EEPROM.put(%d, %f)\n", ADDR_SCALE_BA, scaleBA);
    Serial.printf("  EEPROM.put(%d, %f)\n", ADDR_SCALE_BF, scaleBF);
  #endif
  EEPROM.put(ADDR_SCALE_BA, scaleBA);
  EEPROM.put(ADDR_SCALE_BF, scaleBF);

  EEPROM_writeStr(ADDR_CLI_SSID, cli_ssid, MAX_SSID_LEN);
  EEPROM_writeStr(ADDR_CLI_PWD, cli_pwd, MAX_PWD_LEN);
  EEPROM_writeStr(ADDR_AP_SSID, ap_ssid, MAX_SSID_LEN);
  EEPROM_writeStr(ADDR_AP_PWD, ap_pwd, MAX_PWD_LEN);

  #ifdef DEBUG
    Serial.printf("  EEPROM.put(%d, %f)\n", ADDR_ENTRAXE, entraxe);
    Serial.printf("  EEPROM.put(%d, %f)\n", ADDR_PAF_BA, pafBA);
    Serial.printf("  EEPROM.put(%d, %d)\n", ADDR_MASSE_ETALON, masseEtalon);
  #endif
  EEPROM.put(ADDR_ENTRAXE, entraxe);
  EEPROM.put(ADDR_PAF_BA, pafBA);
  EEPROM.put(ADDR_MASSE_ETALON, masseEtalon);

  EEPROM_writeStr(ADDR_LANG, lang, LANG_LEN);

  #ifdef DEBUG
    Serial.printf("  EEPROM.commit()\n");
  #endif
  EEPROM.commit();
  
}

void EEPROM_writeStr(int address, char *value, int len) {
  // Write string char to eeprom
  #ifdef DEBUG
    Serial.printf("  EEPROM_writeStr(%d, \"%s\", %d)\n", address, value, len);
  #endif
  for (int i=0; i<len; i++) {
    EEPROM.write(address + i, value[i]);
  }  
}

void EEPROM_format(void) {
  // Efface tout le contenu de l'EEPROM
  #ifdef DEBUG
    Serial.printf("  EEPROM_format()\n");
  #endif
  for ( int i = 0 ; i < EEPROM_LENGTH ; i++ )
    EEPROM.write(i, 0xFF);
   EEPROM.commit();
}

// Is this an IP?
bool isIp(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

// IP to String.
String IPtoString(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String((ip >> (8 * 3)) & 0xFF);
  return res;
}

String macToString(const unsigned char* mac) {
  char buf[20];
  snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}
