#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>
#include "colaT.h"
#define AUMENTO 5
#define HASHSIZE 32
#define PATHSIZE 250
#define NONZERO 1
#define HASH_EMPTY_FILE "e4c23762ed2823a27e62a64b95c024e7"


/*
poner todo modular por decepcion

las funciones de la cola funcionan
la lectura de directorios funciona
*/
/*funcion del hash modo libreria*/
int MDFile(char* filename, char hashValue[HASHSIZE]);

/*Estructura de visitados y a visitar*/
/*a y b son el par de archivos duplicados*/
typedef struct sol{
   char *a, *b;
}sol;

void waitpro(const Cola *cola);
void postpro(const Cola *cola);
void Inicializar ( Cola *cola); //Sería el constructor de nuestra cola
int colaVacia(const Cola *cola); //Si esta vacía devuelve un 1, de lo contrario devuelve un 0
void Insertar( Cola *cola, char *_path , char* _hash); //Insertar datos a la cola
void Leer(const Cola *cola); //Para ver lo que contiene la cola
sol* hayDuplicado( const Cola *cola, nodo hash);//Ve si en la cola hay alguien igual a hash
nodo Desencolar( Cola *cola); //Desencola y devuelve el contenido de esta
char* generarHash(char* path); //genera el hash de un archivo

Cola aVisitar, visitado; //Declaramos las dos colas que vamos a usar
pthread_t *hilo;
sem_t scola[NSEMAFOROS];//0 es para cola avisitar, 1 para cola visitado
sem_t scont;
void *recorrerArchivos();

char modo = ' ';
int Gtermino;

int sembeta(int  semaforo, int *var, int cambio){
  sem_wait(scola + semaforo);
    *var+=cambio; //printf("SEM BETAAAA %d\n", *var);
  sem_post(scola + semaforo);
  return *var;
}

/*Variables donde se guardaran los parÃ¡metros*/
  sol *respuesta=NULL;
  int numHilos, numDuplicados=0, numMaxDuplicados=0;
  int i, hespera, Gtermino;
  
int main(int argc, char *argv[])
{
  if(argc != 7 ){
    printf("ARGUMENTOS INSUFICIENTES\n");
    exit(0);
  }
  Gtermino=0;
  /*Variables del getopt*/
  int c;
  extern char* optarg; //Parámetro
  extern int optind;
  extern int optopt;
  extern int opterr;
  opterr = 0;

  /*Inicializando semaforo*/
  sem_init(&scola[SEMAVISITAR], NONZERO, 1);
  sem_init(&scola[SEMVISITADO], NONZERO, 1);
  sem_init(&scola[SEMRESPUESTA], NONZERO, 1);
  sem_init(&scola[SEMHESPERA], NONZERO, 1);
  sem_init(&scola[SEMCONTDIR], NONZERO, 1);
  sem_init(&scola[SEMGTERMINO], NONZERO, 1);
  /*> ./duplicados -t <numero de threads> -d <directorio de inicio> -m <e | l >*/
    //Entrada
  while ((c = getopt (argc, argv, "t:d:m:")) != -1)
  {
    switch(c)
    {
      case 't':/*parametro -t*/
        numHilos = strtoul(optarg, NULL, 10);
        hespera= numHilos;
        if(numHilos <= 0){
          printf("ERROR PARAMETRO -t\n");
          exit(0);
        } 
      break;

      case 'd': /*parametro -d*/
        Inicializar(&aVisitar);
        Inicializar(&visitado);
        Insertar(&aVisitar, optarg, NULL);
      break;

      case 'm':/*parametro -m*/
        modo = optarg[0];
        if(modo!='e' && modo!='l'){
        printf("ERROR PARAMETRO -m\n");
        exit(0);
        }
      break;
      default:
      exit(1);
    }
  }  
 
  //Crear cantidad de hilos que trabajaran en el problema
  hilo= malloc(numHilos * sizeof(pthread_t));
  for (i = 0; i < numHilos; ++i)
  {
    int *aux = malloc(sizeof(int));
    *aux = i;
    pthread_create(hilo + i, NULL, recorrerArchivos, (void *) aux);
  }

  for (i = 0; i < numHilos; ++i)
  {
    pthread_join(hilo[i], NULL);
  }
  //Salida
  printf("Se han encontrado %d archivos duplicados.\n",numDuplicados);
  for(i=0; i<numDuplicados; i++)
    printf("%s es duplicado de %s\n",respuesta[i].a, respuesta[i].b);
  //Fsalida
  

  free(respuesta);
  free(hilo);
  for(i=0; i<numHilos; i++)
  sem_destroy(scola + i);
   return 0;
}



void *recorrerArchivos(void *thread)
{
  int taskid = *(int*)thread;
  DIR *dirp;
  struct dirent* dent;

  while(1)
  {
    sem_wait(scola + SEMCONTDIR);

    //printf("Paso %d\n", taskid);
    if(sembeta(SEMGTERMINO, &Gtermino, 0)==1){
      //printf("MUERO %d\n", taskid);
      pthread_exit(0);
    }
    sembeta(SEMHESPERA, &hespera, -1);
    
    nodo nodoAVisitar = Desencolar(&aVisitar);
    dirp = opendir(nodoAVisitar.path);
    if(dirp==NULL){
      printf("NO SE ENCUENTRA EL DIRECTORIO\n");
      exit(0);
    }
    /*Inicio de lectura de directorio*/
    do{
      dent = readdir(dirp);
      if(dent==NULL)break;//NO borrar
      // Genero path
      char *path= malloc(PATHSIZE* sizeof(char));
      sprintf(path, "%s/%s", nodoAVisitar.path, dent->d_name);

      if (dent->d_type == DT_DIR)
      {
        if(strcmp(dent->d_name,".")!=0 && strcmp(dent->d_name,"..")!=0)
        { 
          Insertar(&aVisitar, path, NULL);
          sem_post(scola + SEMCONTDIR);//IMPORTANTEEEE
        }
      }
      else if (dent->d_type == DT_REG)
      {
        sol *auxsol=NULL;
        nodo a;
        a.path= path;
        a.hash= generarHash(a.path);
        //printf("\n[%s]---->[%s]\n", a.path, a.hash);
        if((auxsol= hayDuplicado(&visitado,a)) != NULL)
        {
          sem_wait(scola + SEMRESPUESTA);
          if(numDuplicados >= numMaxDuplicados)
          {
            respuesta = realloc (respuesta, (numMaxDuplicados + AUMENTO) * sizeof(sol));
            numMaxDuplicados += AUMENTO;
          }
          respuesta[numDuplicados] = *auxsol;
          numDuplicados++;
          sem_post(scola + SEMRESPUESTA);
        }else{
            Insertar(&visitado, a.path, a.hash);
        }

      }
    }while(1);
    closedir(dirp);

    if (sembeta(SEMHESPERA, &hespera, 1)==numHilos && colaVacia(&aVisitar))
    {
      //printf("<LIBERO %d>",taskid);
      sembeta(SEMGTERMINO, &Gtermino, 1);

      while(sembeta(SEMHESPERA, &hespera,-1)!=-1)
        sem_post(scola + SEMCONTDIR);
      }

      //printf("Salgo %d\n", taskid);
  }
}

sol* hayDuplicado(const Cola *cola, nodo a){
  nodo *aux; //lo usare para iterar
  if(strcmp(a.hash, HASH_EMPTY_FILE)==0){
    return NULL; //NO TOMO EN CUENTA ARCHIVOS VACIOS
  }
  waitpro(cola);
  //printf("hD");
  aux=cola->primero;
  while(aux!=NULL){
    if(strcmp(a.hash,aux->hash)==0){
      sol *respuestica= malloc(sizeof(sol));
      respuestica->a=a.path;
      respuestica->b=aux->path;
      //printf("hD");
      postpro(cola);
      return respuestica;
    }
    aux= aux->siguiente;
  }
  //printf("hD");
  postpro(cola);
  return NULL; 
}

char* generarHash(char* path){
  char *hash=malloc(HASHSIZE * sizeof(char));
  if(modo=='e')
  {
  pid_t pid; int fd[2], wstatus;
    pipe(fd);
    pid= fork();
    if(pid==0)
    {
      char *dir[]={"./md5",path,(char) 0};
      dup2 (fd[1], STDOUT_FILENO);
      execv("md5", dir);
      close(fd[0]);
      close(fd[1]);
      exit(0);
    }
    wait(&wstatus);

    read(fd[0], hash, HASHSIZE);
    close(fd[0]);
    close(fd[1]);
    return hash;
  }
  else
  if(modo='l')
  {
    MDFile(path,  hash);
    return hash;
  }
}