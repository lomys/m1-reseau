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


/* signatures des méthodes */
void envoyer(int socket_descriptor);
void recuperer(int socket_descriptor);
void supprimer(int socket_descriptor);
void lister(int socket_descriptor);
void creerRep(int socket_descriptor);
void deplacer(int socket_descriptor);
void afficher_menu();
int menu();
int main(int argc, char **argv);

/**
 * Procédure appelée pour envoyer un fichier sur le serveur
 */
void envoyer(int socket_descriptor) {
    char buffer[BUFFER_SIZE];
    int longueur;
    char temp[PATH_LIMIT];
    //Récupération du path du fichier
    puts("Indiquer le path du fichier à envoyer.");
    scanf("%s", buffer);
    FILE* fichier= NULL;
    fichier = fopen(buffer,"rb");
    buffer[strlen(buffer)] = '\0';
    memset(buffer,0,sizeof(buffer));
    //Envoie du path ou sauvegarder le fichier
    puts("Indiquer le path de destination du fichier envoyé.");
    scanf("%s", buffer);
    buffer[strlen(buffer)] = '\0';
    if ((write(socket_descriptor, buffer, strlen(buffer))) < 0) {
        perror("erreur : impossible d'ecrire le message destine au serveur.");
        exit(0);
    }
    memset(buffer,0,sizeof(buffer));

    char * res ;
    int m,position_actuel;
    int fin = 0;
    if (fichier != NULL){
         // On lit le fichier tant qu'on ne reçoit pas d'erreur (NULL)
        fseek(fichier, 0, SEEK_END);
        m = ftell(fichier);
        fseek(fichier, 0, SEEK_SET);
        sprintf(buffer, "%d", m);
        buffer[strlen(buffer)] = '\0';
        if ((write(socket_descriptor, buffer, sizeof(buffer))) < 0) {
            perror("erreur : impossible d'envoyer la taille du fichier.");
            exit(0);
        }
        memset(buffer,0,sizeof(buffer));   
        while(fin == 0) 
        {
            position_actuel = ftell(fichier);
            fseek(fichier, 0, SEEK_SET);
            if(m - position_actuel > BUFFER_SIZE){
                fseek(fichier,position_actuel,SEEK_CUR);
                res = malloc((sizeof(char)) * BUFFER_SIZE);
                position_actuel = position_actuel + BUFFER_SIZE;
                fread(res, BUFFER_SIZE, 1, fichier);         
            }else{
                fin = 1;         
                res = malloc((sizeof(char)) * m-position_actuel);
                fseek(fichier, position_actuel, SEEK_CUR);
                fread(res, m-position_actuel, 1, fichier);      
            }
            
            res[strlen(res)] = '\0';

            write(socket_descriptor, res, strlen(res));
            memset(res,0,strlen(res));
        }
        fclose(fichier);
        if((longueur = read(socket_descriptor, buffer, sizeof(buffer))) > 0) {
            printf("reponse du serveur : \n");
            write(1,buffer,longueur);
        }else{
            printf("Absence de réponse du serveur. \n");
        }
    }else{
        printf("Impossible d'ouvrir le fichier.\n");
    }
}

/**
 * Procédure appelée pour réceptionner un fichier du serveur
 */
void recuperer(int socket_descriptor) {
    int pointeur_fichier = 0;
    char buffer[BUFFER_SIZE];
    char *res;
    int tf;
    char taille_fichier[BUFFER_SIZE];
    FILE* fichier= NULL;
    puts("Indiquer le path où enregistrer le fichier.");
    scanf("%s", buffer);
    char cmd[PATH_LIMIT+7];
    strcpy(cmd, "rm -fr ");
    strcat(cmd, buffer);
    system(cmd);
    fichier = fopen(buffer,"ab");
    buffer[strlen(buffer)] = '\0';
    memset(buffer,0,sizeof(buffer));
    //Envoie du path du fichier à receptionner
    puts("Indiquer le path du fichier à receptionner.");
    scanf("%s", buffer);
    buffer[strlen(buffer)] = '\0';
    if ((write(socket_descriptor, buffer, strlen(buffer))) < 0) {
        perror("erreur : impossible d'ecrire le message destine au serveur.");
        exit(0);
    }
    memset(buffer,0,sizeof(buffer));

    //lecture de la taille du fichier enregistrer
    read(socket_descriptor, taille_fichier, sizeof(taille_fichier));
    tf = atoi(taille_fichier);
    while(pointeur_fichier < tf){
        read(socket_descriptor,buffer,sizeof(buffer));
        res = buffer;
        pointeur_fichier = pointeur_fichier + strlen(res);
        fwrite(res,1,strlen(res),fichier);
        memset(res, 0, strlen(res));
    }
    fclose(fichier);
    memset(buffer,0,strlen(buffer));
    strcpy(buffer, "Fichier récupéré.");            
    printf("%s\n", buffer);
}

/**
 * Procédure appelée pour supprimer un fichier ou un répertoire distant
 */
void supprimer(int socket_descriptor) {
    char buffer[BUFFER_SIZE];
    int longueur;

    puts("Quel chemin ?");
    scanf("%s", buffer);
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
 * Procédure appelée pour lister les fichiers d'un chemin d'accès distant
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
 * Procédure appelée pour créer un répertoire ou une suite de répertoires sur le serveur
 */
void creerRep(int socket_descriptor) {
    char buffer[BUFFER_SIZE];
    int longueur;

    puts("Quel chemin ?");
    scanf("%s", buffer);
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
 * Procédure appelée pour déplacer un fichier du serveur
 */
void deplacer(int socket_descriptor) {
    char buffer[BUFFER_SIZE];
    int longueur;
    char temp[PATH_LIMIT];

    puts("Indiquer le fichier à déplacer puis, séparé d'un espace, sa destination.");
    scanf("%s %s", buffer, temp);
    strcat(buffer, " ");
    strcat(buffer, temp);
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
                strcpy(buffer, "1\0");
                if ((write(socket_descriptor, buffer, strlen(buffer))) < 0) {
                    perror("erreur : impossible d'ecrire le message destine au serveur.");
                    break;
                }
                envoyer(socket_descriptor);
                break;

            case 2:
                strcpy(buffer, "2\0");
                if ((write(socket_descriptor, buffer, strlen(buffer))) < 0) {
                    perror("erreur : impossible d'ecrire le message destine au serveur.");
                    break;
                }
                recuperer(socket_descriptor);
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
