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
    FILE* fichier= NULL;
    FILE* fp= NULL;
    fp = fopen("file.txt","a");
    fichier = fopen(path,"rb");
    int caractereActuel = 0;
    char * res ;
    char chaine[TAILLE_MAX];
    int m,position_actuel;
    int fin = 0;
    if (fichier != NULL){
         // On lit le fichier tant qu'on ne reçoit pas d'erreur (NULL)
        
        while(fin == 0) 
        {
            position_actuel = ftell(fichier);
            printf("position_actuel : %d\n",position_actuel);
            fseek(fichier, 0, SEEK_END);
            m = ftell(fichier);
            fseek(fichier, 0, SEEK_SET);
            
            if(m - position_actuel > TAILLE_MAX){
                printf("%s\n","boucle 1");
                fseek(fichier,position_actuel,SEEK_CUR);
                res = malloc((sizeof(char)) * position_actuel+TAILLE_MAX);
                position_actuel = position_actuel + TAILLE_MAX;
                printf("position_actuel : %d\n",position_actuel);
                printf("position final : %d\n",m);
                //fseek(fichier, position_actuel, SEEK_CUR);
                fread(res, TAILLE_MAX, 1, fichier);
                printf("affichage : %s\n",res);
                printf("pos : %d\n",ftell(fichier));

            }else{
                printf("%s\n","boucle 2");
                fin = 1;         
                res = malloc((sizeof(char)) * m-position_actuel);
                printf("%d\n",m-position_actuel);
                fseek(fichier, position_actuel, SEEK_CUR);
                fread(res, m-position_actuel, 1, fichier);
                printf("Affichage : %s\n",res);
                fwrite(res,1,m-position_actuel,fp);
                printf("%s\n","test");
                
            }
            //sleep(2);
            //op1[m]='/0';
            //send(sd, res, m, 0);
        }
        fclose(fp);
        fclose(fichier);
    }else{
        printf("Impossible d'ouvrir le fichier.\n");
    }
    return "";
} 

int main(int argc, char **argv) {
  
    printf("%s\n",lire_fichier("fichier.txt"));
    exit(0);
    
}
