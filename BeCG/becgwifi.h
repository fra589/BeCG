/****************************************************************************/
/*                                                                          */
/* Copyright (C) 2023-2024 Gauthier Brière (gauthier.briere "at" gmail.com) */
/*                                                                          */
/* This file: becgwifi.h is part of BeCG                                    */
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

#ifndef becgwifi_h
  #define becgwifi_h

  void wifiApInit(void);
  void wifiClientInit(void);

  #ifdef DEBUG
    void onStationConnected(const WiFiEventSoftAPModeStationConnected& evt);
    void onStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& evt);
    void onProbeRequestPrint(const WiFiEventSoftAPModeProbeRequestReceived& evt);
  #endif

#endif // becgwifi_h
