// GRR20206147 Rubens Zandomenighi Laszlo 

#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"

task_t *runningTask,mainTask; // ponteiro para a tarefa corrente e para a main
unsigned int taskCounter; // contador para inicializacao das tarefas

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
    ++taskCounter; 

    runningTask = &mainTask; // a tarefa corrente eh inicializada com a main
    
    #ifdef DEBUG
    printf("PPOS: System initialized \n");
    #endif
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
    task->id = taskCounter; 
    ++taskCounter; // Incrementa contador para que a task inicializada seja a ultima
    makecontext(&(task->context), (void (*)(void))start_func, 1, arg); // inicia o contexto da tarefa com a funcao indicada e argumentos recebidos

    #ifdef DEBUG
    printf("PPOS: task_init() Task  %d initialized\n", task->id);
    #endif

    return task->id;
} ;

int task_switch (task_t *task){
    task_t *prevTask = runningTask; // guarda em variavel auxiliar a tarefa corrente que vai ser a anterior para que consiga alterar o ptr da tarefa corrente para a nova tarefa

    if (runningTask != task){
        prevTask->status = READY; // a tarefa que estava em execucao vai para o status de pronta
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

    runningTask->status = COMPLETED; 
    #ifdef DEBUG
    printf("PPOS: task_exit() Running task switched to main \n");
    #endif
    task_switch(&mainTask) ; // tarefa encerrada, controle volta pra main

};

int task_id(){
    return runningTask->id; 
};

