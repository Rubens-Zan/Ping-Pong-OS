#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include "ppos.h"
#include "ppos_data.h"
#include "ppos_disk.h"
#include "disk.h"

extern queue_t *readyQueue;
extern task_t *runningTask;
task_t *suspendedQueueMngDisk;

request_t *requestsQueue; 
task_t *suspendedDiskReqQueue;
task_t mngDiskTask;
disk_t disk;
int signal_disk;
struct sigaction action2;
semaphore_t semDiskMgr;
int diskMgrSuspended; 

void handle_signal () {
    signal_disk = 1;
    sem_down(&semDiskMgr);

    if (diskMgrSuspended){
        task_resume(&mngDiskTask, &suspendedQueueMngDisk);
        diskMgrSuspended = 0;
    }
    sem_up(&semDiskMgr);

}

request_t *request(requestStatusT reqType, void *buf, int block) {
    request_t *req = malloc (sizeof (request_t));
    if (!req) return 0;

    req->task = runningTask;
    req->task->isInUserSpace = 0;
	req->type = reqType;
	req->buffer = buf;
	req->block = block;
    req->next = req->prev = NULL;

    return req;
}

void diskDriverBody(void * args) {
    while (1) {
        sem_down(&disk.sem);  // obtém o semáforo de acesso ao disco

        // se foi acordado devido a um sinal do disco
        if (signal_disk) {
            request_t *requestCalled = disk.request;
            // acorda a tarefa cujo pedido foi atendido
            task_resume((task_t *)requestCalled->task,(task_t**) &suspendedDiskReqQueue );
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
            
            if(queue_remove((queue_t **)&requestsQueue,(queue_t *)disk.request)!= 0 )
                perror("ERROR: diskDriverBody() Erro ao remover requestsQueue\n");
        }
        // libera o semáforo de acesso ao disco
        sem_up(&disk.sem);  

        // suspende a tarefa corrente (retorna ao dispatcher)
        mngDiskTask.status = SUSPENDED;
        diskMgrSuspended = 1;
        task_suspend(&suspendedQueueMngDisk);
    }
}

int disk_mgr_init (int *numBlocks, int *blockSize) {
    // inicializa disco 
    if (disk_cmd(DISK_CMD_INIT, 0, 0) || ((*numBlocks = disk_cmd(DISK_CMD_DISKSIZE, 0, 0)) < 0) || ((*blockSize = disk_cmd(DISK_CMD_BLOCKSIZE, 0, 0)) < 0))
        return -1;
    
    signal_disk = 0;
    sem_init(&disk.sem, 1);
    sem_init(&semDiskMgr, 1);
    diskMgrSuspended =1;
    task_init(&mngDiskTask, diskDriverBody, NULL);
    mngDiskTask.status = SUSPENDED;
    queue_append((queue_t **) &suspendedQueueMngDisk, (queue_t *) &mngDiskTask);  // aqui pode usar a task e fazer o cast pois os primeiros tres campos da estrutura da task sao os mesmo do queue_t    
    mngDiskTask.staticPriority = mngDiskTask.dynamicPriority = MAX_PRIORITY;
    
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
    if(queue_append((queue_t **) &requestsQueue, (queue_t *) req) != 0){
        perror("ERROR: disk_block_read() Erro ao incluir o pedido na fila_disco\n");
        return -1;
    }

    // acorda o gerente de disco (põe ele na fila de prontas)
    sem_down(&semDiskMgr);

    if (diskMgrSuspended){
        task_resume(&mngDiskTask, &suspendedQueueMngDisk);
        diskMgrSuspended = 0;
    }
    sem_up(&semDiskMgr);

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
    if(queue_append((queue_t **) &requestsQueue, (queue_t *) req)!=0){
        perror("ERROR: disk_block_write() Erro ao incluir o pedido na fila_disco\n");
        return -1;
    }

    // acorda o gerente de disco (põe ele na fila de prontas)
    sem_down(&semDiskMgr);

    if (diskMgrSuspended){
        task_resume(&mngDiskTask, &suspendedQueueMngDisk);
        diskMgrSuspended = 0;
    }
    sem_up(&semDiskMgr);

    sem_up(&disk.sem);    // libera semáforo de acesso ao disco
    
    // suspende a tarefa corrente (retorna ao dispatcher)
    task_suspend(&suspendedDiskReqQueue);
    return 0;
}

