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

#include "constantes.h" //pour nos constantes générales prédéfinies

//constantes propres au serveur
#define TAILLE_MAX_NOM 256
#define NB_CONNEXIONS 5


typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

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

void * handler(void *socket_desc) {
    int sock = *(int *) socket_desc;
    int longueur;
    char client_buffer[BUFFER_SIZE];

    //Receive a message from client
    while( (longueur = read(sock, client_buffer, sizeof(client_buffer))) > 0 )
    {
    
        //end of string marker
        client_buffer[longueur] = '\0';
    
        //Send the message back to client
        write(sock , client_buffer , strlen(client_buffer));
    
        //clear the message buffer
        memset(client_buffer, 0, sizeof(client_buffer));
    }

    if(longueur == 0)
    {
        puts("Client déconnecté.");
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
    
    gethostname(machine,TAILLE_MAX_NOM);		/* recuperation du nom de la machine */
    
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
    // longueur_adresse_courante = sizeof(adresse_client_courant);
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

        // les actions du client sont gérées par un code à part : le handler
        if( pthread_create( &client_thread , NULL , handler , (void*) &nouv_socket_descriptor) < 0)
        {
            perror("erreur : impossible de créer le thread.");
            exit(1);
        }
        pthread_detach(client_thread);//pas besoin d'attendre la fin du thread
		
    }

    // close(nouv_socket_descriptor);//plus besoin, fait dans le handler
    close(socket_descriptor);
    return 0;
    
}
