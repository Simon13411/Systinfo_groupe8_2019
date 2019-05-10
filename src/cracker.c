#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include "reverse.h"
#include "sha256.h"
#include "sha256.c"
#include "reverse.c"
// Initialisation
#define N 100 // slots du buffer
#define cons "-c"
#define nbthread "-t"
#define output "-o"
int Fini =0;//indique si tout les fichier ont ete lu
int consonne = 1;
pthread_mutex_t mutex;//protege le buffer
pthread_mutex_t lettre;//protege stack de fin
sem_t empty;
sem_t full; //2 sem pour le probleme producteur consommateur
u_int8_t* tab[N]={};//notre buffer
int nbfiles;//nombre de fichiers
int placetab=0;
int maxmdp=0;//max de consonnne ou voyelle dans le mot de passe
int out=0;//si il faut fichier de sortie
char *fileOutput;

typedef struct node {
    struct node *next;
    char *name;
    int nbcons;
}node_t; //struct de notre list chaine avec les mdp decrypter
node_t *head;
node_t *init(char* nom,int voy){//creation d un noeud de la chaine
  node_t* new=(node_t*)malloc(sizeof(node_t));
  if(new==NULL){
    return NULL;
  }
  char* val=(char*)malloc(strlen(nom)+1);
  if (val==NULL){
    free(new);
    return NULL;
  }
  int *nb=(int*)malloc(sizeof(int));
  if(nb==NULL){
    free(new);
    free(val);
    return NULL;
  }
  *nb=voy;
  new->name=strcpy(val,nom);
  new->nbcons=*nb;
  new->next=NULL;
  return new; //on retourne le noeud qui a ete cree
}
//Initialisation des diverse variale inter thread afin d eviter de les passer par des structure

void insert_item(u_int8_t* ajout){  //ajout d un bloc de 32 bit dans le buffer tab
  tab[placetab]=ajout;
  placetab++;
}
//lit un des fichiers donné en arguments et les sépare en groupe de 32 bytes
//qu il met dans le buffer de taille "N"
void* producteur(void* nomfile){
  int err;
  char* fileName= (char*) nomfile;
  int fileReader =open(fileName,O_RDONLY);//ouverture du fichier
  if(fileReader==-1){
    printf("pas open file fileName= %s\n",fileName);
    return ((void*)-1);  //gerer dans le main en cas d errreur -1 probleme dopen
  }
  size_t taille = sizeof(u_int8_t);
  int read1=1;
  while(read1>0){//tant que le fichier n est pas fini
    u_int8_t* ptr=(u_int8_t*) malloc(taille*32);//mot de passe crypter
    if (ptr==NULL){
      printf("ton malloc a fail \n");
      return ((void*)-2);//malloc fail
    }
    for(int j=0;j<32;j++){//on lit les 32 bytes
      u_int8_t bit;
      read1 = read(fileReader,(void*)&bit,taille);//1 byte par 1 byte
      if(read1==-1){
        return ((void*)-3); //probleme de read
      }
      if(read1==0){ // fin du fichier
        int fermer = close(fileReader);
        if(fermer==-1){
          return ((void*)-4); //probleme de close
        }
        return ((void*)-5); // moins 32 bytes
      }
      *(ptr+j)=bit;//on mets les byte les 1 a la suite des autres
  }
  sem_wait(&empty);  // attente d'un slot libre
  err=pthread_mutex_lock(&mutex);  // section critique
  if (err!=0){
    printf("mutex_mutex dans producteur fail lock\n");
  }
  insert_item(ptr); //ajout dans le tableau
  err=pthread_mutex_unlock(&mutex);
  if (err!=0){
    printf("mutex_mutex dans producteur fail unlock\n");
  }
  sem_post(&full);  // il y a un slot rempli en plus
 }
 int fermer = close(fileReader);
 if(fermer==-1){
  return ((void*)-6);
 }
 return ((void *)0); // tout est ok tout c est passe correctement
}


int nbconsvoy(char* mdp){
  int count=0;
  if (mdp==NULL){
    return -8;
  }
    int i=0;
    while (*(mdp+i)!='\0') {//regarde le nombre de voyelle
      if (*(mdp+i)=='a' || *(mdp+i)=='e' || *(mdp+i)=='i' || *(mdp+i)=='o' || *(mdp+i)=='u' || *(mdp+i)=='y'){
        count++;
      }
      i++;
    }
    if(consonne==0){//alors on doit compter les consonne pas les voyelle
      return(i-count);
    }
return count;//return les voyelle
}



//il s'agit des thread qui vont prendre les 32 bytes et les decrypter
void* consommateur(){
  int err;
  u_int8_t *item;//mot de passe crypter

  sem_wait(&full);// attente d'un slot rempli pour l empecher de bloquer le producteur au debut
  err=pthread_mutex_lock(&mutex);// section critique a cause du tableau
  if (err!=0){
    printf("mutex_mutex dans consommateur fail lock\n");
  }
  while (Fini!=1 || placetab!=0) {// si place tab ==0 va poser pb avoir valeur d'un sem
    placetab--;
    item=tab[placetab];
    err=pthread_mutex_unlock(&mutex);
    if (err!=0){
      printf("mutex_mutex dans consommateur fail unlock\n");
    }
    if (Fini!=1){//car si fini est egal a 1 il n y a plus de producteur donc on doit juste vide le tableau
      sem_post(&empty);// il y a un slot libre en plus
    }
    char* mdp=(char*) malloc(sizeof(char)*16);//futur mot de passe decrypter
    if (mdp==NULL){
      printf("ton malloc a fail en char \n");
      return ((void*)-7);//malloc fail
    }
    bool boole;
    boole=reversehash(item,mdp,sizeof(char)*16);//appelle reversehash
    free(item);//on libere le mot de pass crypter
    if (!boole){
      printf("reversehash fail\n");
      return ((void*)-8);//reversehash fail
    }
    int nbconsvoye=nbconsvoy(mdp);//on regarde le nombre de consonne ou voyelle
    err=pthread_mutex_lock(&lettre);// section critique de mise en stack
    if (err!=0){
      printf("mutex_lettre fail lock\n");
    }
    if(maxmdp<=nbconsvoye){//rajoute a la stack suelement si il y a autant ou plus de consonne ou voyelle
      node_t *new=init(mdp,nbconsvoye);//creation du node
      if(new==NULL){
        pthread_mutex_unlock(&lettre);//on libere la stack
        if (err!=0){
          printf("mutex_lettre fail lock\n");
        }
        return ((void*)9);
      }
      if (head==NULL){
        //free(new->nbcons);
        free(new->name);
        free(new);
        pthread_mutex_unlock(&lettre);//on libere la stack
        if (err!=0){
          printf("mutex_lettre fail lock\n");
        }
        return ((void*)10);
      }
      new->next=head;
      head=new;//on rajoute le noeud
      if(maxmdp<nbconsvoye){
        maxmdp=nbconsvoye;//si le nombre de voyelle ou consonne ets plus grand que le dernier plus grand
        //on lui donne alors la plus haute valeur
      }
    }
    free(mdp);//on libere mdp car il a ete malloc avec la stack
    pthread_mutex_unlock(&lettre);//on libere la stack
    if (err!=0){
      printf("mutex_lettre fail lock\n");
    }
    if(Fini!=1){//car si fini est egal a 1 il n y a plus de producteur donc on doit juste vide le tableau
      sem_wait(&full);// attente d'un slot rempli
    }
    err=pthread_mutex_lock(&mutex);// section critique
    if (err!=0){
      printf("mutex_mutex dans consommateur fail lock\n");
    }
    //les condition du while
  }
  err=pthread_mutex_unlock(&mutex);//on unlock les mutex si les condition ont ete verifer
  if (err!=0){
    printf("mutex_mutex dans consommateur fail unlock\n");
  }
  //sem_post(&empty);// il y a un slot libre en plus
  return (void*)0;
}


int main(int argc, char *argv[]) {
  int maxThread = 1;
  int err;
  int j=0;
  char *files[argc];//tableau avec nos fichier
  nbfiles = argc-1; //car les arg contienne nom fichier donc -1 pour le nom
  for(int i=1;i<argc; i++){
    if(strcmp(*(argv+i),cons)==0){  //regarde si on demande de verifier pour les consonnes(==true) ou les voyelle(==false)
      consonne = 0;
      nbfiles--;//car on considere que tout les argv sont des files donc on decremente
    }
    else if(strcmp(*(argv+i),nbthread)==0){  //regarde le nombre de threads utilise
      i++;
      nbfiles--;
      nbfiles--;//car on considere que tout les argv sont des files donc on decremente
      if ((atoi(*(argv+i)))>1){
        maxThread = atoi(*(argv+i));
      }
    }
    else if(strcmp(*(argv+i), output)==0){
      out=1;
      i++;
      fileOutput = *(argv+i);
      nbfiles=nbfiles-2;//car on considere que tout les argv sont des files donc on decremente
    }//si pas -c ou -t alors c'est un fichier
    else{ //ecrit dans un tableau le nom des fichiers
      *(files+j) = *(argv+i);
      j++;
    }
  }

  err=pthread_mutex_init(&mutex, NULL);
  if (err!=0){
    printf("mutex_mutex fail Initialisation\n");
  }
  err=pthread_mutex_init(&lettre, NULL);
  if (err!=0){
    printf("mutex_lettre fail Initialisation\n");
  }
  //Initialisation de tout nos mutex
  head=(node_t*)malloc(sizeof(node_t));//on malloc notre head
  head->next=NULL;
  head->name=NULL;
  head->nbcons=0;//on lui donne quelque valeur
  sem_init(&empty,0,N); // buffer vide
  sem_init(&full,0,0);  // buffer vide
  pthread_t threadfil[nbfiles];
  for(long i=0;i<nbfiles;i++){
    err=pthread_create(&(threadfil[i]),NULL,&producteur,(void*)files[i]);
    if(err!=0){
      printf("errreur create producteur\n");
    }
  }//cree pthread producteur
  pthread_t thread[maxThread];
  for(long i=0;i<maxThread;i++){
    err=pthread_create(&(thread[i]),NULL,&consommateur,NULL);
    if(err!=0){
      printf("errreur create consommateur\n");
    }
  }//cree pthread consommateur

  for(long i=0;i<nbfiles;i++){
    err=pthread_join(threadfil[i],NULL);
    if(err!=0){
      printf("errreur pthread_join consommateur\n");
    }
  }//join pthread producteur
  Fini=1;
  for(long i=0;i<maxThread;i++){
    err=pthread_join(thread[i],NULL);
    if(err!=0){
      printf("errreur pthread_join consommateur\n");
    }
  }//join pthread consommateur
  err=pthread_mutex_destroy(&mutex);
  if(err!=0){
    printf("errreur destroy mutex\n");
  }  //!!!!!!!!!!! on doit détruire mutex et semaphore

  err=pthread_mutex_destroy(&lettre);
  if(err!=0){
    printf("errreur destroy lettre\n");
  }

  err=sem_destroy(&empty);
  if(err!=0){
    printf("errreur destroy sem empty\n");
  }
  err=sem_destroy(&full);
  if(err!=0){
    printf("errreur destroy sem full\n");
  } //on detruit toute nos mutex et semaphore
  node_t *runner=head;//on va parcourir notre stack
  int openfile=0;
  if(out==1){//si il est egal a 1 alors on a un file de sortie
    openfile = open(fileOutput, O_WRONLY | O_CREAT| O_TRUNC,S_IRWXU);
    if(openfile==-1){
      printf("pas reussi a aller dans le ficheir\n");
    }
  }
  while (runner->nbcons==maxmdp) {//tant que c est ceux avec le plus de voyelle ou consonne
    if(out!=1){
      printf("%s\n",runner->name);//mot de passe decrypter
    }
    else{
      char * str = strcat((char *) runner->name, "\n");
      write(openfile,str, strlen(runner->name));//ecriture dans fichier
        /*if(writefile==-1){
          printf("Erreur d ecriture dans file\n");
        }*/
    }
      runner=runner->next;//on passe au suivant
  }
  if(out==1){//fermeture du fichier
    int fermer = close(openfile);
    if(fermer==-1){
      printf("Erreur de fermeture\n");
    }
  }
  node_t *run=head;
  while (head!=NULL){
    run=head;
    head=run->next;
    free(run->name);
    //free(run->nbcons);
    free(run);
  }
}
