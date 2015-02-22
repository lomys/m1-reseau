/*-----------------------------------------------------------

------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#define TAILLE_MAX 10

typedef struct sockaddr 	sockaddr;
typedef struct sockaddr_in 	sockaddr_in;
typedef struct hostent 		hostent;
typedef struct servent 		servent;


char *	lire_fichier(char * path){
    FILE* fichier = NULL;
    fichier = fopen(path,"r");
    int caractereActuel = 0;
    char * res ="" ;
    char chaine[TAILLE_MAX] = "";
    int i = 1;
    if (fichier != NULL){
        while (fgets(chaine, TAILLE_MAX, fichier) != NULL) // On lit le fichier tant qu'on ne reçoit pas d'erreur (NULL)

        {

            printf("%d %s\n", i,chaine); // On affiche la chaîne qu'on vient de lire
            i++;
        }
 
        fclose(fichier);
    }else{
        printf("Impossible d'ouvrir le fichier.\n");
    }
    return res;
} 

int main(int argc, char **argv) {
  
    printf("%s\n",lire_fichier("fichier.txt"));
    exit(0);
    
}
