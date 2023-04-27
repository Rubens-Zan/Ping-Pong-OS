// GRR20206147 Rubens Zandomenighi Laszlo 

#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"
#include "queue.h"

task_t *runningTask,mainTask, dispatcherTask; // ponteiro para a tarefa corrente e para a main
queue_t *readyQueue; 
unsigned int taskCounter; // contador para inicializacao das tarefas
unsigned int remainingTasks; // contador para tarefas restantes

// Funcao para escalonamento da tarefa utilizando a politica por prioridades
task_t *scheduler(){
    task_t *biggestPriorityTask = (task_t *) readyQueue;
    int biggestPriorityVal =  biggestPriorityTask->dynamicPriority;
    task_t *auxQueue = (task_t *) readyQueue;

    // percorre a fila de tarefas prontas para achar a mais prioritaria, ja as envelhecendo
    do{ /// enquanto a lista circular nao voltou ao inicio 
        int currentTaskPrio = auxQueue->dynamicPriority;

        if (biggestPriorityVal > currentTaskPrio ){ // a comparacao eh maior aqui pois quanto menor o numero da prioridade, mais prioritario eh a tarefa 
            biggestPriorityTask = auxQueue; 
            biggestPriorityVal = currentTaskPrio;
        }
        
        if (currentTaskPrio != MAX_PRIORITY){
            if (currentTaskPrio + TASK_AGING < MAX_PRIORITY){
                auxQueue->dynamicPriority = MAX_PRIORITY;
                // printf("task: id %d %d  ",auxQueue->id ,auxQueue->dynamicPriority);
            }
            else {
                // printf("task: id %d %d  ",auxQueue->id ,auxQueue->dynamicPriority);
                auxQueue->dynamicPriority = currentTaskPrio + TASK_AGING;
            }

        }

        auxQueue = auxQueue->next;

    } while (auxQueue != (task_t *) readyQueue); // para que efetue o aging na cabeca da fila tambem

    return (task_t *) biggestPriorityTask; // como a politica eh do tipo FCFS, retorna a cabeca da fila de tarefas prontas  
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
                    printf("PPOS: dispatcher() Task %d ran and returned to ready queue, returning to dynamic priority to static %d \n", nextTask->id,nextTask->staticPriority);
                #endif
                // task_setprio(nextTask, nextTask->staticPriority); // A tarefa que recebeu retorna a prioridade padrao 
                // nextTask->staticPriority = nextTask->dynamicPriority;
                break;
            case COMPLETED:
                queue_remove(&readyQueue,(queue_t *) nextTask); // caso a tarefa foi completa remove da fila de prontas
                --remainingTasks; 
                free(nextTask->context.uc_stack.ss_sp);

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



void ppos_init (){
    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0) ; // evitar erros do printf com trocas de contexto
    
    // Inicializacao do contexto da task main
    if (getcontext(&(mainTask.context)) == -1)
    {
        perror("ERROR: Failed to get context for main task");
        exit(1);
    }

    taskCounter = 0; // garantindo que o id da main sera sempre 0 
    mainTask.id = taskCounter; 
    remainingTasks = 1;
    ++taskCounter; 

    queue_append(&readyQueue, (queue_t *) &mainTask);

    #ifdef DEBUG
    printf("PPOS: System initialized \n");
    #endif
    runningTask = &mainTask;
    task_init(&dispatcherTask, dispatcher, NULL);
    task_switch(&dispatcherTask);
}


// Funcoes para lidar com as tarefas

int task_init (task_t *task,void  (*start_func)(void *),void   *arg){
    if (getcontext(&(task->context)) == -1){
        perror("ERROR: Failed to get context for task");
        exit(1);
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
    if (task != &dispatcherTask){
        task->id = taskCounter; 
        task->dynamicPriority = DEFAULT_PRIORITY;
        task->staticPriority = DEFAULT_PRIORITY;

        ++taskCounter; // Incrementa contador para que a task inicializada seja a ultima
        ++remainingTasks; 
        queue_append(&readyQueue, (queue_t *) task);  // aqui pode usar a task e fazer o cast pois os primeiros tres campos da estrutura da task sao os mesmo do queue_t
    }else {
        task->id = DISPATCHER_ID; // Serve para debugs
        task->staticPriority = MAX_PRIORITY; // Dispatcher eh o mais prioritario 
        task->dynamicPriority = MAX_PRIORITY;
    }

    makecontext(&(task->context), (void (*)(void))start_func, 1, arg); // inicia o contexto da tarefa com a funcao indicada e argumentos recebidos


    #ifdef DEBUG
    printf("PPOS: task_init() Task  %d initialized %d with priority\n", task->id, task->staticPriority);
    #endif

    return task->id;
} ;

int task_switch (task_t *task){
    task_t *prevTask = runningTask; // guarda em variavel auxiliar a tarefa corrente que vai ser a anterior para que consiga alterar o ptr da tarefa corrente para a nova tarefa
    if (runningTask != task){
        task->status = RUNNING; // a tarefa que vai entrar em execucao recebe o status de em execucao 
        prevTask->dynamicPriority =prevTask->staticPriority;

        #ifdef DEBUG
        // printf("PPOS: task_switch() Running task switched %d -> %d\n", runningTask->id,task->id);
        #endif
        runningTask = task; // altera o ponteiro para a tarefa corrente da anterior para a atual
        swapcontext (&(prevTask->context), &(task->context)) ; // altera o contexto da tarefa corrente para a nova tarefa a ser a corrente, guarda o contexto anterior na tarefa anterior apontada
    }
    return 0;
};

void task_exit (int exit_code){

    runningTask->status = COMPLETED; // como estamos tratando de uma politica FCFS colaborativa, o estado da tarefa vai ser completa
    #ifdef DEBUG
    printf("PPOS: task_exit() Running task switched to dispatcher \n");
    #endif
    task_switch(&dispatcherTask) ; // tarefa encerrada, controle volta para o dispatcher 
};


int task_getprio (task_t *task){
    return (task != NULL ? task->staticPriority : runningTask->staticPriority); 
}

void task_setprio (task_t *task, int prio){
    #ifdef DEBUG
    printf("PPOS: task_setprio() Task %d received priority %d \n", task->id, prio);
    #endif
    if (task != NULL )
        task->staticPriority = prio;
    else
        runningTask->staticPriority = prio; 
}


int task_id(){
    return runningTask->id; 
};

void task_yield(){
    // a tarefa volta ao final da fila de prontas, devolvendo o processador ao dispatcher
    runningTask->status = READY; 
    readyQueue = readyQueue->next; // a tarefa volta ao final da fila de tarefas prontas
    #ifdef DEBUG
        printf("PPOS: task_yield() Running task altered to dispatcher\n");
    #endif
    task_switch(&dispatcherTask); 
}

