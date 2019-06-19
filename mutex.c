#include  <stdio.h>
#include  <stdlib.h>
#include  <pthread.h>
#include  <unistd.h>

#define MAX_PROCESOS 1
#define NUM_HILOS 8

struct datos_tipo
{

    int dato;
    int p;
};

//variable goblal mutex
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *proceso(void *datos)
{

    struct datos_tipo *datos_proceso;
    datos_proceso=(struct datos_tipo *)datos;
    int a,i,j,p;
       
    a=datos_proceso->dato;
    p=datos_proceso->p;
    
    //bloqueamos el mutex
    pthread_mutex_lock(&lock);
    
    for(i=0;i<=p;i++){
        printf("%i",a);

    }
    fflush(stdout);
    sleep(1);
    for(i=0;i<=p;i++){
        printf("- ");   
    }
    fflush(stdout);
    
    //desbloqueamos el mutex
    pthread_mutex_unlock(&lock);

}

int main()
{
    int error,i;
    char *valor_devuelto;

    //variables para hilos
    struct datos_tipo hilo_datos[NUM_HILOS];
    pthread_t idhilo[NUM_HILOS];

        if(pthread_mutex_init(&lock,NULL)!=0)
    {
        printf("Mutex init failed");
        exit(-1);
    }   

    for(i=0;i<NUM_HILOS;i++){
        hilo_datos[i].dato=i;
        hilo_datos[i].p=i+1;
    }

    //se crean y lanzan los hilos

    for(i=0;i<NUM_HILOS;i++){
        error=pthread_create(&idhilo[i],NULL,(void*)proceso,(void*)(&hilo_datos[i]));
        if(error!=0)
        {
            perror("No se pudo crear hilo");
            exit(-1);
        }   
    }

    //se espera a que terminen

        for(i=0;i<NUM_HILOS;i++){
            pthread_join(idhilo[i],(void**)&valor_devuelto);
        }
    pthread_mutex_destroy(&lock);       
    return 0;
}