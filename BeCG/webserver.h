/****************************************************************************/
/*                                                                          */
/* Copyright (C) 2023-2024 Gauthier Brière (gauthier.briere "at" gmail.com) */
/*                                                                          */
/* This file: webserver.h is part of BeCG                                   */
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

#ifndef webserver_h
  #define webserver_h

  void webServerInit(void);
  bool captivePortal(void);
  void handleRoot(void);
  void handleGetValues(void);
  void handleGetVersion(void);
  void handleGetWifi(void);
  String getWifiNetworks(void);
  void handleGetNetworks(void);
  void handleAffichage(void);
  void handleTare(void);
  void handleResetScale(void);
  void handleEtalon(void);
  void handleStopMesure(void);
  void handleStartMesure(void);
  bool handleFileRead(String path);
  void handleGetSettings(void);
  void handleSetSettings(void);
  void handleWifiConnect(void);
  void handleDeconnection(void);
  void handleSetLang(void);
  void handleFactory(void);
  void handleNotFound(void);
  void handleReboot(void);
  void handleUpdate(void);

#endif
