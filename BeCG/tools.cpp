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

void etalonnage() {

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
  afficheMessage("Remise \x85 zero\ndes balances...");
  #ifdef DEBUG
    Serial.printf("Remise à zero balances...\n");
  #endif
  hx711_ba.set_scale();
  hx711_ba.tare();
  hx711_bf.set_scale();
  hx711_bf.tare();

  // Tarrage bord d'attaque
  sprintf(msgBuffer, "Placez une masse de\n%dg sur la balance\ncot\x82 bord d'attaque,\npuis pressez \"Tare\"", MASSE_ETALON);
  afficheMessage(msgBuffer);
  #ifdef DEBUG
    Serial.printf("Placez une masse de %dg sur la balance coté bord d'attaque, puis appuyez sur \"Tare\"\n", MASSE_ETALON);
  #endif
  waitForTareButton();
  afficheMessage("Mesure de la tare\nbord d'attaque...");
  #ifdef DEBUG
    Serial.printf("Mesure de la tare bord d'attaque...\n");
  #endif
  masse_ba = hx711_ba.get_units(20);
  hx711_ba.set_scale(masse_ba/MASSE_ETALON);

  // Tarrage bord de fuite
  sprintf(msgBuffer, "Placez une masse de\n%dg sur la balance\ncot\x82 bord de fuite,\npuis pressez \"Tare\"", MASSE_ETALON);
  afficheMessage(msgBuffer);
  #ifdef DEBUG
    Serial.printf("Placez une masse de %dg sur la balance coté bord de fuite, puis appuyez sur \"Tare\"\n", MASSE_ETALON);
  #endif
  waitForTareButton();
  afficheMessage("Mesure de la tare\nbord de fuite...");
  #ifdef DEBUG
    Serial.printf("Mesure de la tare bord de fuite...\n");
  #endif
  masse_bf = hx711_bf.get_units(20);
  hx711_bf.set_scale(masse_bf/MASSE_ETALON);

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
String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String((ip >> (8 * 3)) & 0xFF);
  return res;
}
