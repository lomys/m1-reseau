/*-----------------------------------------------------------

------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#define TAILLE_MAX 1024

typedef struct sockaddr 	sockaddr;
typedef struct sockaddr_in 	sockaddr_in;
typedef struct hostent 		hostent;
typedef struct servent 		servent;


char *	lire_fichier(char * path){
    FILE* fichier = NULL;
    fichier = fopen(path,"rb");
    int caractereActuel = 0;
    char * res ;
    char chaine[TAILLE_MAX];
    int m,position_actuel;
    if (fichier != NULL){
         // On lit le fichier tant qu'on ne reçoit pas d'erreur (NULL)
        
        while(1) 
        {
            //position_actuel = ftell(fichier);
            //m = ftell(fichier);
            /*if(m > TAILLE_MAX){
                fseek(fichier,TAILLE_MAX,position_actuel);
                m = ftell(fichier);
                res = malloc((sizeof(char)) * m);
                fseek(fichier, 0, SEEK_SET);
                fread(res, m, 1, fichier);
                printf("%s\n",res );

            }else*/
                fseek(fichier, 0L, SEEK_END);

                m = ftell(fichier);
                res = malloc((sizeof(char)) * m);
                printf("%d\n",m);
                fseek(fichier, 0, SEEK_SET);
                fread(res, m, 1, fichier);
                printf("%s\n","test");
                printf("%s\n",res);
            
            if(feof(fichier)){break;}
            
            //op1[m]='/0';
            //send(sd, res, m, 0);
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
