#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include "ppos.h"
#include "ppos_data.h"
#include "ppos_disk.h"
#include "disk.h"

extern queue_t *readyQueue;
extern task_t *runningTask;
extern task_t *suspendedQueue;
request_t *requestsQueue; 
task_t *suspendedDiskReqQueue;
task_t mngDiskTask;
disk_t disk;
int signal_disk;
struct sigaction action2;


void handle_signal () {
    signal_disk = 1;
    if (mngDiskTask.status == SUSPENDED){
        task_resume(&mngDiskTask, &suspendedQueue);
    }
}

request_t *request(requestStatusT reqType, void *buf, int block) {
    request_t *req = malloc (sizeof (request_t));
    if (!req) return 0;

    req->req = runningTask;
	req->type = reqType;
	req->buffer = buf;
	req->block = block;
    req->next = req->prev = NULL;

    return req;
}

void print_queue_element_teste(void *ptr)
{
    task_t *task = ptr;
    printf("{ '%d': { 'D':%d,'S':%d}}", task->id, task->dynamicPriority, task->staticPriority); // D = DYNAMIC, S = STATIC
    if (task->next != (task_t *) readyQueue){
        printf(", "); 
    }
}


void diskDriverBody(void * args) {
    
    while (1) {
        sem_down(&disk.sem);  // obtém o semáforo de acesso ao disco

        // se foi acordado devido a um sinal do disco
        if (signal_disk) {
            task_t *task = disk.request->req;
            // acorda a tarefa cujo pedido foi atendido
            task_resume((task_t *)task,(task_t**) &suspendedDiskReqQueue );
            signal_disk = 0;
        }
        int status_disk = disk_cmd (DISK_CMD_STATUS, 0, 0);
        
        // se o disco estiver livre e houver pedidos de E/S na fila
        if ((status_disk == DISK_STATUS_IDLE) && (requestsQueue)) {
            // escolhe na fila o pedido a ser atendido, usando FCFS
            disk.request = requestsQueue;

            // solicita ao disco a operação de E/S, usando disk_cmd()
            if (disk.request->type == READ)
                disk_cmd (DISK_CMD_READ, disk.request->block, disk.request->buffer);
            else
                disk_cmd (DISK_CMD_WRITE, disk.request->block, disk.request->buffer);
            
            // printf("Atendendo ao request %d ",requestsQueue->block);
            queue_remove((queue_t **)&requestsQueue,(queue_t *)requestsQueue);

        }
        // libera o semáforo de acesso ao disco
        sem_up(&disk.sem);  

        // suspende a tarefa corrente (retorna ao dispatcher)
        task_suspend(&suspendedQueue);
    }
}

int disk_mgr_init (int *numBlocks, int *blockSize) {
    // inicializa disco 
    if (disk_cmd(DISK_CMD_INIT, 0, 0) || ((*numBlocks = disk_cmd(DISK_CMD_DISKSIZE, 0, 0)) < 0) || ((*blockSize = disk_cmd(DISK_CMD_BLOCKSIZE, 0, 0)) < 0))
        return -1;
    
    signal_disk = 0;
    sem_init(&disk.sem, 1);
    task_init(&mngDiskTask, diskDriverBody, NULL);

    action2.sa_handler = handle_signal;
    sigemptyset (&action2.sa_mask) ;
    action2.sa_flags = 0 ;
    if (sigaction (SIGUSR1, &action2, 0) < 0) {
        perror ("Erro em sigaction: ");
        exit(1);
    }

    return 0;
}

int disk_block_read (int block, void *buf) {
    sem_down(&disk.sem);   // obtém o semáforo de acesso ao disco
    
    request_t *req = request(READ, buf, block); // cria o pedido
    if (!req) return -1;
    // inclui o pedido na fila_disco    
    queue_append((queue_t **) &requestsQueue, (queue_t *) req);

    // acorda o gerente de disco (põe ele na fila de prontas)
    if (mngDiskTask.status == SUSPENDED){
        task_resume(&mngDiskTask, &suspendedQueue);
    }

    sem_up(&disk.sem);    // libera semáforo de acesso ao disco
    
    // suspende a tarefa corrente (retorna ao dispatcher)
    task_suspend(&suspendedDiskReqQueue);

    return 0;
}

int disk_block_write (int block, void *buf) {
    sem_down(&disk.sem);

    request_t *req = request(WRITE, buf, block);
    if (!req) return -1;
    // inclui o pedido na fila_disco    
    queue_append((queue_t **) &requestsQueue, (queue_t *) req);

    // acorda o gerente de disco (põe ele na fila de prontas)
    if (mngDiskTask.status == SUSPENDED){
        task_resume(&mngDiskTask, &suspendedQueue);
    }

    sem_up(&disk.sem);    // libera semáforo de acesso ao disco
    
    // suspende a tarefa corrente (retorna ao dispatcher)
    task_suspend(&suspendedDiskReqQueue);

    return 0;
}

