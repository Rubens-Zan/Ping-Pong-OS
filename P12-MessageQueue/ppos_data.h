// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.5 -- Março de 2023

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto
#include "queue.h"

#define STACKSIZE 64*1024	/* tamanho de pilha das threads */

typedef enum  {
  READY,
  RUNNING,
  SUSPENDED,
  COMPLETED 
} taskStatusT;

// Algumas contanstes definidas para facilitacao de leitura do codigo,debugs e manutencao
#define ERROR_CODE -1 // Codigo de erro, para facilitar na leitura/manutencao
#define MIN_PRIORITY 20 // Lembrando que nos sistemas Windows eh o inverso, mas estou utilizando o padrao POSIX 
#define MAX_PRIORITY -20  // e as prioridades vao de -20 a 20 no Posix
#define TASK_AGING -1 // Fator de envelhecimento das tarefas 
#define DEFAULT_PRIORITY 0 // Definindo a prioridade inicial das tarefas como padrao de 0
#define QUANTUM_TIME 20 // O quantum das tarefas eh de 20 ticks de relogio
#define TICK_HANDLER_FREQUENCY 1000 // tempo para chamada da funcao que lida com ticks do relogio em microsegundos, por referencia 1000 microsegundos = 1 milisegundo 

// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
  struct task_t *prev, *next ;		// ponteiros para usar em filas
  int id ;				// identificador da tarefa
  ucontext_t context ;			// contexto armazenado da tarefa
  short status ;			// pronta, rodando, suspensa, ...
  short staticPriority, dynamicPriority;
  short isInUserSpace; // indica se eh uma tarefa do espaço do usuario para ser preemptada ou nao
  unsigned int processorTime;
  unsigned int executionTime;
  unsigned int activations;
  struct task_t *awaitingTasksQueue; // fila de tarefas esperando que essa tarefa termine para serem acordadas
  unsigned int alarmTime; //  representa o tempo do sistema que essa tarefa deve ser acordada, se suspensa por tempo 
} task_t ;

// estrutura que define um semáforo
typedef struct
{
  int counter; // contador do semaforo
  task_t *queue; // Fila de aguardo do semaforo caso hajam tarefas aguardando
  int cslock; // Lock da secao critica do semaforo, utilizado para que tarefas distintas consigam lidar com a estrutura do semaforo atraves de funcoes de entrada e saida na secao critica 
  short isActive; // representa se o semaforo esta ativo, ou seja n foi destruido
} semaphore_t ;

// estrutura que define um mutex
typedef struct
{
  // preencher quando necessário
} mutex_t ;

// estrutura que define uma barreira
typedef struct
{
  // preencher quando necessário
} barrier_t ;

// estrutura que define uma fila de mensagens
typedef struct
{
  int counter; // contador de tarefas no buffer corrente
  int maxMsgs; // maximo de mensagens disponiveis no buffer da fila
  int msgSize; // tamanho da mensagem contida na fila de mensagens, para que o buffer de mensagens seja generico
  short isActive; // representa se a fila de mensagens esta ativa, ou seja nao foi destruido
  semaphore_t semProducts; // Semaforo contendo a quantidade de produtos produzidos atualmente na fila, usado para controlar o fluxo de consumidores
  semaphore_t semProductionsSpots; // Semaforo contendo os espacos de producao, para que sejam controlados os produtores, tal que eles nao produzam mais mensagens do que os slots maximos disponiveis
  void * buffer; // buffer de mensagens da fila de mensagens, void * para tipo do buffer de mensagens ser generico 
  // variaveis auxiliares pois a implementacao eh com um buffer circular
  int lastConsumptionIdx; // index do ultimo consumo do buffer da fila
  int lastProductionIdx;  // index da ultima producao do buffer da fila
} mqueue_t ;

#endif

