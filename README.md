m1-reseau
=========
Projet de manipulation de threads et de sockets en C

Application
-----------
Création d'un outil FTP en mode clients/serveur.

Fonctionnement
--------------
* Un serveur gère les requêtes
* Le client est un outil avec des fonctionnalités disponibles via un menu

### Fonctionnalités attendues
	* Envoyer un fichier à distance
	* Récupérer un fichier distant
	* Supprimer un fichier distant
	* Lister les fichiers à distance
	* Créer des répertoires distants
	* Déplacer un fichier distant

Contraintes
-----------
* Programme ne fonctionnant qu'avec des systèmes UNIX

Utilisation
-----------
	1) Compiler les programmes
		* gcc client.c -o client
		* gcc serveur.c -o serveur -lpthread
	2) Lancer le serveur dans un terminal
		* ./serveur
	3) Lancer un client dans un terminal
		* ./client <@ip_du_serveur>
		Répéter l'opération sur plusieurs terminaux pour lancer plusieurs clients