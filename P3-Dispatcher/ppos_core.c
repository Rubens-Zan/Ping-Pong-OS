// GRR20206147 Rubens Zandomenighi Laszlo 

#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"
#include "queue.h"

task_t *runningTask,mainTask, dispatcherTask; // ponteiro para a tarefa corrente e para a main
queue_t *readyQueue; 
unsigned int taskCounter; // contador para inicializacao das tarefas
unsigned int remainingTasks; // contador para tarefas restantes


// Funcao para escalonamento da tarefa utilizando a politica FCFS (~FIFO)
task_t *scheduler(){
    return (task_t *) readyQueue; // como a politica eh do tipo FCFS, retorna a cabeca da fila de tarefas prontas  
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
        ++taskCounter; // Incrementa contador para que a task inicializada seja a ultima
        ++remainingTasks; 
        queue_append(&readyQueue, (queue_t *) task);  // aqui pode usar a task e fazer o cast pois os primeiros tres campos da estrutura da task sao os mesmo do queue_t
    }else {
        task->id = DISPATCHER_ID; // Serve para debugs
    }

    makecontext(&(task->context), (void (*)(void))start_func, 1, arg); // inicia o contexto da tarefa com a funcao indicada e argumentos recebidos


    #ifdef DEBUG
    printf("PPOS: task_init() Task  %d initialized\n", task->id);
    #endif

    return task->id;
} ;

int task_switch (task_t *task){
    task_t *prevTask = runningTask; // guarda em variavel auxiliar a tarefa corrente que vai ser a anterior para que consiga alterar o ptr da tarefa corrente para a nova tarefa
    if (runningTask != task){
        // if (task != &dispatcherTask){ // se a tarefa a ser executada for o dispatcher a anterior continuar como completada pois veio do task_exit
            // prevTask->status = READY; // a tarefa que estava em execucao vai para o status de pronta
        // }
        task->status = RUNNING; // a tarefa que vai entrar em execucao recebe o status de em execucao 
        #ifdef DEBUG
        printf("PPOS: task_switch() Running task switched %d -> %d\n", runningTask->id,task->id);
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

int task_id(){
    return runningTask->id; 
};

// a tarefa volta ao final da fila de prontas, devolvendo o processador ao dispatcher
void task_yield(){
    runningTask->status = READY; 
    readyQueue = readyQueue->next; // a tarefa volta ao final da fila de tarefas prontas
    #ifdef DEBUG
        printf("PPOS: task_yield() Running task altered to dispatcher\n");
    #endif
    task_switch(&dispatcherTask); 
}

