/****************************************************************************/
/*                                                                          */
/* Copyright (C) 2023-2024 Gauthier Brière (gauthier.briere "at" gmail.com) */
/*                                                                          */
/* This file: translate.h is part of BeCG                                   */
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

#ifndef translate_h
  #define translate_h

  // Les traduction sont définies dans un tableau de caractères : translations[NB_TRANSLATIONS, NB_LANG, MAX_LENGHT]
  // avec pour la deuxième dimention 0 = fr, 1 = en
  // la fonction de traduction recherche la chaine passée en argument (translations[i][0][] et renvoie soit :
  // translations[i][1][] pour le français, ou bien
  // translations[i][2][] pour l'anglais.
  #define NB_TRANSLATIONS 11
  #define NB_LANG 2
  #define MAX_LENGHT 177
  #define FR 1
  #define EN 2
  
  #define TRANSLATIONS {                                                                      \
    {                                                                                         \
      "\n Remise à zéro\n balances...",                                                       \
      "\n Remise à zéro\n balances...",                                                       \
      "\n Reseting\n scales..."                                                               \
    },                                                                                        \
    {                                                                                         \
      "\n Erreur capteur\n de masse !",                                                       \
      "\n Erreur capteur\n de masse !",                                                       \
      "\n Mass sensor\n error!"                                                               \
    },                                                                                        \
    {                                                                                         \
      "\n Remise à zéro\n balance BA...",                                                     \
      "\n Remise à zéro\n balance BA...",                                                     \
      "\n Reseting\n LE scales..."                                                            \
    },                                                                                        \
    {                                                                                         \
      "\n Remise à zéro\n balance BF...",                                                     \
      "\n Remise à zéro\n balance BF...",                                                     \
      "\n Reseting\n TE scales..."                                                            \
    },                                                                                        \
    {                                                                                         \
      "Mesure de la tare\nbord d'attaque...",                                                 \
      "Mesure de la tare\nbord d'attaque...",                                                 \
      "Leading Edge tare\nmeasurement"                                                        \
    },                                                                                        \
    {                                                                                         \
      "Mesure de la tare\nbord de fuite...",                                                  \
      "Mesure de la tare\nbord de fuite...",                                                  \
      "Trailing Edge tare\nmeasurement"                                                       \
    },                                                                                        \
    {                                                                                         \
      "Assurez vous que la\nbalance est vide puis\nappuyez sur \"Tare\"",                     \
      "Assurez vous que la\nbalance est vide puis\nappuyez sur \"Tare\"",                     \
      "Make sure the scale\nis empty then\npress \"Tare\""                                    \
    },                                                                                        \
    {                                                                                         \
      "Remise à zéro\ndes balances...",                                                       \
      "Remise à zéro\ndes balances...",                                                       \
      "\n Reseting\nthe scales..."                                                            \
    },                                                                                        \
    {                                                                                         \
      "Placez une masse de\n%dg sur la balance\ncoté bord d'attaque,\npuis pressez \"Tare\"", \
      "Placez une masse de\n%dg sur la balance\ncoté bord d'attaque,\npuis pressez \"Tare\"", \
      "Place a mass of\n%dg on the scale on\nthe Leading edge side,\nthen press \"Tare\""     \
    },                                                                                        \
    {                                                                                         \
      "Placez une masse de\n%dg sur la balance\ncoté bord de fuite,\npuis pressez \"Tare\"",  \
      "Placez une masse de\n%dg sur la balance\ncoté bord de fuite,\npuis pressez \"Tare\"",  \
      "Place a mass of\n%dg on the scale on\nthe Trailing edge side,\nthen press \"Tare\""    \
    },                                                                                        \
    {                                                                                         \
      "Etalonnage terminé,\nscale_ba = %f\nscale_bf = %f",                                    \
      "Etalonnage terminé,\nscale_ba = %f\nscale_bf = %f",                                    \
      "Calibration complete,\nscale_ba = %f\nscale_bf = %f"                                   \
    }                                                                                         \
  }

  char* tr(const char* lang, const char* a_traduire);

#endif
