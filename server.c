/*----------------------------------------------
Serveur à lancer avant le client
------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h> // #include <linux/types.h> 	/* pour les sockets */
#include <sys/socket.h>
#include <netdb.h> 		/* pour hostent, servent */
#include <string.h> 		/* pour bcopy, ... */  
#include <pthread.h>    /* pour les threads */
#include <unistd.h>     /* pour fonctions sleep, read, write, close */
#include <arpa/inet.h>  /* pour fonction inet_ntoa */

#include "constantes.h" //pour nos constantes générales prédéfinies

//constantes propres au serveur
#define TAILLE_MAX_NOM 256
#define NB_CONNEXIONS 5 //ne devrait pas dépasser 5


typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;
typedef struct {
    int * socket_desc;
    char * adresse_ip;
} thread_params;

/* signatures des méthodes */
void renvoi(int sock); 
void * handler(void *socket_desc); // gestionnaire de clients après connexion au serveur

/*------------------------------------------------------*/
void renvoi (int sock) {

    char buffer[256];
    int longueur;
   
    if ((longueur = read(sock, buffer, sizeof(buffer))) <= 0) {
    	printf("erreur longueur du buffer");
        return;
    }
    
    printf("message lu : %s \n", buffer);
    
    buffer[0] = 'R';
    buffer[1] = 'E';
    buffer[longueur] = '#';
    buffer[longueur+1] ='\0';
    
    printf("message apres traitement : %s \n", buffer);
    
    printf("renvoi du message traite.\n");

    /* mise en attente du prgramme pour simuler un delai de transmission */
    sleep(3);
    
    // strcat(buffer,system("ls"));
    // longueur = sizeof(buffer);
    // buffer[longueur+1] ='\0';
    write(sock,buffer,strlen(buffer)+1);
    
    printf("message envoye. \n");
        
    return;
    
}
/*------------------------------------------------------*/

void * handler(void *args) {
    thread_params * params = args;
    int sock = *(int *) params->socket_desc;
    char * adresse_ip = (char *) params->adresse_ip;
    int longueur;
    char client_buffer[BUFFER_SIZE];
    int status; //statut pour la fonction popen
    char path[PATH_LIMIT];

    printf("Nouveau client : %s\n", adresse_ip);

    //Receive command from client
    while( (longueur = read(sock, client_buffer, sizeof(client_buffer))) > 0 )
    {
        //end of string marker
        // client_buffer[longueur] = '\0';
        //traite la commande
        if(strcmp(client_buffer, "1") == 0) { //réception d'un fichier
            printf("\nRéception d'un fichier envoyé par %s\n", adresse_ip);
            memset(client_buffer, 0, sizeof(client_buffer));

        } else if(strcmp(client_buffer, "2") == 0) { //envoi d'un fichier
            printf("\nEnvoi d'un fichier à %s\n", adresse_ip);
            memset(client_buffer, 0, sizeof(client_buffer));

        } else if(strcmp(client_buffer, "3") == 0) { //rm
            printf("\nSuppression d'un fichier par %s\n", adresse_ip);
            memset(client_buffer, 0, sizeof(client_buffer));

            //lecture du path
            read(sock, path, sizeof(path));

            //exécution de la commande
            char cmd[PATH_LIMIT+7];
            strcpy(cmd, "rm -fr ");
            strcat(cmd, path);
            printf("%s\n", cmd);
            system(cmd);
            strcpy(client_buffer, "Fichier(s) supprimé(s).");
            
            //envoi du résultat au client
            write(sock, client_buffer, strlen(client_buffer));   

        } else if(strcmp(client_buffer, "4") == 0) { //ls
            printf("\nExécution de la commande 'ls' par %s\n", adresse_ip);
            memset(client_buffer, 0, sizeof(client_buffer));
            
            //lecture du path
            read(sock, path, sizeof(path));
            // printf("path : %s\n", path);

            //exécution de la commande
            FILE * fp;
            char pipe[BUFFER_SIZE];
            char cmd[PATH_LIMIT+6];
            strcpy(cmd, "ls -lh ");
            strcat(cmd, path);
            printf("%s\n", cmd);
            fp = popen(cmd, "r");//la fonction popen permet de récupérer l'output de la commande grâce à un pipe
            if(!fp) {
                strcpy(client_buffer, "error : impossible de lister les fichiers\0");
                perror("error : impossible de lister les fichiers");
            } else {
                //lecture du pipe
                while( fgets(pipe, BUFFER_SIZE, fp) != NULL ){
                    strcat(client_buffer, pipe);
                }
                status = pclose(fp);
                if(status > 0) {
                    strcpy(client_buffer, "error : impossible de lister les fichiers\0");
                    perror("error : impossible de lister les fichiers");
                }
            }
            
            //envoi du résultat au client
            write(sock, client_buffer, strlen(client_buffer));

        } else if(strcmp(client_buffer, "5") == 0) { //mkdir
            printf("\nCréation d'un dossier par %s\n", adresse_ip);
            memset(client_buffer, 0, sizeof(client_buffer));

            //lecture du path
            read(sock, path, sizeof(path));

            //exécution de la commande
            char cmd[PATH_LIMIT+9];
            strcpy(cmd, "mkdir -p ");
            strcat(cmd, path);
            printf("%s\n", cmd);
            system(cmd);
            strcpy(client_buffer, "Répertoire créé.");
            
            //envoi du résultat au client
            write(sock, client_buffer, strlen(client_buffer));   

        } else if(strcmp(client_buffer, "6") == 0) { //mv
            printf("\nDéplacement d'un fichier par %s\n", adresse_ip);
            memset(client_buffer, 0, sizeof(client_buffer));

        } else  {
            printf("Client déconnecté : %s\n", adresse_ip);
            break;
        }
        //clear the message buffer
        memset(client_buffer, 0, sizeof(client_buffer));
        memset(path, 0, sizeof(path));
    }

    if(longueur == 0)
    {
        printf("Client déconnecté : %s\n", adresse_ip);
        fflush(stdout);
    }
    else if(longueur == -1)
    {
        perror("erreur : impossible de lire les messages.");
    }
    close(sock);
    
    pthread_exit(0);

}

/*------------------------------------------------------*/
int main(int argc, char **argv) {
  
    int 		socket_descriptor, 		/* descripteur de socket */
			nouv_socket_descriptor, 	/* [nouveau] descripteur de socket */
			longueur_adresse_courante; 	/* longueur d'adresse courante d'un client */
    sockaddr_in 	adresse_locale, 		/* structure d'adresse locale*/
			adresse_client_courant; 	/* adresse client courant */
    hostent*		ptr_hote; 			/* les infos recuperees sur la machine hote */
    servent*		ptr_service; 			/* les infos recuperees sur le service de la machine */
    char 		machine[TAILLE_MAX_NOM+1]; 	/* nom de la machine locale */
    pthread_t client_thread;            //thread créé pour un client
    thread_params params;               //paramètres du thread
    char adresse_ip[INET_ADDRSTRLEN];   //adresse ip du client
    
    
    gethostname(machine,TAILLE_MAX_NOM);		/* recuperation du nom de la machine */
    //TODO : get real ip and print it
    
    /* recuperation de la structure d'adresse en utilisant le nom */
    if ((ptr_hote = gethostbyname(machine)) == NULL) {
		perror("erreur : impossible de trouver le serveur a partir de son nom.");
		exit(1);
    }
    
    /* initialisation de la structure adresse_locale avec les infos recuperees */			
    
    /* copie de ptr_hote vers adresse_locale */
    bcopy((char*)ptr_hote->h_addr, (char*)&adresse_locale.sin_addr, ptr_hote->h_length);
    adresse_locale.sin_family		= ptr_hote->h_addrtype; 	/* ou AF_INET */
    adresse_locale.sin_addr.s_addr	= INADDR_ANY; 			/* ou AF_INET */

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
    /* SOLUTION 2 : utiliser un nouveau numero de port */
    //adresse_locale.sin_port = htons(5000);
    /*-----------------------------------------------------------*/
    
    printf("numero de port pour la connexion au serveur : %d \n", 
		   ntohs(adresse_locale.sin_port) /*ntohs(ptr_service->s_port)*/);
    
    /* creation de la socket */
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("erreur : impossible de creer la socket de connexion avec le client.");
		exit(1);
    }

    /* association du socket socket_descriptor à la structure d'adresse adresse_locale */
    if ((bind(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
		perror("erreur : impossible de lier la socket a l'adresse de connexion.");
		exit(1);
    }
    
    /* initialisation de la file d'ecoute */
    if (listen(socket_descriptor, NB_CONNEXIONS) < 0) {
        perror("erreur : impossible d'écouter sur le socket défini");
        exit(1);
    }

    /* attente des connexions et traitement des donnees recues */
    longueur_adresse_courante = sizeof(adresse_client_courant);
    while(1) {

		/* adresse_client_courant sera renseigné par accept via les infos du connect */
		if ((nouv_socket_descriptor = accept(socket_descriptor, 
			                                 (sockaddr*)(&adresse_client_courant),
			                                 (socklen_t*) &longueur_adresse_courante)) < 0) 
        {
			perror("erreur : impossible d'accepter la connexion avec le client.");
			exit(1);
		}
        //récupération de l'addresse IPv4
        inet_ntop(AF_INET, &adresse_client_courant.sin_addr, adresse_ip, INET_ADDRSTRLEN);
        //initialisation des paramètres du thread
        params.socket_desc = &nouv_socket_descriptor;
        params.adresse_ip = &adresse_ip[0];
        // les actions du client sont gérées par un code à part : le handler
        if( pthread_create( &client_thread , NULL , handler , (void *) &params) < 0)
        {
            perror("erreur : impossible de créer le thread.");
            exit(1);
        }
        pthread_detach(client_thread);//pas besoin d'attendre la fin du thread
		
    }

    close(socket_descriptor);
    return 0;
    
}
