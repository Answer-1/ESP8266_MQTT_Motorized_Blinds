# ESP8266 MQTT Motorized Blinds

/// Version francaise plus bas ///

Home Assistant ESP8266 based MQTT motorized blinds

2in "faux wood" horizontal blinds controlled with Home Assistant via MQTT

Based on the great work of http://www.thesmarthomehookup.com/automated-motorized-window-blinds-horizontal-blinds/
and modified to use the AccelStepper library instead which I find smoother

This version can control two blinds independently and is also fitted with a DHT22 temp sensor.

Before the first run, you'll need to publish value 0 on the MQTT topic so it knows where to start.

-----------------------------------------------------------------------

Stores horizontaux controlés par Home Assistant via MQTT

Controle des stores horizontaux 2" de type "faux bois" (vendus chez Bouclair et surement ailleurs)

Basé sur le superbe travail de http://www.thesmarthomehookup.com/automated-motorized-window-blinds-horizontal-blinds/ 
et modifié par moi pour utiliser la librairie AccelStepper car je la trouve plus fluide.

Cette version peut comtroler deux stores indépendament et contient un DHT22 pour lire la température.

Contient aussi des fichiers STL pour imprimer une petite boite qui peut contenir le stepper motor (28byj-48 5v modifié unipolaire) et s'accrocher au store pour permettre le controle.

Avant la première exécution, il faut publier la valeur 0 sur le topic MQTT.
