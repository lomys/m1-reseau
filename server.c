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
    int socket_desc;
    char adresse_ip[INET_ADDRSTRLEN];
} thread_params;

/* signatures des méthodes */
void * handler(void *socket_desc); // gestionnaire de clients après connexion au serveur
int main(int argc, char **argv); //fonction principale

/*------------------------------------------------------*/

void * handler(void *args) {
    thread_params * params = args;
    int sock = params->socket_desc;
    char adresse_ip[INET_ADDRSTRLEN];
    memcpy(adresse_ip, params->adresse_ip, INET_ADDRSTRLEN);
    int longueur;
    char client_buffer[BUFFER_SIZE];
    int status; //statut pour la fonction popen
    char path[PATH_LIMIT];
    char taille_fichier[BUFFER_SIZE];
    long tf;
    char * res;
    FILE* fichier= NULL;
    int pointeur_fichier;

    printf("Nouveau client : %s\n", adresse_ip);
    printf("%s\n", "En attente.");
    //Receive command from client
    while( (longueur = read(sock, client_buffer, sizeof(client_buffer))) > 0 )
    {
        //end of string marker
        // client_buffer[longueur] = '\0';
        //traite la commande
        if(strcmp(client_buffer, "1") == 0) { //réception d'un fichier
            pointeur_fichier = 0;
            printf("\nRéception d'un fichier envoyé par %s\n", adresse_ip);
            memset(client_buffer, 0, sizeof(client_buffer));
            //lecture du path où enregistrer le fichier
            read(sock, path, sizeof(path));
            char cmd[PATH_LIMIT+7];
            strcpy(cmd, "rm -fr ");
            strcat(cmd, path);
            system(cmd);
            fichier = fopen(path,"ab");
            //lecture de la taille du fichier enregistrer
            read(sock, taille_fichier, sizeof(taille_fichier));
            tf = atoi(taille_fichier);
            printf("Création %s de taille %d \n",path,tf);
            while(pointeur_fichier < tf){
                
                read(sock,client_buffer,sizeof(client_buffer));
                //printf("%s \n",client_buffer);
                res = client_buffer;
                pointeur_fichier = pointeur_fichier + strlen(res);
                //printf("%d \n",strlen(res));
                fwrite(res,1,strlen(res),fichier);
                memset(res, 0, strlen(res));
                
            }
            fclose(fichier);
            strcpy(client_buffer, "Fichier envoyé.");            
            //envoi du résultat au client
            write(sock, client_buffer, strlen(client_buffer));

        } else if(strcmp(client_buffer, "2") == 0) { //envoi d'un fichier
            printf("\nEnvoi d'un fichier à %s\n", adresse_ip);
            memset(client_buffer, 0, sizeof(client_buffer));
            char buffer[BUFFER_SIZE];
            //Récupération du path du fichier
            memset(path,0,sizeof(path));
            read(sock, path, sizeof(path));
            int m,position_actuel;
            int fin = 0;
            fichier = fopen(path,"rb");
            if (fichier != NULL){
                // On lit le fichier tant qu'on ne reçoit pas d'erreur (NULL)
                fseek(fichier, 0, SEEK_END);
                m = ftell(fichier);
                fseek(fichier, 0, SEEK_SET);
                sprintf(buffer, "%d", m);
                buffer[strlen(buffer)] = '\0';
                if ((write(sock, buffer, sizeof(buffer))) < 0) {
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
                    write(sock, res, strlen(res));
                    memset(res,0,strlen(res));
                }
                fclose(fichier);
            }else{
                printf("Impossible d'ouvrir le fichier.\n");
            }

        } else if(strcmp(client_buffer, "3") == 0) { //rm
            printf("\nSuppression d'un fichier par %s\n", adresse_ip);
            memset(client_buffer, 0, sizeof(client_buffer));

            //lecture du path
            read(sock, path, sizeof(path));

            //exécution de la commande
            char cmd[PATH_LIMIT+7];//le chiffre additionné correspond à la commande et ses paramètres (rm -rf )=7 char
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
            //printf("path : %s\n", path);

            //exécution de la commande
            FILE * fp;
            char pipe[10000];
            char buffer_pipe[10000];
            char cmd[PATH_LIMIT+7];
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
                    //hack pour éviter de dépasser la taille du buffer
                    if(strlen(client_buffer) + strlen(pipe) < BUFFER_SIZE ) {
                        strcat(client_buffer, pipe);
                    }
                }
                status = pclose(fp);
                if(status > 0) {
                    strcpy(buffer_pipe, "error : impossible de lister les fichiers\0");
                    perror("error : impossible de lister les fichiers");
                }
            }
            
            //envoi du résultat au client
            write(sock, client_buffer, sizeof(client_buffer));

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

            //lecture du path
            read(sock, path, sizeof(path));

            //exécution de la commande
            char cmd[PATH_LIMIT+5];
            strcpy(cmd, "mv -n ");
            strcat(cmd, path);
            printf("%s\n", cmd);
            system(cmd);
            strcpy(client_buffer, "Fichier déplacé.");
            
            //envoi du résultat au client
            write(sock, client_buffer, strlen(client_buffer));   

        } else  {
            printf("Client déconnecté : %s\n", adresse_ip);
            break;
        }
        //clear the message buffer
        memset(client_buffer, 0, sizeof(client_buffer));
        memset(path, 0, sizeof(path));
        printf("%s\n", "En attente.");
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
    char ip_serveur[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &adresse_locale.sin_addr, ip_serveur, INET_ADDRSTRLEN);
    printf("IP serveur (%s) : %s\n", machine, ip_serveur);
    
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

    char ip_serveur[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &adresse_locale.sin_addr, ip_serveur, INET_ADDRSTRLEN);
    printf("IP serveur (%s) : %s\n", machine, ip_serveur);

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
        //récupération de l'adresse IPv4
        inet_ntop(AF_INET, &adresse_client_courant.sin_addr, adresse_ip, INET_ADDRSTRLEN);
        //initialisation des paramètres du thread
        memcpy(params.adresse_ip, adresse_ip, INET_ADDRSTRLEN);
        params.socket_desc = nouv_socket_descriptor;
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
