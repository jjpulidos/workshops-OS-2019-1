# Taller Mutex

```c=
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

```


### Explicación del código presente en el taller

Después de la transcripción proveniente del PDF y de la ejecución del código también proveniente del mismo se evidencia que el funcionamiento del mismo corresponde a un cierto número N (N=8) de hilos los cuales imprimen el valor asignado a dato, p+1 veces, Aunque esto se realiza sin ningún metodo de sincronizacion de procesos, por lo cual se presenta el problema de que la impresión se da de manera desorganizada, es decir, que durante la impresión realizada por el hilo A se intercalan valores impresos por la ejecución del otro hilo B.

### Implementación del Mutex y análisis del mismo

Se desarrollo un método de sincronización del tipo Mutex en el cual se identificó las zonas críticas,  las cuales corresponden dentro de la función proceso en donde se inicia la impresión de los valores y donde también se finaliza la misma, en las cuales encontraremos las funciones características de Mutex (pthread_mutex_lock y pthread_mutex_unlock), como entrada encontramos un puntero al valor del mutex, que se encuentra inicializado en el main usando el método pthread_mutex_init, para luego de ser utilizado ser destruido con pthread_mutex_destroy.

Al ejecutar dicho código obtendremos la impresión de los valores de cada hilo de manera sincronizada, esto nos indica que cuando un hilo se encuentra en procesamiento este no es interrumpido por la impresión de algún otro hilo, por lo cual deducimos  que los demás hilos tendrán que esperar hasta que el buffer de impresión no esté bloqueado para de esta manera ellos puedan proceder a la impresión de los datos. Analizando un poco más en detalle, cuando cada hilo entra en la función de "proceso", justo antes de comenzar la impresión de los valores, mediante pthread_mutex_lock se bloquea y se notifica a los demás hilos que el buffer de impresión está en uso y no puede ser interrumpido, al final se desbloquea el buffer notificando con el mutex mediante pthread_mutex_unlock.