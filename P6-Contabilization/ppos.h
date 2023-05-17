// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.5 -- Março de 2023

// ESTE ARQUIVO NÃO DEVE SER MODIFICADO - ELE SERÁ SOBRESCRITO NOS TESTES

// Interface do núcleo para as aplicações

#ifndef __PPOS__
#define __PPOS__

#include "ppos_data.h"		// estruturas de dados necessárias

// macros importantes ==========================================================

// habilita compatibilidade POSIX no MacOS X (para ucontext.h)
#define _XOPEN_SOURCE 600

// este código deve ser compilado em sistemas UNIX-like
#if defined(_WIN32) || (!defined(__unix__) && !defined(__unix) && (!defined(__APPLE__) || !defined(__MACH__)))
#warning Este codigo foi planejado para ambientes UNIX (LInux, *BSD, MacOS). A compilacao e execucao em outros ambientes e responsabilidade do usuario.
#endif

// otimizações podem atrapalhar código que manipula contexto
#ifdef __OPTIMIZE__
#error "Please do not use optimizations (-O1, -O2, ...)"
#endif

// funções gerais ==============================================================

// Inicializa o sistema operacional; deve ser chamada no inicio do main()
void ppos_init () ;

// gerência de tarefas =========================================================

// Inicializa uma nova tarefa. Retorna um ID> 0 ou erro.
int task_init (task_t *task,			// descritor da nova tarefa
               void  (*start_func)(void *),	// funcao corpo da tarefa
               void   *arg) ;			// argumentos para a tarefa

// retorna o identificador da tarefa corrente (main deve ser 0)
int task_id () ;

// Termina a tarefa corrente com um valor de status de encerramento
void task_exit (int exit_code) ;

// alterna a execução para a tarefa indicada
int task_switch (task_t *task) ;

// suspende a tarefa atual,
// transferindo-a da fila de prontas para a fila "queue"
void task_suspend (task_t **queue) ;

// acorda a tarefa indicada,
// trasferindo-a da fila "queue" para a fila de prontas
void task_resume (task_t *task, task_t **queue) ;

// operações de escalonamento ==================================================

// a tarefa atual libera o processador para outra tarefa
void task_yield () ;

// define a prioridade estática de uma tarefa (ou a tarefa atual)
void task_setprio (task_t *task, int prio) ;

// retorna a prioridade estática de uma tarefa (ou a tarefa atual)
int task_getprio (task_t *task) ;

// operações de gestão do tempo ================================================

// retorna o relógio atual (em milisegundos)
unsigned int systime () ;

// suspende a tarefa corrente por t milissegundos
void task_sleep (int t) ;

// operações de sincronização ==================================================

// a tarefa corrente aguarda o encerramento de outra task
int task_wait (task_t *task) ;

// inicializa um semáforo com valor inicial "value"
int sem_init (semaphore_t *s, int value) ;

// requisita o semáforo
int sem_down (semaphore_t *s) ;

// libera o semáforo
int sem_up (semaphore_t *s) ;

// "destroi" o semáforo, liberando as tarefas bloqueadas
int sem_destroy (semaphore_t *s) ;

// inicializa um mutex (sempre inicialmente livre)
int mutex_init (mutex_t *m) ;

// requisita o mutex
int mutex_lock (mutex_t *m) ;

// libera o mutex
int mutex_unlock (mutex_t *m) ;

// "destroi" o mutex, liberando as tarefas bloqueadas
int mutex_destroy (mutex_t *m) ;

// inicializa uma barreira para N tarefas
int barrier_init (barrier_t *b, int N) ;

// espera na barreira
int barrier_wait (barrier_t *b) ;

// destrói a barreira, liberando as tarefas
int barrier_destroy (barrier_t *b) ;

// operações de comunicação ====================================================

// inicializa uma fila para até max mensagens de size bytes cada
int mqueue_init (mqueue_t *queue, int max, int size) ;

// envia uma mensagem para a fila
int mqueue_send (mqueue_t *queue, void *msg) ;

// recebe uma mensagem da fila
int mqueue_recv (mqueue_t *queue, void *msg) ;

// destroi a fila, liberando as tarefas bloqueadas
int mqueue_destroy (mqueue_t *queue) ;

// informa o número de mensagens atualmente na fila
int mqueue_msgs (mqueue_t *queue) ;

//==============================================================================

// Redefinir funcoes POSIX "proibidas" como "FORBIDDEN" (gera erro ao compilar)

// POSIX threads
#define pthread_create			FORBIDDEN
#define pthread_join			FORBIDDEN
#define pthread_exit			FORBIDDEN

// POSIX semaphores
//#define sem_init			FORBIDDEN
#define sem_post			FORBIDDEN
#define sem_wait			FORBIDDEN
#define sem_trywait			FORBIDDEN

// POSIX message queues
#define mq_open				FORBIDDEN
#define mq_send				FORBIDDEN
#define mq_receive			FORBIDDEN
#define mq_close			FORBIDDEN

// POSIX condvars
#define pthread_cond_init		FORBIDDEN
#define pthread_cond_wait		FORBIDDEN
#define pthread_cond_signal		FORBIDDEN
#define pthread_cond_timedwait		FORBIDDEN

// POSIX barriers
#define pthread_barrier_init		FORBIDDEN
#define pthread_barrier_wait		FORBIDDEN
#define pthread_barrier_destroy		FORBIDDEN

// POSIX mutexes
#define pthread_mutex_init		FORBIDDEN
#define pthread_mutex_lock		FORBIDDEN
#define pthread_mutex_unlock		FORBIDDEN
#define pthread_mutex_timedlock		FORBIDDEN
#define pthread_mutex_trylock		FORBIDDEN
#define pthread_mutex_destroy		FORBIDDEN

// POSIX RW-locks
#define pthread_rwlock_init		FORBIDDEN
#define pthread_rwlock_rdlock		FORBIDDEN
#define pthread_rwlock_wrlock		FORBIDDEN
#define pthread_rwlock_unlock		FORBIDDEN
#define pthread_rwlock_tryrdlock	FORBIDDEN
#define pthread_rwlock_tryrwlock	FORBIDDEN
#define pthread_rwlock_timedrdlock	FORBIDDEN
#define pthread_rwlock_timedrwlock	FORBIDDEN
#define pthread_rwlock_destroy		FORBIDDEN

// POSIX spinlocks
#define pthread_spin_init		FORBIDDEN
#define pthread_spin_lock		FORBIDDEN
#define pthread_spin_unlock		FORBIDDEN
#define pthread_spin_trylock		FORBIDDEN
#define pthread_spin_destroy		FORBIDDEN

#endif
