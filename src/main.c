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
int Fini =0;
pthread_mutex_t mutex;
pthread_mutex_t lettre;
pthread_mutex_t place;
sem_t empty;
int consonne = 1;
sem_t full;
u_int8_t* tab[N]={};
int nbfiles;
int placetab=0;
int maxmdp=0;

typedef struct node {
    struct node *next;
    char *name;
    int nbcons;
}node_t;
node_t *head;
node_t *init(char* nom,int voy){
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
  return new;
}
//(struct node**)malloc(sizeof(struct node**));
//Initialisation des diverse variale inter thread afin d eviter de les passer par des structure

void insert_item(u_int8_t* ajout){  //ajout d un bloc de 32 bit dans le buffer tab
  int err;
  err=pthread_mutex_lock(&place);
  if (err!=0){
    printf("mutex_place de insert_item fail lock\n");
  }
  tab[placetab]=ajout;
  placetab++;
  err=pthread_mutex_unlock(&place);
  if (err!=0){
    printf("mutex_place de insert_item fail unlock\n");
  }
}
//lit un des fichiers donné en arguments et les sépare en groupe de 32 bytes
//qu il met dans le buffer de taille "N"
void* producteur(void* nomfile){
  int err;
  char* fileName= (char*) nomfile;
  printf("%s\n",fileName );
  int fileReader =open(fileName,O_RDONLY);
  if(fileReader==-1){
    printf("pas open file fileName= %s\n",fileName);
    return ((void*)-1);  //gerer dans le main en cas d errreur -1 probleme dopen
  }
  size_t taille = sizeof(u_int8_t);
  int read1=1;
  while(read1>0){
    u_int8_t* ptr=(u_int8_t*) malloc(taille*32);
    if (ptr==NULL){
      printf("ton malloc a fail \n");
      return ((void*)-2);//malloc fail
    }
    for(int j=0;j<32;j++){
      u_int8_t bit;
      read1 = read(fileReader,(void*)&bit,taille);
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
      *(ptr+j)=bit;
  }
  sem_wait(&empty);  // attente d'un slot libre
  err=pthread_mutex_lock(&mutex);  // section critique
  if (err!=0){
    printf("mutex_mutex dans producteur fail lock\n");
  }
  insert_item(ptr);
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
    while (*(mdp+i)!='\0') {
      if (*(mdp+i)=='a' || *(mdp+i)=='e' || *(mdp+i)=='i' || *(mdp+i)=='o' || *(mdp+i)=='u' || *(mdp+i)=='y'){
        count++;
      }
      i++;
    }
    if(consonne==0){
      return(i-count);
    }
return count;
}



//il s'agit des thread qui vont prendre les 32 bytes et les décripter
void* consommateur(){
  int err;
  u_int8_t *item;
  //node_t *head=(node_t*)param;
  //printf("%d\n",head->nbcons);
  sem_wait(&full);// attente d'un slot rempli
  err=pthread_mutex_lock(&mutex);// section critique
  if (err!=0){
    printf("mutex_mutex dans consommateur fail lock\n");
  }
  pthread_mutex_lock(&place);  // section critique
  while (Fini!=1 || placetab!=0) {// si place tab ==0 va poser pb avoir valeur d'un sem

    placetab--;
    item=tab[placetab];
    err=pthread_mutex_unlock(&place);
    if (err!=0){
      printf("mutex_mutex dans place fail unlock\n");
    }
    err=pthread_mutex_unlock(&mutex);
    if (err!=0){
      printf("mutex_mutex dans consommateur fail unlock\n");
    }
    if (Fini!=1){
      sem_post(&empty);// il y a un slot libre en plus
    }
    char* mdp=(char*) malloc(sizeof(char)*16);
    if (mdp==NULL){
      printf("ton malloc a fail en char \n");
      return ((void*)-7);//malloc fail
    }
    bool boole;
    boole=reversehash(item,mdp,sizeof(char)*16);
    free(item);
    if (!boole){
      printf("reversehash fail\n");
      return ((void*)-8);//reversehash fail
    }
    int nbconsvoye=nbconsvoy(mdp);
    err=pthread_mutex_lock(&lettre);// section critique de mise en stack
    if (err!=0){
      printf("mutex_lettre fail lock\n");
    }
    if(maxmdp<=nbconsvoye){//rajoute a la stack
      printf("il est egale maxmdp et nbconsvoye\n");
      node_t *new=init(mdp,nbconsvoye);
      if(new==NULL){
        pthread_mutex_unlock(&lettre);//on libere la stack
        if (err!=0){
          printf("mutex_lettre fail lock\n");
        }
        return ((void*)9);
      }
      if (head==NULL){
        pthread_mutex_unlock(&lettre);//on libere la stack
        if (err!=0){
          printf("mutex_lettre fail lock\n");
        }
        return ((void*)10);
      }
      new->next=head;
      head=new;
      if(maxmdp<nbconsvoye){
        maxmdp=nbconsvoye;
      }
    }
    //printf("mdp=%s\n",*head->name);
    free(mdp);
    pthread_mutex_unlock(&lettre);//on libere la stack
    if (err!=0){
      printf("mutex_lettre fail lock\n");
    }
    if(Fini!=1){
      sem_wait(&full);// attente d'un slot rempli
    }
    err=pthread_mutex_lock(&mutex);// section critique
    if (err!=0){
      printf("mutex_mutex dans consommateur fail lock\n");
    }
    pthread_mutex_lock(&place);  // section critique
  }
  pthread_mutex_unlock(&place);
  err=pthread_mutex_unlock(&mutex);
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
  char *files[argc];
  nbfiles = argc-1; //car les arg contienne nom fichier donc -1 pour le nom
  for(int i=1;i<argc; i++){
    if(strcmp(*(argv+i),cons)==0){  //regarde si on demande de verifier pour les consonnes(==true) ou les voyelle(==false)
      consonne = 0;
      nbfiles--;
    }
    else if(strcmp(*(argv+i),nbthread)==0){  //regarde le nombre de threads utilise
      i++;
      nbfiles--;
      nbfiles--;
      if ((atoi(*(argv+i)))>1){
        maxThread = atoi(*(argv+i));
      }
    }//si pas -c ou -t alors c'est un fichier
    else{ //ecrit dans un tableau le nom des fichiers
      *(files+j) = *(argv+i);
      printf("nom du fichier %s\n",*(files+j));
      j++;
    }
  }

  printf("nb de fichier = %d\n",nbfiles);
  err=pthread_mutex_init(&mutex, NULL);
  if (err!=0){
    printf("mutex_mutex fail Initialisation\n");
  }
  err=pthread_mutex_init(&place, NULL);
  if (err!=0){
    printf("mutex_place fail Initialisation\n");
  }
  err=pthread_mutex_init(&lettre, NULL);
  if (err!=0){
    printf("mutex_lettre fail Initialisation\n");
  }
  head=(node_t*)malloc(sizeof(node_t));
  head->next=NULL;
  head->name=NULL;
  head->nbcons=0;
  sem_init(&empty,0,N); // buffer vide
  sem_init(&full,0,0);  // buffer vide
  pthread_t threadfil[nbfiles];
  for(long i=0;i<nbfiles;i++){
    err=pthread_create(&(threadfil[i]),NULL,&producteur,(void*)files[i]);
    if(err!=0){
      printf("errreur create producteur\n");
    }
  }
  //producteur((void*)*(files+0));
  pthread_t thread[maxThread];
  for(long i=0;i<maxThread;i++){
    err=pthread_create(&(thread[i]),NULL,&consommateur,NULL);
    if(err!=0){
      printf("errreur create consommateur\n");
    }
  }
  printf("j'ai cree pthread\n");

  for(long i=0;i<nbfiles;i++){
    err=pthread_join(threadfil[i],NULL);
    if(err!=0){
      printf("errreur pthread_join consommateur\n");
    }
  }
  Fini=1;
  for(long i=0;i<maxThread;i++){
    err=pthread_join(thread[i],NULL);
    if(err!=0){
      printf("errreur pthread_join consommateur\n");
    }
  }
  printf("j'ai join pthread\n");
  err=pthread_mutex_destroy(&mutex);
  if(err!=0){
    printf("errreur destroy mutex\n");
  }  //!!!!!!!!!!! on doit détruire mutex et semaphore
  err=pthread_mutex_destroy(&place);
  if(err!=0){
    printf("errreur destroy place\n");
  }
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
  }
  node_t *runner=head;
  while (runner->nbcons==maxmdp) {
    printf("mdp = %s\n",runner->name);
    runner=runner->next;
  }
}
