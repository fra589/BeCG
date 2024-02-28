/****************************************************************************/
/*                                                                          */
/* Copyright (C) 2023-2024 Gauthier Brière (gauthier.briere "at" gmail.com) */
/*                                                                          */
/* This file: update_script.js is part of BeCG                              */
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

var rebootDelay  = 15
var compteRebour = rebootDelay;

function update_onload() {

  // Vide le contenu du file input
  document.getElementById('file1').value = "";
  
  document.getElementById('upload_form').addEventListener('submit', (e) => {
    e.preventDefault();
    uploadFile();
  });
  document.getElementById('firmware-button').addEventListener('click',
    function (e) {
      e.target.classList.add('selected');
      document.getElementById('filesystem-button').classList.remove('selected');
      document.getElementById("file1").name = 'firmware';
      document.getElementById("status").innerHTML = "Téléchargement du Firmware";
      }
  );
  document.getElementById('filesystem-button').addEventListener('click',
    function (e) {
      e.target.classList.add('selected');
      document.getElementById('firmware-button').classList.remove('selected');
      document.getElementById("file1").name = 'filesystem';
      document.getElementById("status").innerHTML = "Téléchargement de l'application web";
    }
  );
  document.getElementById('file1').addEventListener('change', 
    function(e){
    var file = document.getElementById("file1").files[0];
      if (file && file.name){
        document.getElementById('button-send').disabled = false;
        document.getElementById("progressBar").value = 0; //wil clear progress bar after successful upload
      }else{
        document.getElementById('button-send').disabled = true;
      }
      if (document.getElementById('firmware-button').classList.contains("selected")) {
        document.getElementById('status').innerHTML = "Téléchargement du Firmware";
      } else {
        document.getElementById('status').innerHTML = "Téléchargement de l'application web";
      }
      document.getElementById("loaded_n_total").innerHTML = "";
    }
  );
}
function stringToBoolean(string){
  switch(string.toLowerCase().trim()){
    case "true": case "yes": case "1": return true;
    case "false": case "no": case "0": case null: return false;
    default: return Boolean(string);
  }
}
function disableAll() {
  document.getElementById("overlay").style.display = "block";
}
function enableAll() {
  document.getElementById("overlay").style.display = "none";
}
function uploadFile() {
  var file = document.getElementById("file1").files[0];
  var formdata = new FormData();
  
  formdata.append(document.getElementById("file1").name, file, file.name);
  
  var xhttp = new XMLHttpRequest();
  xhttp.upload.addEventListener("progress", progressHandler, false);
  xhttp.addEventListener("load", completeHandler, false);
  xhttp.addEventListener("loadstart", startHandler, false);
  xhttp.addEventListener("error", errorHandler, false);
  xhttp.addEventListener("abort", abortHandler, false);

  xhttp.open("POST", "/update");
  xhttp.setRequestHeader('Access-Control-Allow-Headers', '*');
  xhttp.setRequestHeader('Access-Control-Allow-Origin', '*');
  xhttp.send(formdata);
}
function progressHandler(event) {
  document.getElementById("loaded_n_total").innerHTML = "Envoyés " + event.loaded + " octets sur " + event.total;
  var percent = Math.round((event.loaded / event.total) * 100);
  document.getElementById("progressBar").value = percent;
  document.getElementById("status").innerHTML = percent + "% Téléchargé... patientez quelques instants";
}
function completeHandler(event) {
  enableAll();
  document.getElementById("status").innerHTML = "Téléchargement réussi."; //event.target.responseText;
  //document.getElementById("progressBar").value = 0; //wil clear progress bar after successful upload
  // Affiche l'image d'attente
  var dlgMask = document.getElementById("dlgMask0");
  var divWait = document.getElementById("attente0");
  dlgMask.classList.remove("noshow");
  divWait.classList.remove("noshow");
  // Compte à rebours pour attente reboot
  compteRebour = rebootDelay;
  document.getElementById("compte_a_rebour").innerHTML = compteRebour;
  setTimeout(function() { rebours(); }, 1000);
  // Recharge la page principale après 20 secondes
  var rebootTimeout = rebootDelay * 1000 + 100
  setTimeout(function() { document.location.replace("/") }, rebootTimeout);
}
function startHandler(event) {
  disableAll();
}
function errorHandler(event) {
  enableAll();
  document.getElementById("status").innerHTML = "Erreur du téléchargement.";
}
function abortHandler(event) {
    enableAll();
    document.getElementById("status").innerHTML = "Téléchargement annulé.";
}
function rebours() {
  // Mise à jour compte à rebour en attendant le reboot
  compteRebour = compteRebour - 1;
  document.getElementById("compte_a_rebour").innerHTML = compteRebour;
  if (compteRebour > 0) {
    setTimeout(function() { rebours(); }, 1000);
  }
}
