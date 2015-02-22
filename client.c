/*-----------------------------------------------------------
Client a lancer apres le serveur avec la commande :
client <adresse-serveur> <message-a-transmettre>
------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h> //#include <linux/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>     /* pour fonctions sleep, read, write, close */
#include <stdbool.h>    /* pour booléens */

#include "constantes.h" //pour nos constantes générales prédéfinies

typedef struct sockaddr 	sockaddr;
typedef struct sockaddr_in 	sockaddr_in;
typedef struct hostent 		hostent;
typedef struct servent 		servent;

/**
 *
 */
void envoyer() {

}

/**
 *
 */
void recuperer() {

}

/**
 *
 */
void supprimer(int socket_descriptor) {

}

/**
 *
 */
void lister(int socket_descriptor) {
    char buffer[BUFFER_SIZE];
    int longueur;

    puts("Quel chemin ?");
    scanf("%s", buffer);
    if(strcmp(buffer, "/") == 0) {
        puts("Chemin interdit, remplacé par .");//bug si accès à la racine
        strcpy(buffer, ".");
    }
    buffer[strlen(buffer)] = '\0';
    if ((write(socket_descriptor, buffer, strlen(buffer))) < 0) {
        perror("erreur : impossible d'ecrire le message destine au serveur.");
        exit(0);
    }
    if((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0) {
        printf("reponse du serveur : \n");
        write(1,buffer,longueur);
    }

}

/**
 *
 */
void creerRep(int socket_descriptor) {
    char buffer[BUFFER_SIZE];
    int longueur;

    puts("Quel chemin ?");
    scanf("%s", buffer);
    // if(strcmp(buffer, "/") == 0) {
    //     puts("Chemin interdit, remplacé par .");//bug si accès à la racine
    //     strcpy(buffer, ".");
    // }
    buffer[strlen(buffer)] = '\0';
    if ((write(socket_descriptor, buffer, strlen(buffer))) < 0) {
        perror("erreur : impossible d'ecrire le message destine au serveur.");
        exit(0);
    }
    if((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0) {
        printf("reponse du serveur : \n");
        write(1,buffer,longueur);
    }
}

/**
 *
 */
void deplacer(int socket_descriptor) {

}

/**
 * procédure pour afficher le menu.
 */
void afficher_menu() {
    puts("");
    puts("--- MENU ---");
    puts("0 - Quitter");
    puts("1 - Envoyer un fichier");
    puts("2 - Récupérer un fichier");
    puts("3 - Supprimer un fichier");
    puts("4 - Lister les fichiers distants");
    puts("5 - Créer un répertoire");
    puts("6 - Déplacer un fichier");
    puts("");
}

/**
 * fonction affichant le menu et qui retourne la fonctionnalité souhaité
 *
 * @return entier "choix" représentant la fonctionnalité à effectuer
 */
int menu() {
    int choix;

    do {
        afficher_menu();
        scanf("%d", &choix);
    } while(choix<0 || choix>6);
    //TODO : bug si choix autre que chiffre

    return choix;
}

/**
 * Fonction principale qui lance le client.
 */
int main(int argc, char **argv) {

    int socket_descriptor, 	/* descripteur de socket */
        longueur; 		/* longueur d'un buffer utilisé */
        sockaddr_in adresse_locale; 	/* adresse de socket local */
        hostent *	ptr_host; 		/* info sur une machine hote */
        servent *	ptr_service; 		/* info sur service */
    char buffer[BUFFER_SIZE];
    char *	prog; 			/* nom du programme */
    char *	host; 			/* nom de la machine distante */
    char mesg[BUFFER_SIZE]; 			/* message envoyé */
    int choix; //choix de la fonctionnalité dans le menu
    bool stop; // condition d'arrêt de l'app

    //vérification du nombre d'arguments
    if (argc != 2) {
        perror("usage : client <adresse-serveur>");
        exit(1);
    }

    prog = argv[0];
    host = argv[1];

    printf("nom de l'executable : %s \n", prog);
    printf("adresse du serveur  : %s \n", host);


    /****************************************
     * Initialisation des paramètres réseau *
     ****************************************/
    if ((ptr_host = gethostbyname(host)) == NULL) {
        perror("erreur : impossible de trouver le serveur a partir de son adresse.");
        exit(1);
    }

/* copie caractere par caractere des infos de ptr_host vers adresse_locale */
    bcopy((char*)ptr_host->h_addr, (char*)&adresse_locale.sin_addr, ptr_host->h_length);
adresse_locale.sin_family = AF_INET; /* ou ptr_host->h_addrtype; */

/* 2 facons de definir le service que l'on va utiliser a distance */
/* (commenter l'une ou l'autre des solutions) */

/*-----------------------------------------------------------*/
/* SOLUTION 1 : utiliser un service existant, par ex. "irc" */

    if ((ptr_service = getservbyname("ftp","tcp")) == NULL) {
        perror("erreur : impossible de recuperer le numero de port du service desire.");
        exit(1);
    }
    adresse_locale.sin_port = htons(ptr_service->s_port);

/*-----------------------------------------------------------*/

/*-----------------------------------------------------------*/
/* SOLUTION 2 : utiliser un nouveau numero de port */
//adresse_locale.sin_port = htons(5000);
/*-----------------------------------------------------------*/

    printf("numero de port pour la connexion au serveur : %d \n", ntohs(adresse_locale.sin_port));

/* creation de la socket */
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("erreur : impossible de creer la socket de connexion avec le serveur.");
        exit(1);
    }

/* tentative de connexion au serveur dont les infos sont dans adresse_locale */
    if ((connect(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
        perror("erreur : impossible de se connecter au serveur.");
        exit(1);
    }

    printf("connexion etablie avec le serveur. \n");

    /*******************
     * Fonctionnalités *
     *******************/
    stop = false;
    while(!stop) {
        choix = menu();
        //condition arrêtant le programme si l'utilisateur fait une mauvaise saisie
        // if( !choix ) {
        //     break;
        // }

        switch(choix) {
            case 1:
                envoyer();
                break;

            case 2:
                recuperer();
                break;

            case 3:
                strcpy(buffer, "3\0");
                if ((write(socket_descriptor, buffer, strlen(buffer))) < 0) {
                    perror("erreur : impossible d'ecrire le message destine au serveur.");
                    break;
                }
                supprimer(socket_descriptor);
                break;

            case 4:
                strcpy(buffer, "4\0");
                if ((write(socket_descriptor, buffer, strlen(buffer))) < 0) {
                    perror("erreur : impossible d'ecrire le message destine au serveur.");
                    break;
                }
                lister(socket_descriptor);
                break;

            case 5:
                strcpy(buffer, "5\0");
                if ((write(socket_descriptor, buffer, strlen(buffer))) < 0) {
                    perror("erreur : impossible d'ecrire le message destine au serveur.");
                    break;
                }
                creerRep(socket_descriptor);
                break;

            case 6:
                strcpy(buffer, "6\0");
                if ((write(socket_descriptor, buffer, strlen(buffer))) < 0) {
                    perror("erreur : impossible d'ecrire le message destine au serveur.");
                    break;
                }
                deplacer(socket_descriptor);
                break;

            default:
                stop = true;
                break;
        }
        memset(buffer, 0, sizeof(buffer));
    }

    puts("\nfin de la reception.");
    close(socket_descriptor);

    puts("connexion avec le serveur fermee, fin du programme.");

    return 0;

}
