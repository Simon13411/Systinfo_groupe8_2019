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
#include "./reverse.h"
#include "./reverse.c"
// Initialisation
#define N 10 // slots du buffer
#define cons "-c"
#define nbthread "-t"
int Fini =0;
pthread_mutex_t mutex;
pthread_mutex_t FiniMutex;
pthread_mutex_t lettre;
pthread_mutex_t place;
sem_t empty;
int consonne = 1;
sem_t full;
uint8_t tab[N]={};
int nbfiles;
int placetab=0;
int maxmdp=0;
struct node {
    struct node *next;
    char *name;
};
struct node **head;
//Initialisation des diverse variale inter thread afin d eviter de les passer par des structure


int main(int argc, char *argv[]) {
  int maxThread = 1;
  int err;
  char *files[argc];
  int testthread=0;
  nbfiles = argc-1; //car les arg contienne nom fichier donc -1 pour le nom
  printf("argc=%d\n",argc);
  for(int i=1;i<argc; i++){
    printf("i=%d\n",i);
    if(strcmp(*(argv+i),cons)==0){  //regarde si on demande de verifier pour les consonnes(==true) ou les voyelle(==false)
      consonne = 0;
      nbfiles--;
      printf("consonne = %d\n",consonne);
    }
    else if(strcmp(*(argv+i),nbthread)==0){  //regarde le nombre de threads utilise
      i++;
      if ((atoi(*(argv+i)))>1){
        nbfiles--;
        maxThread = atoi(*(argv+i));
        printf("nombre de threads = %d\n", maxThread);
      }
      printf("thread<1\n");
      nbfiles--;
      testthread=1;
    }//si pas -c ou -t alors c'est un fichier
    else{ //ecrit dans un tableau le nom des fichiers
    *(files+i) = *(argv+i);
    printf("nom du fichier %s\n",*(argv+i));
    }
  }
  printf("nombre de threads = %d\n",maxThread);
  if (testthread==1){
    printf("contient nbthreads\n");
  }
  if (consonne==0){
    printf("contient consonne\n");
  }
  printf("longuer fichier = %d\n",nbfiles);


  pthread_t thread[maxThread];
  for(long i;i<maxThread;i++){
    err=pthread_create(&(thread[i]),NULL,&consommateur,NULL)
    if(err!=0){
      printf("errreur create consommateur\n");
    }
  }

  err=pthread_mutex_init(&mutex, NULL);
  if (err!=0){
    printf("mutex_mutex fail Initialisation\n");
  }
  sem_init(&empty,0,N); // buffer vide
  sem_init(&full,0,0);  // buffer vide


  for(long i;i<maxThread;i++){
    err=pthread_join(thread[i],NULL);
    if(err!=0){
      printf("errreur pthread_join consommateur\n");
    }
  }

  //!!!!!!!!!!! on doit détruire mutex et semaphore
}

void insert_item(uint8_t ajout){  //ajout d un bloc de 32 bit dans le buffer tab
  *(tab+placetab)=ajout;
  int err;
  err=pthread_mutex_lock(&place);
  if (err!=0){
    printf("mutex_place de insert_item fail lock\n");
  }
  placetab++;
  err=pthread_mutex_unlock(&place);
  if (err!=0){
    printf("mutex_place de insert_item fail unlock\n");
  }
}

//lit un des fichiers donné en arguments et les sépare en groupe de 32 bytes
//qu il met dans le buffer de taille "N"
void* producteur(char *fileName){
  int err;
  int fileReader =open(fileName,O_RDONLY);
  if(fileReader==-1){
    pthread_mutex_lock(&FiniMutex);  // section critique
    Fini++;
    pthread_mutex_unlock(&FiniMutex);
    return ((void*)-1);  //gerer dans le main en cas d errreur -1 probleme dopen
  }
  size_t taille = sizeof(uint8_t);
  int read1=1;
  while(read1>0){
    uint8_t* ptr=(uint8_t*) malloc(taille*32);
    if (ptr==NULL){
      printf("ton malloc a fail \n");
      return ((void*)-2);//malloc fail
    }
    for(int j=0;j<32;j++){
      uint8_t bit;
      read1 = read(fileReader,(void*)&bit,taille);
      if(read1==-1){
        pthread_mutex_lock(&FiniMutex);  // section critique
        Fini++;
        pthread_mutex_unlock(&FiniMutex);
        return ((void*)-3); //probleme de read
      }
      if(read1==0){ // fin du fichier
        int fermer = close(fileReader);
        if(fermer==-1){
          pthread_mutex_lock(&FiniMutex);  // section critique
          Fini++;
          pthread_mutex_unlock(&FiniMutex);
          return ((void*)-4); //probleme de close
        }
        pthread_mutex_lock(&FiniMutex);  // section critique
        Fini++;
        pthread_mutex_unlock(&FiniMutex);
        return ((void*)-5); // moins 32 bytes
      }
      *(ptr+j)=bit;
  }
  sem_wait(&empty);  // attente d'un slot libre
  err=pthread_mutex_lock(&mutex);  // section critique
  if (err!=0){
    printf("mutex_mutex dans producteur fail lock\n");
  }
  insert_item(*ptr);
  err=pthread_mutex_unlock(&mutex);
  if (err!=0){
    printf("mutex_mutex dans producteur fail unlock\n");
  }
  sem_post(&full);  // il y a un slot rempli en plus
 }
 int fermer = close(fileReader);
 if(fermer==-1){
  pthread_mutex_lock(&FiniMutex);  // section critique
  Fini++;
  pthread_mutex_unlock(&FiniMutex);
  return ((void*)-6);
 }
 pthread_mutex_lock(&FiniMutex);  // section critique
 Fini++;
 pthread_mutex_unlock(&FiniMutex);
 return ((void *)0); // tout est ok tout c est passe correctement
}



uint8_t remplacer(){ // retire le dernier du buffer tab
  int err;
  err=pthread_mutex_lock(&place);
  if (err!=0){
    printf("mutex_place de remplacer fail lock\n");
  }
  placetab--;
  err=pthread_mutex_unlock(&place);
  if (err!=0){
    printf("mutex_place de remplacer fail unlock\n");
  }
  return *(tab+placetab);
}
int nbconsvoy(char* mdp){
  int count=0;
  if (mdp==NULL){
    return -8;
  }
  if (consonne==0){
    int i=0;
    while (*(mdp+i)!='\0') {
      if (*(mdp+i)!='a' || *(mdp+i)!='e' || *(mdp+i)!='i' || *(mdp+i)!='o' || *(mdp+i)!='u' || *(mdp+i)!='y'){
        count++;
      }
    }
  }
  else{
    int i=0;
    while (*(mdp+i)!='\0') {
      if (*(mdp+i)=='a' || *(mdp+i)=='e' || *(mdp+i)=='i' || *(mdp+i)=='o' || *(mdp+i)=='u' || *(mdp+i)=='y'){
        count++;
      }
    }
  }
return count;
}


int pop(struct node **head){
  if (head==NULL){
    return 1;
  }
  struct node *rm=(struct node*)malloc(sizeof(struct node));
  if (rm==NULL){
    return 2;
  }
  rm=*head;
  *head=rm->next;
  free(rm->name);
  free(rm);
  return 0;
}

int push(struct node **head, const char *value){
  struct node *new=(struct node*)malloc(sizeof(struct node));
  if (new==NULL){
    return 1;
  }
  char* val=(char*)malloc(strlen(value)+1);
  if (val==NULL){
    free(new);
    return 1;
  }
  if (head==NULL)
    return 1;
  new->next=*head;
  *head= new;
  new->name =strcpy(val,value);
  return 0;
}

//il s'agit des thread qui vont prendre les 32 bytes et les décripter
void* consommateur(void * param){
  int err;
  pthread_mutex_lock(&FiniMutex);  // section critique
  int Fin=Fini;
  pthread_mutex_unlock(&FiniMutex);
  while (Fin!=nbfiles) {// si place tab ==0 va poser pb avoir valeur d'un sem
    sem_wait(&full);// attente d'un slot rempli
    err=pthread_mutex_lock(&mutex);// section critique
    if (err!=0){
      printf("mutex_mutex dans consommateur fail lock\n");
    }
    const uint8_t *item=remplacer();
    err=pthread_mutex_unlock(&mutex);
    if (err!=0){
      printf("mutex_mutex dans consommateur fail unlock\n");
    }
    sem_post(&empty);// il y a un slot libre en plus
    char* mdp=(char*) malloc(16);
    if (mdp==NULL){
      printf("ton malloc a fail en char \n");
      return ((void*)-7);//malloc fail
    }
    int boole;
    boole=reversehash(item,mdp,16);
    if (boole==0){
      return ((void*)-8);//reversehash fail
    }
    int nbconsvoye=nbconsvoy(mdp);
    err=pthread_mutex_lock(&lettre);// section critique de mise en stack
    if (err!=0){
      printf("mutex_lettre fail lock\n");
    }
    if(maxmdp==nbconsvoye){//rajoute a la stack
      int good=push(head,mdp);
      if(good!=0)
        return ((void*)-9);//pas mis dans la stack
    }
    if(maxmdp<nbconsvoye){//retire tt puis on met dans la stack
      int good=0;
      while (good!=2) {//retire tt de la stack
        good=pop(head);
        if(good==1)
          return ((void*)-10);//head est NULL
      }
      int good2=push(head,mdp);//put ds la stack
      if(good2!=0)
        return ((void*)-11);//pas mis dans la stack
    }
    pthread_mutex_unlock(&lettre);//on libere la stack
    if (err!=0){
      printf("mutex_lettre fail lock\n");
    }
  }
}
