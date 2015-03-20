//
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
// do while mesmo que isso seja errado. Assim, os mutex_produced e mutex_consumed
// sao utilizados para que a checagem das variaveis produced e consumed,
// respectivamente, seja feita com exclusao mutua. Isso faz com que o valor
// dessas variaveis esteja sempre atualizado quando uma thread verificar as
// condicoes do while.

// para compilar: gcc prodcons_n_thread_sem.c -o prodcons_n_thread_sem -pthread
// para executar: prodcons_n_thread_sem
//
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#define N 19
#define MAX_PRODUCED 3907
#define MAX_QUEUE 149

sem_t  mutex, empty, full, mutex_produced, mutex_consumed;

int queue[MAX_QUEUE], item_available=0, produced=0, consumed=0;


int create_item(void) {

	return(rand()%1000);

} //fim  create_item()

void insert_into_queue(int item) {
	
	queue[item_available++] = item;
	// produced++ acontece agora na funcao producer, já que as threads concorrem por essa
	// variavel. Dessa forma, a checagem produced < MAX_PRODUCED sempre tera o valor produced
	// atualizado
	printf("producing item:%d, value:%d, queued:%d \n", produced, item, item_available); 
	return;

} // fim insert_into_queue()

int extract_from_queue() {

	// consumed++ acontece agora na funcao consumer, já que as threads concorrem por essa
	// variavel. Dessa forma, a checagem consumed < MAX_PRODUCED sempre tera o valor consumed
	// atualizado
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

	// Semaforo eh utilizado aqui para a checagem da primeira iteracao do laco while
	sem_wait(&mutex_produced);
	while (produced < MAX_PRODUCED ) {
	  // Se a checagem acima resultar for verdadeira, um novo item sera produzido e assim
	  // produced deve ser incrementada; isso acontece com exclusao mutua para que a
	  // produced seja sempre lida com o valor atualizado (quando uma thread estiver
	  // dentro do laco e for produzir um item, outra thread nao entre no laco tambem caso
	  // a primeira sofra preempcao antes de produzir o item e o numero maximo de itens
	  // produzidos estiver prestes a ser ultrapassado)
	  produced++;
	  // Semaforo liberado pois produced ja foi alterada
	  sem_post(&mutex_produced);
	  item = create_item();
	  sem_wait(&empty);
	  sem_wait(&mutex);
	  insert_into_queue(item);
	  sem_post(&mutex);
	  sem_post(&full);
	  // Exclusao mutua na checagem e modificacao de produced comeca abaixo: o semaforo
	  // eh usado para que apenas uma thread por vez leia e possivelmente altere a
	  // variavel produced
	  sem_wait(&mutex_produced);
	}

	// Liberacao do semaforo
	sem_post(&mutex_produced);
	printf("\nThread producer saindo.\n\n");
	fflush(0);

	pthread_exit(0);
} // fim producer

void *consumer(void) {
	int my_item = 0;

	// Algo analogo ao caso do produtor acontece para o consumidor:
	// consumed eh uma variavel concorrida pelas threads consumidoras e deve
	// haver exclusao mutua na sua checagem e possivel alteracao de valor (caso
	// a condicao do loop while seja verdadeira)
	sem_wait(&mutex_consumed);
	while (consumed < MAX_PRODUCED) {
	  consumed++;
	  sem_post(&mutex_consumed);
	  sem_wait(&full);
	  sem_wait(&mutex);
	  my_item = extract_from_queue();
	  sem_post(&mutex);
	  sem_post(&empty);
	  process_item(my_item);
	  sem_wait(&mutex_consumed);
	}

	// Liberacao do semaforo
	sem_post(&mutex_consumed);
	printf("\nThread consumer saindo.\n\n");
	fflush(0);
	
	pthread_exit(0);
} // fim_consumer


int main(void) {
	// Vetores para threads dos produtores e consumidores
	pthread_t prod_handle[N], cons_handle[N];
	int i;

	/* declarations and initializations */
	item_available = 0;
	sem_init (&mutex, 0 , 1);
	sem_init(&empty, 0, MAX_QUEUE);
	sem_init(&full, 0, 0);
	sem_init (&mutex_produced, 0 , 1);
	sem_init (&mutex_consumed, 0 , 1);
	
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