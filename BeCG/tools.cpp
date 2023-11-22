/****************************************************************************/
/*                                                                          */
/* Copyright (C) 2023-2023 Gauthier Brière (gauthier.briere "at" gmail.com) */
/*                                                                          */
/* This file: tools.h is part of BeCG                                       */
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
    Serial.printf("Remise à zero balances...\n");
  #endif

  if (hx711_ba.wait_ready_timeout(1000)) {
    afficheMessage("\n   Remise \x85 zero\n   balance BA...");
    #ifdef DEBUG
      Serial.printf("Remise à zero balance BA...\n");
    #endif
    hx711_ba.tare(15); // Remise à zero de balance bord d'attaque
  }
  if (hx711_ba.wait_ready_timeout(1000)) {
    afficheMessage("\n   Remise \x85 zero\n   balance BF...");
    #ifdef DEBUG
      Serial.printf("Remise à zero balance BF...\n");
    #endif
    hx711_bf.tare(15); // Remise à zero de balance bord de fuite
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
  afficheMessage("Remise \x85 z\x82ro\ndes balances...");
  #ifdef DEBUG
    Serial.printf("Remise à zero balances...\n");
  #endif
  hx711_ba.set_scale();
  hx711_ba.tare();
  hx711_bf.set_scale();
  hx711_bf.tare();

  // Tarrage bord d'attaque
  sprintf(msgBuffer, "Placez une masse de\n%dg sur la balance\ncot\x82 bord d'attaque,\npuis pressez \"Tare\"", masseEtalon);
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
  sprintf(msgBuffer, "Placez une masse de\n%dg sur la balance\ncot\x82 bord de fuite,\npuis pressez \"Tare\"", masseEtalon);
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

  sprintf(msgBuffer, "Etalonnage termin\x82,\nscale_ba = %f\nscale_bf = %f", scale_ba, scale_bf);
  afficheMessage(msgBuffer);
  #ifdef DEBUG
    Serial.printf("Etalonnage terminé, scale_ba = %f, scale_bf = %f\n", scale_ba, scale_bf);
  #endif

  delay(5000);
  clearDisplay();
  affichePiedPage();

}

  void resetFactory(void) {
  // Reset de tous les paramètres à leur valeur par défaut

  #ifdef DEBUG
    Serial.printf("Entrée dans resetFactory()\n");
  #endif

  scaleBA = DEFAULT_SCALE_BA;
  scaleBF = DEFAULT_SCALE_BF;
  strcpy(cli_ssid, DEFAULT_CLI_SSID);
  strcpy(cli_pwd,  DEFAULT_CLI_PWD);
  strcpy(ap_ssid,  DEFAULT_AP_SSID);
  //strcpy(ap_ssid,  DEFAULT_AP_SSID);
  String SSID_MAC = String(DEFAULT_AP_SSID + WiFi.softAPmacAddress().substring(9));
  SSID_MAC.toCharArray(ap_ssid, MAX_SSID_LEN);
  entraxe     = DEFAULT_ENTAXE;
  pafBA       = DEFAULT_PAF_BA;
  masseEtalon = DEFAULT_MASSE_ETALON;

  // Sauvegarde en EEPROM
  EEPROM_format(); // On efface tout
  
  EEPROM_writeStr(ADDR_NAME_VERSION, nameVersion, NAME_VERSION_LEN);
  
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

// Formate une chaine de caractère utf8 et caractères 
// d'échappement pour affichage sur SSD1306 en CP437
String formateCP437(String utf8) {
  String tmpString = "";
  String tmpChar = "";
  int i;
  int len;
  
  len = utf8.length();
  for (i=0; i<len; i++) {
    tmpChar = utf8[i];
    if (tmpChar == "à") {
      tmpString += '\x85';
    } else if (tmpChar == "é") {
      tmpString += '\x82';
    } else if (tmpChar == "ê") {
      tmpString += '\x88';
    } else if (tmpChar == "ë") {
      tmpString += '\x89';
    } else if (tmpChar == "è") {
      tmpString += '\x8A';
    } else if (tmpChar == "ç") {
      tmpString += '\x87';
    } else if (tmpChar == "\\") {
      i++;
      tmpChar = utf8[i];
      if (tmpChar == "n") {
        tmpString += '\n';
      } else if (tmpChar == "t") {
        tmpString += '\t';
      } else if (tmpChar == "\\") {
        tmpString += '\\';
      }
    } else {
      tmpString += tmpChar;
    }
  }
  
  return tmpString;
  
}
