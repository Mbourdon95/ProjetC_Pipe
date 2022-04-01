#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>

#define BUFFER_SIZE 256
#define MSG1 "Paiement securise"
#define MSG2 "Accuse de reception"
#define MSG3 "Bon de livraison"

/* reverse:  reverse string s in place */
 void reverse(char s[])
 {
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}  

int main(void){
    
    pid_t pid_Antoine, pid_sw1, pid_Jule;
    int AchToSw[2], SwToAch[2], SwToTran[2], AchToTran[2], TranToAch[2];
    char buffer[BUFFER_SIZE],bon[BUFFER_SIZE] = {" "}, text[10];
    double bufferDouble, bufferDoubble;
    int bufferInt;
    
    char article[][16] = {"creux", "plein"};
    double surface = 48.0 ,surfaceParpaing = 0.1,nbParpaingPalette = 50.0, prixPalette = 13.0;
    double quantiteEnStock = 100.0, nbPaletteDouble, prix;
    double nbPaletteInt;
    int codeCB = 514953, cryptogramme = 764;
    char *signature = "signé";

    // étape 1 
    srand(time(NULL));
    int searchedValue = (rand()%(sizeof(article)/sizeof(*article)));
    char * article_Choisi = article[searchedValue]; // printf("%s", article_Choisi); 
    // étape 1 
    
    /* INITIALISATION DES TUBES */
    if(pipe(AchToSw) != 0){perror("Erreur creation tube\n");exit(-1);}
    if(pipe(SwToAch) != 0){perror("Erreur creation tube\n");exit(-1);}
    if(pipe(SwToTran) != 0){perror("Erreur creation tube\n");exit(-1);}
    if(pipe(AchToTran) != 0){perror("Erreur creation tube\n");exit(-1);}
    if(pipe(TranToAch) != 0){perror("Erreur creation tube\n");exit(-1);}

    pid_Antoine = fork();
    if(pid_Antoine < 0){perror("Erreur creation processus\n"); exit(-1);}
    else if(pid_Antoine == 0){          /* ACHETEUR */
        printf("Je suis le processus Acheteur Antoine\nMon PID est : %d et mon père est : %d\n", getpid(), getppid());
        close(AchToSw[0]);  //On ferme le tube en lecture. 
        write(AchToSw[1], article_Choisi, sizeof(article_Choisi)); // On envoit dans le tube l'article choisi.
        //close(tube[1]);

        close(SwToAch[1]);  //On ferme le tube en écriture.
        read(SwToAch[0], &bufferDouble, sizeof(double)); // pid_Antoine reçoit la surface disponible.
        //close(SwToAch[0]);
        printf("Antoine a reçu du Serveur : 'quantite en stock : %lf m2'\n",bufferDouble);
        sleep(1);
        // Calcul de la surface demandée par l'acheteur. 
        double b = ((double)rand() / RAND_MAX * (surface - 0.1) + 0.1);
        surface = (double)((int)(b*10))/10; 
        write(AchToSw[1], &surface, sizeof(double)); // On envoit dans le tube la surface choisie.
        read(SwToAch[0], &bufferDoubble, sizeof(double)); // On reçoit le nombre de palette
        printf("Antoine a reçu du Serveur : 'Nombre de palette necessaire : %lf palettes'\n",bufferDoubble);
        read(SwToAch[0], &bufferDouble, sizeof(double)); // On reçoit le prix total des palettes.
        printf("Antoine a reçu du Serveur : 'Prix : %lf euros'\n",bufferDouble);
        
        sleep(1);
        //write(tube[1], MSG1, strlen(MSG1));
        write(AchToSw[1], &codeCB, sizeof(int)); // On envoit le codeCB
        write(AchToSw[1], &cryptogramme, sizeof(int)); // On envoit le cryptogramme
        close(AchToSw[1]); // On ferme le tube en écriture. 

        read(SwToAch[0], buffer, BUFFER_SIZE); // On reçoit l'accusé de reception
        printf("Antoine a reçu du Serveur : '%s, Prix total : %lf euros'\n",buffer, bufferDouble); // On affiche l'accusé + prix total. 
        close(SwToAch[0]); // On ferme le tube en lecture.

        close(TranToAch[1]); // On ferme le tube en écriture.
        read(TranToAch[0], &buffer, BUFFER_SIZE); // On reçoit le bon de livraison avec le nb de palettes
        printf("Antoine a reçu de Jule : '%s'\n", buffer);
        close(TranToAch[0]); // On ferme le tube en lecture. 
        buffer[strlen(buffer)] = ' ';
        strcat(bon, buffer);
        strcat(bon, "signe");

        close(AchToTran[0]); // On ferme le tube en lecture. 
        write(AchToTran[1], bon, BUFFER_SIZE); // On envoit un bon signé. 
        close(AchToTran[1]); // On ferme le tube en écriture. 
        exit(0);
    }
    else{        
        pid_sw1 = fork();
        if(pid_sw1 < 0){perror("Erreur création processus SERVEUR\n");exit(-1);}
        else if(pid_sw1 == 0){          /* SERVEUR WEB */
            printf("Je suis le processus Serveur web\nMon PID est %d et mon père est : %d\n", getpid(),getppid());
            close(AchToSw[1]); // On ferme le tube en écriture. 
            read(AchToSw[0], buffer, BUFFER_SIZE); // On récupère l'article choisi par Antoine. 
            printf("Le Serveur a reçu d'Antoine : 'Article choisi : Article %s'\n", buffer); //close(tube[0]); 

            close(SwToAch[0]); //On ferme le tube en lecture.
            write(SwToAch[1], &quantiteEnStock, sizeof(double)); // On envoit la surface disponible. 
            //close(tube1[1]);
            
            read(AchToSw[0], &bufferDouble, sizeof(double)); // On reçoit la surface demandée par Antoine.
            printf("Le Serveur a reçu d'Antoine : 'Surface demandee : %lf m2'\n", bufferDouble);

            // Calcul du nombre de palettes  de  parpaings correspondant  à  la  surface  en  m2 demandée par l'acheteur
            nbPaletteDouble = ((surface * (1/surfaceParpaing)) / nbParpaingPalette);
            prix = nbPaletteDouble * prixPalette;
            
            write(SwToAch[1], &nbPaletteDouble, sizeof(double)); // On envoit le nombre de palette.
            write(SwToAch[1], &prix, sizeof(double)); // On envoit prix total. 
            read(AchToSw[0], &bufferInt, sizeof(int)); // On reçoit le codeCB.
            printf("Le Serveur a reçu d'Antoine : 'Code CB : %d'\n", bufferInt);
            read(AchToSw[0], &bufferInt, sizeof(int)); // On reçoit le Cryptogramme.
            printf("Le Serveur a reçu d'Antoine : 'Cryptogramme : %d'\n", bufferInt);
    
            write(SwToAch[1], MSG2, strlen(MSG2)); // On envoit l'accusé de reception 
            sleep(1);
            close(SwToTran[0]);
            write(SwToTran[1], MSG3, strlen(MSG3)); // On envoit le Bon de livraison 
            sleep(1);
            
            write(SwToTran[1], &nbPaletteDouble, sizeof(double)); // On envoit le nb de palette à Jule
            close(SwToTran[1]); // on ferme le tube en écriture.
            exit(0);
        }
        else{
            pid_Jule = fork();

            if(pid_Jule < 0){perror("Erreur creation processus\n");exit(-1);}
            else if(pid_Jule == 0){         /* TRANSPORTEUR JULE */
                printf("Je suis le processus Transporteur\nMon PID est %d et mon père est : %d\n", getpid(), getppid());
                close(SwToTran[1]); //On ferme le tube en écriture.
                read(SwToTran[0], buffer, BUFFER_SIZE); // On reçoit Le bon de livraison  
                read(SwToTran[0], &bufferDoubble, sizeof(double)); // On reçoit le nombre de palette.
                printf("Le Transporteur reçoit : '%s, Nombre de palette : %lf palettes'\n", buffer, bufferDoubble);
                sprintf(text,"%f",bufferDoubble);
                strcat(bon, buffer);
                strcat(bon, ", Prix de la Transaction : ");
                strcat(bon, text);
                close(SwToTran[0]); // On ferme le tube en lecture.
                sleep(1);
                close(TranToAch[0]); // On ferme le tube en lecture.
                
                write(TranToAch[1], bon, BUFFER_SIZE); // On envoit le bon à l'acheteur. 
                close(TranToAch[1]); // On ferme le tube en écriture.

                close(AchToTran[1]); // On ferme le tube en écriture. 
                read(AchToTran[0], buffer, BUFFER_SIZE); // on reçoit le bon signé par Antoine. 
                close(AchToTran[0]); // On ferme le tube en lecture.
                printf("Le Transporteur a reçu d'Antoine : 'Bon signé : %s'\n",buffer);
                exit(0);
            }
            else{
                /* PROCESSUS PERE */
                waitpid(pid_Antoine, NULL, 0);
                waitpid(pid_sw1, NULL, 0);
                waitpid(pid_Jule, NULL, 0);
                exit(0);
            }
        }
    }
}