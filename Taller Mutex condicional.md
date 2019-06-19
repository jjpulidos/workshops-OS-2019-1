# Taller Mutex condicional
```c=
#include  <stdio.h>
#include  <stdlib.h>
#include  <pthread.h>
#include  <unistd.h>

#define MAX_BUFFER 15
#define MAX_DATOS 100

//variables globales

pthread_mutex_t mutex;
pthread_cond_t vacio,lleno;
int buffer [MAX_BUFFER], num_datos;

//proceso consumidor

void *consumidor()
{
	struct datos_tipo *datos_proceso;
	int a,i,j,p,posicion=0,dato;
	
	for(i=0;i<MAX_DATOS;i++)
	{
		//Acá se hace mutex
		pthread_mutex_lock(&mutex);
		while(num_datos==0)
			//mutex condicional
			pthread_cond_wait(&vacio,&mutex);
		dato=buffer[posicion];
		if(posicion ==15)
			posicion=0;
		else
			posicion++;
		num_datos--;
		if(num_datos==MAX_BUFFER-1)
			//mutex condicional, si está lleno
			pthread_cond_signal(&lleno);
		//se desbloquea mutex
		pthread_mutex_unlock(&mutex);
		printf("\nse ha consumido el dato: %d",dato);
		fflush(stdout);
		sleep(1);			 
	}
	pthread_exit(0);
}

void *productor()
{
	struct datos_tipo*datos_proceso;
	int a,i,j,p,posicion=0,dato;

	for(i=0;i<MAX_DATOS;i++)
	{
		//lock mutex
		pthread_mutex_lock(&mutex);
		while(num_datos==MAX_BUFFER)
			//mutex cond
			pthread_cond_wait(&lleno,&mutex);
		buffer[posicion]=i;
		dato=i;

		if(posicion==15)
			posicion=0;
		else
			posicion++;
		num_datos ++;
		if(num_datos==1)
			//mutex cond vacio
			pthread_cond_signal(&vacio);
		//mutex unlock
		pthread_mutex_unlock(&mutex);
		printf("\nse ha producido el dato: %d",dato);
		fflush(stdout);
	}	
	pthread_exit(0);
}

int main()
{
	int error;
	char *valor_devuelto;
	//variables para hilos
	pthread_t idhilo1,idhilo2,idhilo3,idhilo4;

	pthread_mutex_init(&mutex,NULL);
        pthread_cond_init(&lleno,NULL);
        pthread_cond_init(&vacio,NULL);		
	error=pthread_create(&idhilo1,NULL,(void*)productor,NULL);
	if(error !=0)
	{
		perror("No se pudo crear hilo");
		exit(-1);
	}
	error=pthread_create(&idhilo2,NULL,(void*)consumidor,NULL);
	if(error !=0)
	{
		perror("No se pudo crear hilo");
		exit(-1);
	}
	pthread_join(idhilo2,(void**)&valor_devuelto);
	pthread_join(idhilo1,(void**)&valor_devuelto);
	pthread_cond_destroy(&lleno);
	pthread_cond_destroy(&vacio);
	return 0;
}

```


### Puntos Críticos

Se pueden evidenciar los puntos críticos del programa contenidos de las funciones de consumidor y productor para los casos en los que el buffer está vacío y cuando se encuentra lleno. En el caso del consumidor (cuando el buffer está vacío) se activa la variable condicional de vacio  y se libera el buffer para que el producer pueda utilizarlo, llenando así el buffer con al menos un elemento. Se realizó de la misma manera con el productor y la variable condicional lleno, liberando el buffer cuando se encuentra lleno para que el consumidor pueda vaciarlo.


### Análisis de la ejecución

Al ejecutar el programa podemos ver que al principio se ejecutan los dos hilos sin instrucciones, pero al entrar el consumidor este se bloquea ante la variable condicional "vacío" así que procede a esperar que el productor introduzca datos. El productor al producir los datos lanza una señal usando pthread_cond_signal a la variable "vacío" para liberar el bloqueo sobre el consumidor empezando así la lógica entre el consumidor y el productor, la cual luego de haber producido aunque fuese un dato, este se consume y luego se terminan de producir los datos restantes hasta llenar el buffer, a partir de allí el consumidor consume los datos y desbloquea al productor para que continúe la producción y viceversa.

### Ejecución sin el uso de variables condicionales

Sin la presencia de las variables auxiliares la lógica se reduce simplemente a que el productor genere todos los datos, osea los valores hasta MAX_DATOS estarán sobreescribiendo los valores del buffer hasta terminar dejándolo lleno, luego aquellos valores desde MAX_DATOS - MAX_BUFFER hasta MAX_DATOS -1 serán tomados por el consumidor


### Implementación usando Tuberias

```c=
#include  <stdio.h>
#include  <stdlib.h>
#include  <pthread.h>
#include  <unistd.h>

#define MAX_BUFFER 16
#define MAX_DATOS 10

//variables globales

pthread_mutex_t mutex;
pthread_cond_t vacio,lleno;
int buffer [MAX_BUFFER], num_datos,r;
char buff;
int pipefd[2];

//proceso consumidor

void *consumidor()
{
	struct datos_tipo *datos_proceso;
	int a,i,j,p,posicion=0,dato;
        r=read(pipefd[0], &buff, 1024);	
	for(i=0;i<MAX_DATOS;i++)
	{
		//Acá se bloquea pipe

		dato=buffer[posicion];
		if(posicion ==15)
			posicion=0;
		else
			posicion++;
		num_datos--;
		if(num_datos==MAX_BUFFER-1)
		//se desbloquea pipe
                r=write(pipefd[1], "T", 1);
		printf("\nse ha consumido el dato: %d",dato);
		fflush(stdout);
		r=read(pipefd[0], &buff, 1024);
		sleep(1);			 
	}
	r=write(pipefd[1], "T", 1);
	pthread_exit(0);
}

void *productor()
{
	struct datos_tipo*datos_proceso;
	int a,i,j,p,posicion=0,dato;
        r=read(pipefd[0], &buff, 1024);	
	for(i=0;i<MAX_DATOS;i++)
	{
		//Acá se bloquea pipe	
		buffer[posicion]=i;
		dato=i;
		if(posicion==15)
			posicion=0;
		else
			posicion++;
		num_datos ++;
		if(num_datos==1)
		//se desbloquea pipe
                r=write(pipefd[1], "T", 1);
		printf("\nse ha producido el dato: %d",dato);
                r=read(pipefd[0], &buff, 1024);
		fflush(stdout);
	}	
	pthread_exit(0);
}

int main()
{
	r=pipe(pipefd);
        //validate pipe creation	
	int error;
	char *valor_devuelto;
	//variables para hilos
	pthread_t idhilo1,idhilo2,idhilo3,idhilo4;
	r=write(pipefd[1], "T", 1);
	error=pthread_create(&idhilo1,NULL,(void*)productor,NULL);
	if(error !=0)
	{
		perror("No se pudo crear hilo");
		exit(-1);
	}
	error=pthread_create(&idhilo2,NULL,(void*)consumidor,NULL);
	if(error !=0)
	{
		perror("No se pudo crear hilo");
		exit(-1);
	}
	pthread_join(idhilo2,(void**)&valor_devuelto);
	pthread_join(idhilo1,(void**)&valor_devuelto);
        close(pipefd[0]);
        close(pipefd[1]);
	return 0;
}
```