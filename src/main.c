#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
//#include <reverse.h>
//#include <sha256.h>
// Initialisation
#define N 10 // slots du buffer
#define cons "-c"
#define nbthread "-t"
#define Fini 0
pthread_mutex_t mutex;
pthread_mutex_t FiniMutex;
sem_t empty;
sem_t full;
uint32_t tab[N];

int main(int argc, char *argv[]) {
  int consonne = 1;
  int maxThread = 1;
  char *files[argc];
  int test=0;
  int nbFiles = argc-1;
  printf("argc=%d\n",argc);
  for(int i=1;i<argc; i++){
    printf("i=%d\n",i);
    if(strcmp(*(argv+i),cons)==0){  //regarde si on demande de verifier pour les consonnes(==true) ou les voyelle(==false)
      consonne = 0;
      printf("consonne = %d\n",consonne );
    }
    else if(strcmp(*(argv+i),nbthread)==0){  //regarde le nombre de threads utilise
      i++;
      if ((atoi(*(argv+i)))>1){
        nbFiles--;
        printf("maxthread if\n");
        maxThread = atoi(*(argv+i));
        printf("nombre de threads = %d\n", maxThread);
      }
      printf("thread<1\n");
      test=1;
    }//si pas -c ou -t alors c'est un fichier
    else{ //ecrit dans un tableau le nom des fichiers
    *(files+i) = *(argv+i);
    printf("nom du fichier %s\n",*(argv+i));
    }
  }
  printf("nombre de threads = %d\n",maxThread );
  if (test==1){
    printf("contient nbthreads\n");
    nbFiles--;
  }
  if (consonne==0){
    printf("contient consonne\n");
    nbFiles--;
  }
  printf("longuer fichier = %d\n",nbFiles );



  pthread_mutex_init(&mutex, NULL);
  sem_init(&empty,0,N); // buffer vide
  sem_init(&full,0,0);  // buffer vide
}

void insert_item(uint32_t ajout){  //ajout d un bloc de 32 bit dans le buffer tab
    for(int i=0; i<N; i++){
      if(*(tab+i)==NULL){
        *(tab+i)=ajout;
      }
    }
  }

//lit un des fichiers donné en arguments et les sépare en groupe de 32 bytes
//qu il met dans le buffer de taille "N"
int producteur(char *fileName){
  int fileReader = open(fileName, O_RDONLY);
  if(fileReader==-1){
    pthread_mutex_lock(&FiniMutex);  // section critique
    Fini++;
    pthread_mutex_unlock(&FiniMutex);
    return -1;  //gerer dans le main en cas d errreur -1 probleme dopen
  }
  size_t taille = sizeof(uint32_t);
  uint32_t item ;
  int read=1;
  while(read>0){
    read = read(fileReader,(void*)&item,taille);
    if(read==-1){
      pthread_mutex_lock(&FiniMutex);  // section critique
      Fini++;
      pthread_mutex_unlock(&FiniMutex);
      return -2; //gerer dans le main en cas d erreur -2 probleme de read
    }
    if(read==0){
      int fermer = close(fileName);
      if(fermer==-1){
        pthread_mutex_lock(&FiniMutex);  // section critique
        Fini++;
        pthread_mutex_unlock(&FiniMutex);
        return -3 //gerer dans le main en cas d erreur -3 probleme de close
      }
      pthread_mutex_lock(&FiniMutex);  // section critique
      Fini++;
      pthread_mutex_unlock(&FiniMutex);
      return 0; // tout est ok tout c est passe correctement
    }
    sem_wait(&empty);  // attente d'un slot libre
    pthread_mutex_lock(&mutex);  // section critique
    insert_item(item);
    pthread_mutex_unlock(&mutex);
    sem_post(&full);  // il y a un slot rempli en plus
  }
  pthread_mutex_lock(&FiniMutex);  // section critique
  Fini++;
  pthread_mutex_unlock(&FiniMutex);
  return 1; // gros souci car pas cense rentret dedans
}


uint32_t remove(){ // retire le dernier du buffer tab
    for(int i=0; i<N; i++){
      if(*(tab+i+1)==NULL){
        uint32_t item = *(tab+i);
        *(tab+i)=NULL;
        return item;
      }
    }
    return NULL;
  }
//il s'agit des thread qui vont prendre les 32 bytes et les décripter
void consommateur(void){
  uint32_t item;
  int nbFini
  pthread_mutex_lock(&FiniMutex);  // section critique
  nbFini=Fini;
  pthread_mutex_unlock(&FiniMutex);
  while(nbFini==nbFiles){
    sem_wait(&full);// attente d'un slot rempli
    pthread_mutex_lock(&mutex);// section critique
    item=remove();
    pthread_mutex_unlock(&mutex);
    sem_post(&empty);// il y a un slot libre en plus

  }
}
