// para compilar: gcc prodcons_1_thread_sem.c -o prodcons_1_thread_sem -pthread
// para executar: prodcons_1_thread_sem
//
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_PRODUCED 25
#define MAX_QUEUE 5
#define N 3

sem_t mutex_critico, empty, full, mutex_produtor;

int queue[MAX_QUEUE], item_available=0, produced=0, consumed=0;


int create_item(void) {

	return(rand()%1000);

} //fim  create_item()

void insert_into_queue(int item) {
	
	queue[item_available++] = item;
	produced++;
	printf("producing item:%d, value:%d, queued:%d \n", produced, item, item_available); 
	return;

} // fim insert_into_queue()

int extract_from_queue() {

	consumed++;
	printf("cosuming item:%d, value:%d, queued:%d \n", consumed, queue[item_available-1], item_available-1); 
	
	return(queue[--item_available]);

} // fim extract_from_queue()

void process_item(int my_item) {
	static int printed=0;

	printf("Printed:%d, value:%d, queued:%d \n", printed++, my_item, item_available);

	return;

} // fim_process_item()


void *producer(void) {
	int item;

	
	while (1) {

		sem_wait(&mutex_produtor);
	// Condicao de checagem é movida para um if envolvido por um novo
	// semaforo para evitar que um novo item seja produzido em excesso
	// (caso haja uma preempcao de uma thread produtora e a substituta
	// seja outra produtora, no momento em que o limite de itens foi
	// atingido)
		if(produced < MAX_PRODUCED) {
			sem_wait(&empty);
			sem_wait(&mutex_critico);
	  		item = create_item(); // Producao de item envolvida por semaforos para
	  						  // evitar que o numero maximo de itens
	  						  // produzidos seja superado
	  		insert_into_queue(item);
	  		sem_post(&mutex_critico);
	  		sem_post(&full);
	  		sem_post(&mutex_produtor);
	  }
	  else {
	  	sem_post(&mutex_produtor);
	  	printf("\nThread producer saindo.\n\n");
	  	fflush(0);
	  	pthread_exit(0);
	  }
	}
} // fim producer

void *consumer(void) {
	int my_item = 0;

	while (consumed < MAX_PRODUCED) {
		sem_wait(&full);
		sem_wait(&mutex_critico);
		my_item = extract_from_queue();
		sem_post(&mutex_critico);
		sem_post(&empty);
		process_item(my_item);
	}

	printf("\nThread consumer saindo.\n\n");
	fflush(0);
	
	pthread_exit(0);
} // fim_consumer


int main(void) {
	pthread_t prod_handle[N], cons_handle[N];

	/* declarations and initializations */
	item_available = 0;

	sem_init (&mutex_critico, 0 , 1);
	sem_init(&empty, 0, MAX_QUEUE);
	sem_init(&full, 0, 0);
	sem_init (&mutex_produtor, 0 , 1);

	/* create and join producer and consumer threads */
	for(int i = 0; i < N; i++) {
		if (pthread_create(&prod_handle[i], 0, (void *) producer, (void *) 0) != 0) { 
			printf("Error creating thread producer! Exiting! \n");
			exit(0);
		}
		if (pthread_create(&cons_handle[i], 0, (void *) consumer, (void *) 0) != 0) { 
			printf("Error creating thread consumer! Exiting! \n");
			exit(0);
		}
	}
	
	printf("\n Thread pai vai esperar filhas.\n\n");
	fflush(0);

	for(int i = 0; i < N; i++) {
		pthread_join(prod_handle[i], 0);
		pthread_join(cons_handle[i], 0);
	}
	getchar();

	printf("Thread pai saindo.\n");
	fflush(0);
	
	exit(0);	
} // main()
