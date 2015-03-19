// Algoritmo Produtor/Consumidor com N produtores e N consumidores
// ***************************************************************
// Grupo:
// Guilherme Nishina Fortes 				- No. USP 7245552
// Henrique de Almeida Machado da Silveira 	- No. USP 7961089
// Marcello de Paula Ferreira Costa 		- No. USP 7960690
// Sergio Yudi Takeda 						- No. USP 7572996
// ***************************************************************
// Engenharia de Computação - USP Sao Carlos
// Data: 20 de marco de 2015

// Comentarios: para implementar o algoritmo do produtor/consumidor de
// Tanenbaum com N consumidores e N produtores, declaramos dois vetores
// de pthread_t de tamanho N, um para os consumidores e outro para os
// produtores. Alem disso, introduzimos dois novos semaforos mutex, um utilizado
// na funcao producer e um na funcao consumer. Eles sao necessarios para que
// sejam produzidos e consumidos exatamente o numero de itens especificado,
// ja que um produtor/consumidor que ja esta executando o laco while original
// pode sofrer preempcao e nao alterar as variaveis produced/consumed
// (respectivamente), o que pode permirtir que outra thread passe no teste
// do while mesmo que isso seja errado. Por isso, as checagens dos whiles
// sao feitas em ifs separados, enquanto os lacos while tem como argumento
// o numero 1 para sempre executarem. A checagem dos ifs eh envolvida
// pelos semaforos citados. Dessa forma, asseguramos que apenas um produtor
// ou apenas um consumidor estejam "acordados" por vez

// para compilar: gcc prodcons_n_thread_sem.c -o prodcons_n_thread_sem -pthread
// para executar: prodcons_n_thread_sem
//

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#define MAX_PRODUCED 3157
#define MAX_QUEUE 527
#define N 113

// mutex_critico eh o mutex originalmente usado, enquanto
// mutex_produtor e mutex_consumidor foram adicionados
sem_t mutex_critico, empty, full, mutex_produtor, mutex_consumidor;

int queue[MAX_QUEUE], item_available=0, produced=0, consumed=0;

int create_item(void) {
	
	return(rand()%1000);
} //fim create_item()

void insert_into_queue(int item) {
	
	queue[item_available++] = item;
	produced++;
	printf("producing item:%d, value:%d, queued:%d \n", produced, item, item_available);
	return;
} 
// fim insert_into_queue()
int extract_from_queue() {
	consumed++;
	printf("consuming item:%d, value:%d, queued:%d \n", consumed, queue[item_available-1], item_available-1);
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
		// mutex auxiliar que permite que apenas um produtor esteja
		// "acordado" por vez
		sem_wait(&mutex_produtor);
		
		// Condicao de checagem é movida para um if envolvido por um novo
		// semaforo para evitar que um novo item seja produzido em excesso
		// (caso haja uma preempcao de uma thread produtora e a nova thread
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
	
	while (1) {
		// O mesmo ocorre com o consumidor: o mutex_consumidor eh
		// utilizado para assegurar que apenas um consumidor seja
		// executado por vez
		sem_wait(&mutex_consumidor);

		// Condicao de checagem é movida para um if envolvido por um novo
		// semaforo para evitar que um item inexistente seja consumido, o
		// que causaria que o consumidor "dormisse" para sempre
		// (caso haja uma preempcao de uma thread consumidora e a nova thread
		// seja outra consumidora, e caso so existir um item restante, a thread
		// que sofreu preempcao dormiria para sempre)
		if(consumed < MAX_PRODUCED) {
			sem_wait(&full);
			sem_wait(&mutex_critico);
			my_item = extract_from_queue();
			sem_post(&mutex_critico);
			sem_post(&empty);
			process_item(my_item);
			sem_post(&mutex_consumidor);
		}
		else {
			sem_post(&mutex_consumidor);
			printf("\nThread consumer saindo.\n\n");
			fflush(0);
			pthread_exit(0);
		}
	}
} // fim_consumer

int main(void) {

	// Vetores para threads dos produtores e consumidores
	pthread_t prod_handle[N], cons_handle[N];
	int i;

	/* declarations and initializations */
	item_available = 0;
	sem_init (&mutex_critico, 0 , 1);
	sem_init(&empty, 0, MAX_QUEUE);
	sem_init(&full, 0, 0);
	sem_init (&mutex_produtor, 0 , 1);
	sem_init (&mutex_consumidor, 0 , 1);
	
	/* create and join producer and consumer threads */
	for(i = 0; i < N; i++) {
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
	for(i = 0; i < N; i++) {
		pthread_join(prod_handle[i], 0);
		pthread_join(cons_handle[i], 0);
	}
	getchar();
	printf("Thread pai saindo.\n");
	fflush(0);
	exit(0);
} // main()