#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
//#include <reverse.h>
//#include <sha256.h>
// Initialisation
#define N 10 // slots du buffer
#define cons "-c"
#define nbthread "-t"

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


// Producteur
void producer(void){
  int item;
  while(true){
    item = produce(item);
    sem_wait(&empty);  // attente d'un slot libre
    pthread_mutex_lock(&mutex);  // section critique
    insert_item();
    pthread_mutex_unlock(&mutex);
    sem_post(&full);  // il y a un slot rempli en plus
  }
}
