// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.5 -- Março de 2023

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include <ucontext.h>		// biblioteca POSIX de trocas de contexto

#define STACKSIZE 64*1024	/* tamanho de pilha das threads */

typedef enum  {
  READY,
  RUNNING,
  SUSPENDED,
  COMPLETED 
} taskStatusT;

// Algumas contanstes definidas para facilitacao de leitura do codigo,debugs e manutencao
#define DISPATCHER_ID -1 // ID do dispatcher, para facilitar na leitura dos debugs
#define MIN_PRIORITY 20 // Lembrando que nos sistemas Windows eh o inverso, mas estou utilizando o padrao POSIX 
#define MAX_PRIORITY -20  // e as prioridades vao de -20 a 20 no Posix
#define TASK_AGING -1 // Fator de envelhecimento das tarefas 
#define DEFAULT_PRIORITY 0 // Definindo a prioridade inicial das tarefas como padrao de 0
#define QUANTUM_TIME 20 // O quantum das tarefas eh de 20 ticks de relogio
#define HANDLER_FREQUENCY 1000
// Estrutura que define um Task Control Block (TCB)
typedef struct task_t
{
  struct task_t *prev, *next ;		// ponteiros para usar em filas
  int id ;				// identificador da tarefa
  ucontext_t context ;			// contexto armazenado da tarefa
  short status ;			// pronta, rodando, suspensa, ...
  short staticPriority, dynamicPriority;
  short isInUserSpace; // indica se eh uma tarefa do espaço do usuario para ser preemptada ou nao
  // ... (outros campos serão adicionados mais tarde)
} task_t ;

// estrutura que define um semáforo
typedef struct
{
  // preencher quando necessário
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
  // preencher quando necessário
} mqueue_t ;

#endif

