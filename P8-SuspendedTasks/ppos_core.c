// GRR20206147 Rubens Zandomenighi Laszlo 

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include "ppos.h"
#include "queue.h"

task_t *runningTask,mainTask, dispatcherTask; // ponteiro para a tarefa corrente e para a main
queue_t *readyQueue; 
int taskCounter; // contador para inicializacao das tarefas
unsigned int remainingTasks; // contador para tarefas restantes
unsigned int quantumTimer; // contador de quantum para a tarefa corrente

unsigned int clockTicks; // Variavel que conta tick do relogio decorridos (tempo em ms)

struct sigaction action; // estrutura que define um tratador de sinal (deve ser global ou static)
struct itimerval timer;  // estrutura que define um timer


void print_queue_element(void *ptr)
{
    task_t *task = ptr;
    printf("{ '%d': { 'D':%d,'S':%d}}", task->id, task->dynamicPriority, task->staticPriority); // D = DYNAMIC, S = STATIC
    if (task->next != (task_t *) readyQueue){
        printf(", "); 
    }
}

// Funcao para escalonamento da tarefa utilizando a politica por prioridades
task_t *scheduler(){
    task_t *biggestPriorityTask = (task_t *) readyQueue;
    int biggestPriorityTaskVal =  biggestPriorityTask->dynamicPriority;
    task_t *auxQueue = (task_t *) readyQueue; // inicio com a tarefa corrente, para que possa comparar e efetuar a troca de contexto apenas quando a prioridade for 'mais alta'
    
    // percorre a fila de tarefas prontas para achar a mais prioritaria, ja as envelhecendo
    do{ 
        int currentTaskPrio = auxQueue->dynamicPriority;

        if (currentTaskPrio != MAX_PRIORITY){
            if (currentTaskPrio + TASK_AGING < MAX_PRIORITY){
                auxQueue->dynamicPriority = MAX_PRIORITY;
            }
            else {
                auxQueue->dynamicPriority = currentTaskPrio + TASK_AGING;
            }
        }
        
        if (biggestPriorityTaskVal > currentTaskPrio ){ // a comparacao eh maior aqui pois quanto menor o numero da prioridade, mais prioritario eh a tarefa 
            biggestPriorityTask = auxQueue; 
            biggestPriorityTaskVal = currentTaskPrio;
        }

        auxQueue = auxQueue->next;
    } while (auxQueue != (task_t *) readyQueue); // para que efetue o aging na cabeca da fila tambem
    
    #ifdef DEBUG
        queue_print("PPOS: scheduler() Queue of ready tasks: ", (queue_t *)readyQueue, print_queue_element); // interessante para ver o funcionamento e task aging das tarefas
        printf("PPOS: scheduler() Task with biggest priority: %d \n",biggestPriorityTask->id);
    #endif
    return (task_t *) biggestPriorityTask;
}

void dispatcher(){
    while (remainingTasks > 0)
    {
        task_t *nextTask = scheduler(); 

        if (nextTask != NULL){

            task_switch(nextTask); 

            switch (nextTask->status) 
            {
            case READY:
                #ifdef DEBUG
                    printf("PPOS: dispatcher() Task %d ran and returned to ready queue, returning to static priority %d \n", nextTask->id,nextTask->staticPriority);
                #endif
                // como a proxima tarefa corrente posterior a tarefa escolhida pelo schedules vai ser o dispatcher perderia a referencia a essa tarefa
                // a tarefa que ganhou a cpu, volta a sua prioridade estatica, como vai ser incrementada pelo scheduler, eh decrementada de aging
                nextTask->dynamicPriority = nextTask->staticPriority - TASK_AGING; 
                break;
            case COMPLETED:

                #ifdef DEBUG
                    printf("PPOS: dispatcher() Task %d has been completed and removed from ready queue, remaining %d tasks \n", nextTask->id, remainingTasks);
                #endif
                break;
            case RUNNING:
            case SUSPENDED:
                /* code */
                break;
            default: 
                break;
            }
        }
    }
    
    task_exit(0); 
}


// Retorna a quantidade de ticks do relogio do sistema
unsigned int systime()
{
    return clockTicks;
}

// Funcao que lida com um tick do relogio do sistema
void tickHandler(int signum)
{
    ++runningTask->processorTime;
    ++clockTicks;
    // Checa se a tarefa corrente esta no userspace, pois se for tarefa do kernel, nao deve ter controle de preempcao
    if (runningTask->isInUserSpace)
    {
        --quantumTimer;
        
        // Se o quantum da tarefa acabou
        if (quantumTimer == 0)
        {
        #ifdef DEBUG
            printf("PPOS: tickHandler() Task %d quantum ended. Resetting the task timer.\n", runningTask->id);
        #endif
            task_yield(); // a tarefa que estourou o quantum, volta a fila de prontas
        }
        else
        {
        #ifdef DEBUG
            // printf("PPOS: tickHandler() Task %d still has %d ticks.\n", runningTask->id, quantumTimer); // printa o quantum da tarefa decrementando, polui muito o log, portanto deixei comentado
        #endif
        }
    }
}

void timer_init()
{
    action.sa_handler = tickHandler; // faz com que a funcao de tickHandler lide com as interrupcoes 
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGALRM, &action, 0) < 0)
    {
        perror("ERROR: timer_init() Error while testing sigaction\n");
        exit(EXIT_FAILURE);
    }
    timer.it_value.tv_usec = TICK_HANDLER_FREQUENCY; // seta o valor da proxima expiracao com o TICK_HANDLER_FREQUENCY
    timer.it_interval.tv_usec = TICK_HANDLER_FREQUENCY; // seta o valor do intervalo a ser executa com o mesmo tempoo, para ser executado de 1ms em 1ms 
    timer.it_value.tv_sec = 0;
    timer.it_interval.tv_sec = 0;
    if (setitimer(ITIMER_REAL, &timer, 0) < 0)
    {
        perror("ERROR: timer_init() Error on setting timer\n");
        exit(EXIT_FAILURE);
    }
}

void ppos_init (){
    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0) ; // evitar erros do printf com trocas de contexto
    
    // Inicializacao do contexto da task main
    if (getcontext(&(mainTask.context)) == -1)
    {
        perror("ERROR: Failed to get context for main task");
        exit(EXIT_FAILURE);
    }

    taskCounter = 0; // garantindo que o id da main sera sempre 0 
    mainTask.id = taskCounter; 
    mainTask.activations = 1;
    mainTask.isInUserSpace = 1;
    quantumTimer = QUANTUM_TIME; /// RESETA QUANTUM PARA A NOVA TAREFA A EXECUTAR

    remainingTasks = 1;
    ++taskCounter; 
    queue_append(&readyQueue, (queue_t *) &mainTask);
    clockTicks=0;
    #ifdef DEBUG
    printf("PPOS: System initialized \n");
    #endif
    runningTask = &mainTask;

    timer_init(); // inicia a contabilizacao de tempo 
    task_init(&dispatcherTask, dispatcher, NULL);
}


// Funcoes para lidar com as tarefas
int task_init (task_t *task,void  (*start_func)(void *),void   *arg){
    if (getcontext(&(task->context)) == -1){
        perror("ERROR: Failed to get context for task");
        exit(EXIT_FAILURE);
    }

    // inicia a pilha da tarefa
    char *stack = malloc (STACKSIZE) ;
    if (stack)
    {
        task->context.uc_stack.ss_sp = stack ;
        task->context.uc_stack.ss_size = STACKSIZE ;
        task->context.uc_stack.ss_flags = 0 ;
        task->context.uc_link = 0 ;
    }
    else
    {
        perror ("Erro na criação da pilha! ") ;
        return -1; // retorna negativo em caso de erro  
    }

    task->status = READY; // atribui status de pronta para executar a tarefa criada
    if (taskCounter != 1){ // se a tarefa nao eh o dispatcher
        task->dynamicPriority = task->staticPriority = DEFAULT_PRIORITY;
        task->isInUserSpace = 1; // como nao eh o dispatcher esta em tarefa de modo kernel, ou seja, nao preemptiva
        ++remainingTasks; 
        queue_append(&readyQueue, (queue_t *) task);  // aqui pode usar a task e fazer o cast pois os primeiros tres campos da estrutura da task sao os mesmo do queue_t    
    } else {
        task->staticPriority = MAX_PRIORITY; // Dispatcher eh o mais prioritario 
        task->dynamicPriority = MAX_PRIORITY;
        task->isInUserSpace = 0;
    }
    task->id = taskCounter; 
    ++taskCounter; // Incrementa contador para que a task inicializada seja a ultima
    task->activations=0;
    task->processorTime=0;
    task->executionTime = systime(); // inicio o tempo de execucao com a quantidade de ticks atuais do relogio, para que quando morrer faca execucao = tempo_morte - tempo_nascimento
    makecontext(&(task->context), (void (*)(void))start_func, 1, arg); // inicia o contexto da tarefa com a funcao indicada e argumentos recebidos


    #ifdef DEBUG
    printf("PPOS: task_init() Task  %d initialized with priority %d\n", task->id, task->staticPriority);
    #endif

    return task->id;
} ;

void task_resume (task_t * task, task_t **queue){
    // se a fila queue não for nula, retira a tarefa apontada por task dessa fila;
    if(queue != NULL)
        queue_remove((queue_t **) queue,(queue_t *)task);
    
    // ajusta o status dessa tarefa para “pronta”;
    task->status = READY;
    // insere a tarefa na fila de tarefas prontas.
    queue_append(&readyQueue, (queue_t *) task); 
};

void task_suspend (task_t **queue){
    #ifdef DEBUG
    printf("PPOS: task_suspend() Suspending task %d \n", runningTask->id);
    #endif
    queue_remove(&readyQueue,(queue_t *) runningTask); // Retira a tarefa atual da fila de tarefas prontas (se estiver nela);
    runningTask->status = SUSPENDED;  //  ajusta o status da tarefa atual para “suspensa”;
    if (queue != NULL){
        queue_append((queue_t **)queue,(queue_t *) runningTask); // Insere a tarefa atual na fila apontada por queue (se essa fila não for nula);
    }
    task_yield(); //  retorna ao dispatcher.
};

// Tarefa corrente aguarda a tarefa de parametro ser concluida
int task_wait (task_t *task){
    #ifdef DEBUG
    printf("PPOS: task_wait()   %d waiting for %d\n",runningTask->id,task->id );
    #endif
    if (task != NULL && task->status != COMPLETED){
        task_suspend( &task->awaitingTasksQueue); 
    }else {
        return -1; // retorna negativo pois tarefa a ser aguardada nao esta viva
    }

    return task->id;
}

int task_switch (task_t *task){
    task_t *prevTask = runningTask; // guarda em variavel auxiliar a tarefa corrente que vai ser a anterior para que consiga alterar o ptr da tarefa corrente para a nova tarefa
    if (runningTask != task){
        task->status = RUNNING; // a tarefa que vai entrar em execucao recebe o status de em execucao 
        prevTask->dynamicPriority = prevTask->staticPriority;
        task->dynamicPriority = task->staticPriority;
        ++task->activations;
        quantumTimer = QUANTUM_TIME; /// RESETA QUANTUM PARA A NOVA TAREFA A EXECUTAR
        #ifdef DEBUG
        printf("PPOS: task_switch() Running task switched %d -> %d\n", runningTask->id,task->id);
        #endif
        runningTask = task; // altera o ponteiro para a tarefa corrente da anterior para a atual
        swapcontext (&(prevTask->context), &(task->context)) ; // altera o contexto da tarefa corrente para a nova tarefa a ser a corrente, guarda o contexto anterior na tarefa anterior apontada
    }
    return 0;
};

void task_exit (int exit_code){
    runningTask->executionTime = systime() - runningTask->executionTime;
    printf("Task %d exit: execution time %d ms,processor time %d ms, %d activations \n",runningTask->id, runningTask->executionTime, runningTask->processorTime,runningTask->activations);

    if (runningTask->awaitingTasksQueue){ // se tem alguem na fila de aguardando fim dessa tarefa
        task_t *auxQueue = (task_t *) runningTask->awaitingTasksQueue;
        task_t *auxQueueNext;
        task_t *start = auxQueue;
        do{ 
            auxQueueNext = auxQueue->next; 
            task_resume(auxQueue,&runningTask->awaitingTasksQueue); 
            auxQueue = auxQueueNext;
        } while (auxQueue != start ); // para que efetue o loop na cabeca da fila tambem
    }   

    if (runningTask != &dispatcherTask){

        // removo no task exit, para que quando o dispatcher rode o scheduler, a main ja nao esteja na fila 
        queue_remove(&readyQueue,(queue_t *) runningTask); // caso a tarefa foi completa remove da fila de prontas
        --remainingTasks; 

        runningTask->status = COMPLETED; // caso deu task_exit quer dizer que a tarefa terminou 
        #ifdef DEBUG
        printf("PPOS: task_exit() Running task %d exited with code %d, returning to dispatcher \n", runningTask->id, exit_code);
        #endif
        task_switch(&dispatcherTask) ; // tarefa encerrada, controle volta para o dispatcher 
        free(runningTask->context.uc_stack.ss_sp); // limpa a stack alocada a tarefa

    }else {
        #ifdef DEBUG
        printf("PPOS: task_exit() Dispatcher task terminated \n");
        #endif   
        free(runningTask->context.uc_stack.ss_sp); // limpa a stack alocada a tarefa

        exit(EXIT_SUCCESS);
    }
};


int task_getprio (task_t *task){
    return (task != NULL ? task->staticPriority : runningTask->staticPriority); 
}

void task_setprio (task_t *task, int prio){
    #ifdef DEBUG
    printf("PPOS: task_setprio() Task %d received priority %d \n", task->id, prio);
    #endif
    if (! (prio >= MAX_PRIORITY && prio <= MIN_PRIORITY)){
        fprintf(stderr, "ERRO: A prioridade esta fora dos limites devidos!\n");  // Caso a prioridade passada, nao estiver entre -20 e 20 retorna erro
        exit(EXIT_FAILURE);
    }

    if (task != NULL )
        task->staticPriority = task->dynamicPriority = prio;
    else
        runningTask->staticPriority = runningTask->dynamicPriority = prio; 
}


int task_id(){
    return runningTask->id; 
};

void task_yield(){
    // a tarefa volta a fila de prontas, devolvendo o processador ao dispatcher
    runningTask->status = READY; 
    #ifdef DEBUG
    printf("PPOS: task_yield() Running task altered to dispatcher\n");
    #endif
    task_switch(&dispatcherTask); 
}

