#define SEMAVISITAR 0
#define SEMVISITADO 1
#define SEMRESPUESTA 2
#define SEMHESPERA 3
#define SEMCONTDIR 4
#define SEMGTERMINO 5
#define NSEMAFOROS 6

typedef struct Nodo {
   char *path, *hash; //Contenido del nodo
   struct Nodo *siguiente; //Puntero al siguiente nodo
} nodo;

typedef struct col {
   nodo *primero; //Puntero que apunta al primer elemento de la cola
   nodo *ultimo; //Puntero que apunta al ultimo elemento de la cola
   int tamanio; //Tamaño de la cola
} Cola;

  
extern Cola aVisitar, visitado;
extern sem_t scola[NSEMAFOROS];

void waitpro(const Cola *cola){

  if(cola == &aVisitar){
    sem_wait(scola + SEMAVISITAR);
    //printf("{");
    return;
  }
  if(cola == &visitado){
    sem_wait(scola + SEMVISITADO);
    //printf("[");
    return;
  }

}

void postpro(const Cola *cola){

  if(cola == &aVisitar){
    //printf("}");
    sem_post(scola + SEMAVISITAR);
    return;
  }
  if(cola == &visitado){
    //printf("]");
    sem_post(scola + SEMVISITADO);
    return;
  }

}




/* Funciones con colas: */
void Inicializar ( Cola *cola){
  cola->primero = NULL;
  cola->ultimo = NULL;
  cola->tamanio = 0;
}

int colaVacia(const Cola *cola)
{
  int retvar=0;
  waitpro (cola)
  ;//printf("CV");
   if(cola->primero == NULL)
    retvar=1;
  //printf("CV");
  postpro(cola);
    return retvar;
}

void Insertar( Cola *cola, char * _path, char *_hash)
{

  nodo *nuevoNodo;
  nuevoNodo = malloc(sizeof(nodo)); //Creamos el nodo

  nuevoNodo->path = _path;
  nuevoNodo->hash = _hash;
  nuevoNodo->siguiente = NULL;
  waitpro(cola);
  //printf("I");
  if (cola->primero==NULL){
    cola->primero = nuevoNodo;
  }else{
    (cola->ultimo)->siguiente = nuevoNodo;
  }

  cola->ultimo = nuevoNodo;
  cola->tamanio++;
  //printf("I");
  postpro(cola);
}

void Leer(const Cola *cola)
{
  waitpro(cola);
  //printf("L");
   if (cola->primero==NULL){
     //printf("La cola está vacia\n");
      //printf("L");
      postpro(cola);
      return;  
   }
    
   int cont = cola->tamanio;
   nodo *nodoActual = cola->primero;

   while(cont > 0)
   {
      //printf("%s\n", nodoActual->path);
      nodoActual = nodoActual->siguiente;
      cont--;
    }
    //printf("L");
    postpro(cola);
}

nodo Desencolar(Cola *cola)
{
  nodo ret;
  waitpro(cola);
  //printf("D");
  if (cola->primero==NULL){
    //printf("D");
    postpro(cola);
    //printf("No hay cola.\n");
    ret.path= NULL;
    return ret;
  }else
  {
    nodo *nodoAux;
    cola->tamanio--;
    nodoAux = cola->primero;
    ret = *nodoAux;
    cola->primero = nodoAux->siguiente;

  if(cola->primero == NULL)
  {
    cola->ultimo = NULL;
  }
  //printf("D");
  postpro(cola);
  ret= *nodoAux;
  free(nodoAux);   
  return ret;
  }
   
}