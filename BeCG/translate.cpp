/****************************************************************************/
/*                                                                          */
/* Copyright (C) 2023-2024 Gauthier Brière (gauthier.briere "at" gmail.com) */
/*                                                                          */
/* This file: translate.cpp is part of BeCG                                 */
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

char translations[NB_TRANSLATIONS][NB_LANG+1][MAX_LENGHT] = TRANSLATIONS;

char* tr(const char* lang, const char* a_traduire) {
  
  int ilang;
  int i;
  char *buff = NULL;
  
  if (strncmp(lang, "en", (size_t)LANG_LEN) == 0) {
    ilang = EN;
  } else {
    ilang = FR; // Par défaut, on est en Français
  }
  
  for (i=0; i<NB_TRANSLATIONS; i++) {
    if (strncmp(a_traduire, &translations[i][0][0], MAX_LENGHT) == 0) {
      buff=(char*)malloc((strlen(&translations[i][ilang][0])+1)*sizeof(char));
      if (buff) {
        strcpy(buff, &translations[i][ilang][0]);
      }
      break;
    }
  }
  
  if (buff) {
    return buff;
  } else {
    buff=(char*)malloc((strlen(a_traduire)+1)*sizeof(char));
    strcpy(buff, a_traduire);
    return buff;
  }
  
}
