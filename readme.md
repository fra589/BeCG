<h1 align="center"><img align="left" src='https://github.com/fra589/BeCG/blob/master/BeCG/data/BeCG-couleur.svg' width='25%' />Balance électronique pour mesure<br />et calculs du Centre de Gravité<br />des modèles réduits d'avion.</h1>

La balance affiche sur son écran les masse mesurées sur les appuis du bord d'attaque et du bord de fuite, la masse totale et la position du centre de gravité en millimètres mesurée à partir de la butée du bord d'attaque de l'aile.
La balance intégre également une interface WiFi qui permet de consulter les mesures et de régler ses paramètres.
1) Soit en se connectant au point d'accès de la balance, par exemple avec un smartphone,
2) soit en connectant la balance à un réseau WiFi existant (après avoir configuré le réseau WiFi à l'aide de la connexion 1.).

Une fois la connexion effectuée, on accède à son interface au travers d'un navigateur web standard.
<p align='center'>
<img src='https://github.com/fra589/BeCG/blob/master/images/perspectiveFreeCAD.png' width='50%' />&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<img src='https://github.com/fra589/BeCG/blob/master/images/captureDouble.png' width='17%' />
</p>

La balance est basée sur :
- Une carte microcontrôleur WiFi ESP8266 de type Wemos D1 mini, 
- deux capteurs de masse formés par une cellule de charge et un module amplificateur HX711,
- un écran I2C SSD1306 de 0.96 pouce (résolution 128x64),
- une structure imprimée en 3D (avec en option un socle métallique pour plus de rigidité).

La précision de mesure estimée de la masse est de l'ordre de ±0.2g, celle du calcul de centrage est estimée à environ ±0.2mm.

---

<h2 align="center">Les documentations d'installation et d'utilisation se trouvent dans le WiKi :<br />https://github.com/fra589/BeCG/wiki</h2>
