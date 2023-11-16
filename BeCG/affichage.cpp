/****************************************************************************/
/*                                                                          */
/* Copyright (C) 2023-2023 Gauthier Brière (gauthier.briere "at" gmail.com) */
/*                                                                          */
/* This file: affichage.cpp is part of BeCG                                 */
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

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
int _affichage_OK = 0;


// Initialisation affichage
void affichage_init(void) {

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.printf("Erreur initialisation écran SSD1306 !\n");
  }

  // Active les caractères accentués
  display.cp437(true);

  // Clear the buffer
  display.clearDisplay();

  // On affichera en blanc sur fond noir
  display.setTextColor(SSD1306_WHITE);

  _affichage_OK = 1;

}


void afficheSplash(void) {

  clearDisplay();
  display.drawBitmap(0, 0, BeCG_splash, BeCG_splash_width, BeCG_splash_height, SSD1306_WHITE);
  display.drawBitmap(0, 49, copyright_bitmap, copyright_width, copyright_height, SSD1306_WHITE);
  display.setTextSize(1,2);
  display.setCursor(18,50);
  display.print(COPYRIGHT);
  display.display();

}


// Efface tout l'écran
void clearDisplay(void) {
  if (_affichage_OK) {
    display.clearDisplay();
  }
}


// Efface l'écran et affiche le message passé en argument
void afficheMessage(const char *message) {

  clearDisplay();
  display.setTextSize(1,2);
  display.setCursor(0,0);
  display.print(message);
  display.display();

}


// Ecriture de la première ligne de l'écran avec
// les masses mesurées au bord d'attaque et au
// bord de fuite.
// La ligne maxi fait 21 caractères de long.
void afficheMasse_BA_BF(float masseBA, float MasseBF) {
  char buffVal1[7];
  char buffVal2[7];
  char buffer[22];

  if (_affichage_OK) {
    // Préparation de la chaine à afficher dans un buffer
    dtostrf(masseBA, 6, 1, buffVal1);
    dtostrf(MasseBF, 6, 1, buffVal2);
    sprintf(buffer, "BA%sg - BF%sg", buffVal1, buffVal2);
    // Efface toute la ligne, la police fait 6x8, 
    // donc, un rectangle de 8 pixels de haut 
    // sur toute la largeur de l'écran.
    display.fillRect(0, 0, SCREEN_WIDTH, 8, SSD1306_BLACK);
    // Positionne le curseur en haut à gauche et affiche
    display.setTextSize(1);
    display.setCursor(0,0);
    display.print(buffer);
    display.display();
  }
}


void afficheMasseTotale(float masseTotale) {
  char buffVal[7];
  char buffer[22];

  if (_affichage_OK) {
    // Préparation de la chaine à afficher dans un buffer
    // Stabilisation de l'affichage
    dtostrf(masseTotale, 6, 1, buffVal);
    sprintf(buffer, "%s", buffVal);
    // Efface toute la ligne, la police fait 16 pixels de haut, 
    // donc, un rectangle de 16 pixels de haut sur toute la 
    // largeur de l'écran.
    display.fillRect(0, 12, SCREEN_WIDTH, 16, SSD1306_BLACK);
    // Positionne le curseur
    // La ligne contient :
    // 5 caractères de largeur    6 pixels =  30
    // + 6 caractères de largeur 12 pixels =  72
    // + 2 caractères de largeur  6 pixels =  12
    //                                      -----
    //                                       114
    // Donc, pour la centrer à l'écran, on décale le curseur 
    // de 7 pixels en X ((128-114)/2)
    display.setCursor(7,12);
    // Entête de ligne font double haut simple larg : 6x16
    display.setTextSize(1,2);
    display.print(F("Tot."));
    // Valeur en font 12x16
    display.setTextSize(2);
    display.print(buffer);
    // Fin de ligne en 6x16
    display.setTextSize(1,2);
    display.print(F(" g"));
    // Go sur écran !
    display.display();
  }

}


void afficheCG(float positionCG) {
  char buffVal[7];
  char buffer[22];

  if (_affichage_OK) {
    // Préparation de la chaine à afficher dans un buffer
    if(positionCG != 0.0) {
      dtostrf(positionCG, 6, 1, buffVal);
      sprintf(buffer, "%s\0", buffVal);
    } else {
      sprintf(buffer, "%s\0", " ~~~~ ");
    }
    // Efface toute la ligne, la police fait 16 pixels de haut, 
    // donc, un rectangle de 16 pixels de haut sur toute la 
    // largeur de l'écran.
    display.fillRect(0, 30, SCREEN_WIDTH, 16, SSD1306_BLACK);
    // Positionne le curseur
    // La ligne contient :
    // 6 caractères de largeur    6 pixels =  36
    // + 6 caractères de largeur 12 pixels =  72
    // + 3 caractères de largeur  6 pixels =  18
    //                                      -----
    //                                       126
    // Donc, pour la centrer à l'écran, on décale le curseur 
    // de 1 pixels en X ((128-126)/2)
    display.setCursor(0,30);
    // Entête de ligne font double haut simple larg : 6x16
    display.setTextSize(1,2);
    display.print(F("CG/BA "));
    // Valeur en font 12x16
    display.setTextSize(2);
    display.print(buffer);
    // Fin de ligne en 6x16
    display.setTextSize(1,2);
    display.print(F(" mm"));
    // Go sur écran !
    display.display();
  }
}


void affichePiedPage() {

  if (_affichage_OK) {
    // Efface les 2 lignes
    display.fillRect(0, 48, SCREEN_WIDTH, 16, SSD1306_BLACK);
    // Affichage à faire plus tard...
    display.setCursor(0,48);
    display.setTextSize(1);
    //                 123456789012345678901
    //display.println(F("SSID = BeCGxxxxxx"));
    display.print("SSID ");
    if (WiFi.status() == WL_CONNECTED) {
      display.println(cli_ssid);
    } else {
      display.println(ap_ssid);
    }
    display.setCursor(0,57);
    //                 123456789012345678901
    //display.println(F("IP =  10. 10. 10. 10"));
    display.print("IP = ");
    if (WiFi.status() == WL_CONNECTED) {
      display.println(WiFi.localIP());
    } else {
      display.println(WiFi.softAPIP());
    }
    // Go sur écran !
    display.display();
  }

}
