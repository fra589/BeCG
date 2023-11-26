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

#ifndef tools_h
  #define tools_h

  extern bool reset_scale_en_cours;
  extern bool tare_en_cours;
  extern bool etalonnage_en_cours;

  void getEepromStartupData(void);
  void balancesInit(void);
  void tare(void);
  void resetScale(void);
  float etalon(String uri);
  void etalonnage(void);
  void waitForTareButton(void);
  void resetFactory(void);
  void EEPROM_format(void);
  void EEPROM_writeStr(int address, char *value, int len);

  bool isIp(String str);
  String IPtoString(IPAddress ip);
  String formateCP437(String utf8);
  String macToString(const unsigned char* mac);

#endif // tools_h
