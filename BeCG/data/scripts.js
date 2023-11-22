/****************************************************************************/
/*                                                                          */
/* Copyright (C) 2023-2023 Gauthier Brière (gauthier.briere "at" gmail.com) */
/*                                                                          */
/* This file: script.js is part of BeCG                                     */
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

var suiteOK    = false;
var attenteOK  = false;
var connectOK  = false;
var scale_baOK = false
var scale_ba   = 0.0;
var scale_bfOK = false
var scale_bf   = 0.0;
var chkRefresh_oldstate = true;

function page_onload() {
  // Mise à jour des infos de la balance
  get_settings();
  // Affichage dynamique des valeurs de la balance
  setTimeout(function() { XMLHttpRequest_get("getvalues") }, 500);

  //setTimeout(function() { alert(window.innerWidth + " - " + document.body.clientWidth) }, 1000);

}

function netconfig_onload() {
  // Mise à jour des infos de la balance
  get_settings();
  // Retrouve la liste des réseaux vsibles
  //XMLHttpRequest_get("getnetworks");
  get_networks();
}

function get_settings() {
  // Mise à jour des infos de version
  XMLHttpRequest_get("getversion");
  // Mise à jour des infos de version
  XMLHttpRequest_get("getwifi");
  // Recupère les paramètres de la balance
  XMLHttpRequest_get("getsettings");
}

async function get_networks() {
  // Affiche le fond gris
  var dlgMask = document.getElementById("dlgMask0");
  dlgMask.classList.remove("noshow");
  var divWait = document.getElementById("attente0");
  // Affiche l'annimation d'attente
  divWait.classList.remove("noshow");
  // envoi la requette au serveur et attend la réponse
  attenteOK = false;
  XMLHttpRequest_get("getnetworks");
  while (!attenteOK) {
    await sleep(100);
  }
  attenteOK = false;
  // Masque l'animation d'attente
  divWait.classList.add("noshow");
  // Masque le fond gris
  dlgMask.classList.add("noshow");
}

function setNetworkList(xml) {
  // Fonction appelée en retour de XMLHttpRequest_get("getnetworks")
  var htmlNetworkTable = "";
  var tmpSSID      = "";
  var tmpChannel   = 0;
  var tmpRSSI      = "";
  var tmpCrypt     = "";
  var tmpImgSignal = "";
  
  var divNetworkTable = document.getElementById("networkTable");
  
  htmlNetworkTable  = "<table class=\"netTables\">\n";
  htmlNetworkTable += "  <thead>\n";
  htmlNetworkTable += "    <tr>\n";
  htmlNetworkTable += "      <th>&nbsp;</th>\n";
  htmlNetworkTable += "      <th>SSID</th>\n";
  htmlNetworkTable += "      <th>Canal</th>\n";
  htmlNetworkTable += "      <th>Sécurité</th>\n";
  htmlNetworkTable += "    </tr>\n";
  htmlNetworkTable += "  </thead>\n";
  htmlNetworkTable += "  <tbody>\n";

  var netListe = xml.getElementsByTagName("network")
  if (netListe.length > 0) {
    for(var i = 0; i< netListe.length; i++){
      // Données du réseau
      tmpSSID    = netListe[i].getElementsByTagName("SSID")[0].childNodes[0].nodeValue;
      tmpChannel = netListe[i].getElementsByTagName("channel")[0].childNodes[0].nodeValue;
      tmpRSSI    = netListe[i].getElementsByTagName("RSSI")[0].childNodes[0].nodeValue;
      tmpCrypt   = netListe[i].getElementsByTagName("encryption")[0].childNodes[0].nodeValue;
      // Choix du pictogramme en fonction de la qualité du signal
      // RSSI (dBm) Interprétation
      // -30 dBm    Extraordinaire (êtes vous assis sur la borne? ^^)
      // -67 dBm    Très bon signal
      // -70 dBm    Très acceptable
      // -80 dBm   	as terrible du tout
      // -90 dBm    Inutilisable
      if (tmpRSSI <= -90) {
        tmpImgSignal = "signal0.svg";
      } else if ((tmpRSSI > -90) && (tmpRSSI <= -80)) {
        tmpImgSignal = "signal1.svg";
      } else if ((tmpRSSI > -80) && (tmpRSSI <= -70)) {
        tmpImgSignal = "signal2.svg";
      } else if ((tmpRSSI > -70) && (tmpRSSI <= -67)) {
        tmpImgSignal = "signal3.svg";
      } else if (tmpRSSI > -67) {
        tmpImgSignal = "signal4.svg";
      }
      htmlNetworkTable += "    <tr class=\"trlink\" onclick=\"wifi_connect('" + tmpSSID + "' , '" + tmpChannel + "')\">\n";
      htmlNetworkTable += "      <td class=\"centreVertical\"><img src=\"" + tmpImgSignal + "\" title=\"RSSI = " + tmpRSSI + "\" /></td>\n";
      htmlNetworkTable += "      <td>" + tmpSSID + "</td>\n";
      htmlNetworkTable += "      <td>" + tmpChannel + "</td>\n";
      htmlNetworkTable += "      <td>" + tmpCrypt + "</td>\n";
      htmlNetworkTable += "    </tr>\n";
    }
  } else {
    htmlNetworkTable += "    <tr>\n";
    htmlNetworkTable += "      <td colspan=\"4\">Aucun réseau disponible</td>";
    htmlNetworkTable += "    </tr>\n";
  }
  htmlNetworkTable += "  </tbody>\n";
  htmlNetworkTable += "</table>\n";
  
  divNetworkTable.innerHTML = htmlNetworkTable;
  
}

async function wifi_connect(SSID, channel) {
  // TODO saisie du mot de passe et confirmation 
  // TODO voir pourquoi on ne reçois pas de réponse XML
  // ? le serveur perd les infos du client lors de la déconnexion ?
  var pwd = "C'est1secret";

  var ssid_input = document.getElementById("ssid_input");
  var pwd_input  = document.getElementById("pwd_input");
  var divWait    = document.getElementById("attente0");
  var btnConnect = document.getElementById("btnConnect");
  var btnAnnuler = document.getElementById("btnAnnuler");

  ssid_input.value = SSID + " (ch." + channel + ")";
  
  connectOK = true;
  suiteOK = false;

  // Affiche la boite de dialogue de saisie du mot de passe réseau
  afficheDialog('dlgConnect');
  // Donne le focus au champ de saisie du mot de passe
  pwd_input.focus();

  // Attente click sur bouton Connexion
  while (!suiteOK) {
    await sleep(100);
  }
  suiteOK = false;
  
  if (connectOK) {
    pwd = pwd_input.value;
    // Désactive les boutons de la boite de dialogue
    btnConnect.disabled = true;
    btnAnnuler.disabled = true;
    // Affiche l'annimation d'attente
    divWait.classList.remove("noshow");
    // Lance la connexion de la balance
    //alert("XMLHttpRequest_post_wificonnect(" + SSID + ", " + pwd + ", " + channel + ");");
    XMLHttpRequest_post_wificonnect(SSID, pwd, channel);
    // On fermera la boite de dialogue et on masquera
    // l'animation d'attente dans 10 secondes
    setTimeout(function() { divWait.classList.add("noshow"); }, 10000);
    setTimeout(function() { closeDialog('dlgConnect'); location.reload(); }, 10000);
  } else {
    alert("Connexion annulée.");
    closeDialog('dlgConnect');
  }

}

function connect_clique() {
  connectOK = true;
  suiteOK   = true;
}

function connect_cancel() {
  connectOK = false;
  suiteOK   = true;
}

function connect_keyup(e) {
  if (e.keyCode === 13) {
    document.getElementById("btnConnect").click();
  } else if (e.keyCode === 27) {
    document.getElementById("btnAnnuler").click();
  }
}

function deconnect_clique() {
  if (confirm("Voulez-vous vraiment vous déconnecter du réseau en cours\net effacer les paramètres WiFi client dans la balance ?")) {
    XMLHttpRequest_get("deconnexion");
  }
}

function XMLHttpRequest_post_wificonnect(SSID, pwd, channel) {
  var xhttp = new XMLHttpRequest();

  xhttp.onreadystatechange = function() {
    if (xhttp.readyState == 4) {
      if ((xhttp.status == 200) || (xhttp.status == 0)) {
        XMLHttpResult("wificonnect", xhttp.responseXML, xhttp.responseText);
      } else {
        alert("XMLHttpRequest_post_wificonnect() : Erreur " + xhttp.status);
      }
    }
  };

  var ssid_encode = encodeURIComponent(SSID);
  var pwd_encode  = encodeURIComponent(pwd);
  xhttp.open("POST", "wificonnect", true);
  xhttp.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
  xhttp.send("ssid=" + ssid_encode + "&pwd=" + pwd_encode + "&channel=" + channel);
}

function XMLHttpRequest_post_settings() {
  var paf_ba    = document.getElementById("paf_ba").value;
  var entraxe   = document.getElementById("entraxe").value;
  var tare      = document.getElementById("tare").value;
  var xhttp = new XMLHttpRequest();

  xhttp.onreadystatechange = function() {
    if (xhttp.readyState == 4) {
      if ((xhttp.status == 200) || (xhttp.status == 0)) {
        XMLHttpResult("setsettings", xhttp.responseXML, xhttp.responseText);
      } else {
        alert("XMLHttpRequest_post_settings() : Erreur " + xhttp.status);
      }
    }
  };

  xhttp.open("POST", "setsettings", true);
  xhttp.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
  xhttp.send("paf_ba=" + paf_ba + "&entraxe=" + entraxe + "&tare=" + tare);
}

function XMLHttpRequest_get(requette) {
  // Requette XML HTTP GET
  var heure = document.getElementById("heure");
  if (heure != null) {
    var now = new Date(Date.now());
    heure.textContent = now.getHours() + ":" + now.getMinutes() + ":" + now.getSeconds() + "."+ now.getMilliseconds();
  }
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (xhttp.readyState == 4) {
      if ((xhttp.status == 200) || (xhttp.status == 0)) {
        XMLHttpResult(requette, xhttp.responseXML, xhttp.responseText);
      } else {
        alert("XMLHttpRequest_get(" + requette + ") : Erreur " + xhttp.status);
      }
      //alert("OK"); // C'est bon \o/
    }
  };
  xhttp.open("GET", requette, true);
  xhttp.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
  xhttp.send();
}

function XMLHttpResult(requette, xml, text) {
  // Traitement de la réponse XML HTTP GET si existe...
  if (xml != null) {
    if (requette == "getversion") {
      var version_string = xml.getElementsByTagName("string")[0].childNodes[0].nodeValue;
      var doc_version = document.getElementById("version");
      doc_version.textContent = version_string;

    } else if (requette == "getwifi") {
      var ap_ssid  = xml.getElementsByTagName("ap_ssid")[0].childNodes[0].nodeValue;
      var ap_ip    = xml.getElementsByTagName("ap_ip")[0].childNodes[0].nodeValue;
      var cli_ssid = xml.getElementsByTagName("cli_ssid")[0].childNodes[0].nodeValue;
      var cli_ip   = xml.getElementsByTagName("cli_ip")[0].childNodes[0].nodeValue;

      var doc_ap_ssid  = document.getElementById("ap_ssid");
      var doc_ap_ip    = document.getElementById("ap_ip");
      var doc_cli_ssid = document.getElementById("cli_ssid");
      var doc_cli_ip   = document.getElementById("cli_ip");

      doc_ap_ssid.textContent  = ap_ssid;
      doc_ap_ip.textContent    = ap_ip;
      doc_cli_ssid.textContent = cli_ssid;
      doc_cli_ip.textContent   = cli_ip;
      
      // Met à jour le réseau en cours si la page en cours est
      // netconfig.html (présence des inputs "ssidActuel" et ipActuel"
      var ssidActuel = document.getElementById("ssidActuel");
      var ipActuel = document.getElementById("ipActuel");
      if (ssidActuel != null) {
        ssidActuel.value = cli_ssid;
      }
      if (ipActuel != null) {
        ipActuel.value = cli_ip;
      }

    } else if (requette == "getvalues") {
      var masse_ba     = xml.getElementsByTagName("ba")[0].childNodes[0].nodeValue;
      var masse_bf     = xml.getElementsByTagName("bf")[0].childNodes[0].nodeValue;
      var masse_totale = xml.getElementsByTagName("total")[0].childNodes[0].nodeValue;
      var position_cg  = xml.getElementsByTagName("cg")[0].childNodes[0].nodeValue;

      var doc_ba    = document.getElementById("ba");
      var doc_bf    = document.getElementById("bf");
      var doc_total = document.getElementById("total");
      var doc_cg    = document.getElementById("cg");
      
      doc_ba.textContent    = masse_ba + " g";
      doc_bf.textContent    = masse_bf + " g";
      doc_total.textContent = masse_totale + " g";
      if (!isNaN(position_cg)) {
        doc_cg.textContent    = position_cg + " mm";
      } else {
        doc_cg.textContent    = position_cg;
      }
    } else if (requette == "resetscale") {
      scale_baOK = false
      scale_ba   = 0.0;
      scale_bfOK = false
      scale_bf   = 0.0;
      attenteOK = true;
    } else if (requette == "tare") {
      attenteOK = true;
    } else if (requette == "etalonba") {
      scale_baOK = true;
      scale_ba   = xml.getElementsByTagName("scale")[0].childNodes[0].nodeValue;
      attenteOK = true;
    } else if (requette == "etalonbf") {
      scale_bfOK = true;
      scale_bf   = xml.getElementsByTagName("scale")[0].childNodes[0].nodeValue;
      attenteOK = true;
    } else if ((requette == "getsettings") || (requette == "setsettings")) {
      var paf_ba          = xml.getElementsByTagName("paf_ba")[0].childNodes[0].nodeValue;
      var entraxe         = xml.getElementsByTagName("entraxe")[0].childNodes[0].nodeValue;
      var tare            = xml.getElementsByTagName("tare")[0].childNodes[0].nodeValue;
      var input_paf_ba    = document.getElementById("paf_ba");
      var input_entraxe   = document.getElementById("entraxe");
      var input_tare      = document.getElementById("tare");
      input_paf_ba.value  = paf_ba;
      input_entraxe.value = entraxe;
      input_tare.value    = tare;
    } else if (requette == "getnetworks") {
      // Rempli la liste des réseaux disponibles
      setNetworkList(xml);
      attenteOK = true;
    } else if (requette == "wificonnect") {
      result = xml.getElementsByTagName("result")[0].childNodes[0].nodeValue;
      if (result == "OK") {
        location.reload();
      } else {
        alert("Erreur de connexion : \n" + result);
      }
    }
  }
  if (requette == "getvalues") {
      // Appel recursif pour boucler au lieu d'utiliser  setInterval()
      // Cela assure que te traitement à été terminé avant de relancer
      // la requette vers le serveur web.
      autoRefresh();
  }
}

function autoRefresh() {
  var chkRefresh = document.getElementById("auto_refresh");
  if (chkRefresh.checked) {
    // Appel recursif pour boucler au lieu d'utiliser  setInterval()
    // Cela assure que te traitement à été terminé avant de relancer
    // la requette vers le serveur web.
    setTimeout(function() { XMLHttpRequest_get("getvalues") }, 250);
    if (!chkRefresh_oldstate) {
      // on s'assure que les mesures sont en  route
      XMLHttpRequest_get("startmesure");
      // Reset de l'écran de la balance
      XMLHttpRequest_get("affichage?reset");
      // Memorise l'état check de la case à cocher
      chkRefresh_oldstate = true;
    }
  } else {
      // Memorise l'état check de la case à cocher
      chkRefresh_oldstate = false;
  }
}

async function balance_tare() {
  // Affiche le fond gris
  var dlgMask = document.getElementById("dlgMask0");
  var divWait = document.getElementById("attente0");
  dlgMask.classList.remove("noshow");
  
  if (confirm("Remise à zéro de la balance ?")) {
    // Affiche l'annimation d'attente
    divWait.classList.remove("noshow");
    // envoi la requette au serveur et attend la réponse
    attenteOK = false;
    XMLHttpRequest_get("tare");
    while (!attenteOK) {
      await sleep(100);
    }
    attenteOK = false;
    // Masque l'animation d'attente
    divWait.classList.add("noshow");
    
  }
  // Masque le fond gris
  dlgMask.classList.add("noshow");
}

function afficheDialog(dialog_id) {
  // Affiche la boite de dialogue
  var dlgMask = document.getElementById("dlgMask0");
  var dlgBox  = document.getElementById(dialog_id);
  
  dlgMask.classList.remove("noshow");
  window.setTimeout(function () {
    dlgBox.classList.remove("masquer");
  }, 0.2);
  if (dialog_id == "dlgEtalonnage") {
    etalonnage();
  }
}

async function etalonnage() {
  var divWait    = document.getElementById("attente0");
  var msg        = "";
  var chkRefresh = document.getElementById("auto_refresh");
  var tare       = document.getElementById("tare").value;

  // Désactive le rafraichissement automatique
  chkRefresh.checked = false;
  // Prépare la boite de dialogue
  // Active le bouton suite
  document.getElementById('btnSuite').disabled = false;
  // Désactive le bouton de fermeture de la boite
  document.getElementById('btnClose').innerHTML = "Annuler";

  // Affiche premier message
  msg = "<p>Assurez vous que la<br />balance est vide puis<br />Cliquez sur \"Suivant\".</p>";
  document.getElementById("dlgContenu0").innerHTML = msg;
  // Bloque les mesures sur la balance
  XMLHttpRequest_get("stopmesure");
  XMLHttpRequest_get("affichage?text=Etalonnage réseau\nen cours...");
  
  // Attente click sur bouton suite
  while (!suiteOK) {
    await sleep(100);
  }
  suiteOK = false;

  // Désactive le bouton de fermeture de la boite, il faut aller jusqu'au bout...
  document.getElementById('btnClose').innerHTML = "Fermer";
  document.getElementById('btnClose').disabled = true;
  
  // Affiche la suite
  msg = "<p>Remise à zéro<br />des balances...</p>";
  document.getElementById("dlgContenu0").innerHTML = msg;

  // Préparation de l'étalonnage (reset scale) et remise à zéro des balances (tare)
  divWait.classList.remove("noshow");
  attenteOK = false;
  XMLHttpRequest_get("resetscale");
  // Attente réponse de resetscale
  while (!attenteOK) {
    await sleep(100);
  }
  attenteOK = false;
  XMLHttpRequest_get("tare");
  // Attente réponse de tare
  while (!attenteOK) {
    await sleep(100);
  }
  attenteOK = false;
  divWait.classList.add("noshow");

  XMLHttpRequest_get("affichage?text=Etalonnage réseau\nen cours...");

  msg = "<p>Placez une masse de<br />" + tare + " g sur la balance<br />coté bord d'attaque, puis<br />Cliquez sur \"Suivant\".</p>"; 
  document.getElementById("dlgContenu0").innerHTML = msg;

  // Attente click sur bouton suite
  while (!suiteOK) {
    await sleep(100);
  }
  suiteOK = false;
  
  // Affiche la suite
  msg = "<p>Mesure de la tare<br />bord d'attaque...</p>";
  document.getElementById("dlgContenu0").innerHTML = msg;

  // Mesure tare BA
  divWait.classList.remove("noshow");
  attenteOK = false;
  XMLHttpRequest_get("etalonba");
  // Attente réponse de etalonba
  while (!attenteOK) {
    await sleep(100);
  }
  attenteOK = false;
  divWait.classList.add("noshow");

  XMLHttpRequest_get("affichage?text=Etalonnage réseau\nen cours...");

  msg = "<p>Placez une masse de<br />" + tare + " g sur la balance<br />coté bord de fuite, puis<br/>Cliquez sur \"Suivant\".</p>"; 
  document.getElementById("dlgContenu0").innerHTML = msg;

  // Attente click sur bouton suite
  while (!suiteOK) {
    await sleep(100);
  }
  suiteOK = false;
  
  // Affiche la suite
  msg = "<p>Mesure de la tare<br />bord de fuite...</p>";
  document.getElementById("dlgContenu0").innerHTML = msg;

  // Mesure tare BF
  divWait.classList.remove("noshow");
  attenteOK = false;
  XMLHttpRequest_get("etalonbf");
  // Attente réponse de etalonbf
  while (!attenteOK) {
    await sleep(100);
  }
  attenteOK = false;
  divWait.classList.add("noshow");

  // Affiche la suite
  msg = "<p>Etalonnage terminé,<br />- nscale_ba = " + scale_ba + "<br />- nscale_bf = " + scale_bf + "</p>";
  document.getElementById("dlgContenu0").innerHTML = msg;

  // Réinitialise l'affichage et relance les mesures sur la balance
  XMLHttpRequest_get("affichage?reset");
  XMLHttpRequest_get("startmesure");
  
  // Désactive le bouton suite qui a fini son boulo
  document.getElementById('btnSuite').disabled = true;
  // Active le bouton de fermeture de la boite
  document.getElementById('btnClose').disabled = false;
  
  // Réactive le rafraichissement automatique
  chkRefresh.checked = true;

}

function suite_clique() {
  suiteOK = true;
}

function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

function closeDialog(dialog_id) {
  // Ferme la boite de dialogue
  var dlgMask = document.getElementById("dlgMask0");
  var dlgBox  = document.getElementById(dialog_id);
  dlgBox.classList.add("masquer");
  window.setTimeout(function () {
    dlgMask.classList.add("noshow");
  }, 1);
}

function reboot() {
  // Redémarrage de la balance
  // Affiche l'image d'attente
  var dlgMask = document.getElementById("dlgMask0");
  var divWait = document.getElementById("attente0");
  dlgMask.classList.remove("noshow");
  divWait.classList.remove("noshow");
  if (confirm("Redémarrer la balance ?")) {
    // Reboot
    XMLHttpRequest_get("reboot");
    setTimeout(function() { document.location.replace("/") }, 15000);
  } else {
    dlgMask.classList.add("noshow");
    divWait.classList.add("noshow");
  }
  
}

