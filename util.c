/*-----------------------------------------------------------

------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

typedef struct sockaddr 	sockaddr;
typedef struct sockaddr_in 	sockaddr_in;
typedef struct hostent 		hostent;
typedef struct servent 		servent;


char *	lire_fichier(char * path){
    FILE* fichier = NULL;
    fichier = fopen(path,"r");
    int caractereActuel = 0;
    char * res ;
    const char * valeur;
    if (fichier != NULL){
        do
        {
            caractereActuel = fgetc(fichier); // On lit le caractère
            valeur = (char)caractereActuel;
            strcpy(res,"test");
            strcat(res, valeur);// On le place dans le buffer
        } while (caractereActuel != EOF); // On continue tant que fgetc n'a pas retourné EOF (fin de fichier)
 
        fclose(fichier);
    }else{
        printf("Impossible d'ouvrir le fichier.");
    }
    return res;
} 

int main(int argc, char **argv) {
  
    printf("%s",lire_fichier("fichier.txt"));
    exit(0);
    
}
