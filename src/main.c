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

uint32_t tab[N];

int main(int argc, char *argv[]) {
  int consonne = 1;
  int maxThread = 1;
  char *files[argc];
  int test=0;
  int nbfiles = argc-1;
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
        nbfiles--;
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
    nbfiles--;
  }
  if (consonne==0){
    printf("contient consonne\n");
    nbfiles--;
  }
  printf("longuer fichier = %d\n",nbfiles );


  pthread_mutex_t mutex;
  sem_t empty;
  sem_t full;
  pthread_mutex_init(&mutex, NULL);
  sem_init(&empty,0,N); // buffer vide
  sem_init(&full,0,0);  // buffer vide



}


//lit un des fichiers donné en arguments et les sépare en groupe de 32 bytes
//qu il met dans le buffer de taille "N"
int producteur(char *fileName){
  int fileReader = open(fileName, O_RDONLY);
  if(fileReader==-1){
    return -1;  //gerer dans le main en cas d errreur
  }
  size_t taille = sizeof(uint32_t);
  uint32_t item ;
  while(true && read!=0){
    int read = read(fileReader,(void*)&item,taille);
    if(read==-1){
      return -2; //gerer dans le main en cas d erreur
    }
    sem_wait(&empty);  // attente d'un slot libre
    pthread_mutex_lock(&mutex);  // section critique
    insert_item(item);
    pthread_mutex_unlock(&mutex);
    sem_post(&full);  // il y a un slot rempli en plus
  }
}
//il s'agit des thread qui vont prendre les 32 bytes et les décripter
void consommateur(void){
  int item;
  while(true){
    sem_wait(&full);// attente d'un slot rempli
    pthread_mutex_lock(&mutex);// section critique
    item=remove(item);
    pthread_mutex_unlock(&mutex);
    sem_post(&empty);// il y a un slot libre en plus
  }
  }
void insert_item(uint32_t ajout){
    for(int i=0; i<N; i++){
      if(*(tab+i)==NULL){
        *(tab+i)=ajout;
      }
    }
  }

uint32_t remove(){
    for(int i=0; i<N; i++){
      if(*(tab+i+1)==NULL){
        uint32_t item = *(tab+i);
        *(tab+i)=NULL;
        return item;
      }
    }
    return NULL;
  }
