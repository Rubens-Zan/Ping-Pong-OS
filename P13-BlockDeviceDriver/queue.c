// GRR20206147 Rubens Zandomenighi Laszlo 
#include <stdio.h>
#include <stdlib.h>

#include "queue.h"

int queue_size (queue_t *queue){
    unsigned int size = queue != NULL ? 1 : 0;  

    if (size){
        queue_t *auxQueue = queue->next;
        while (auxQueue != queue){
            ++size; 
            auxQueue = auxQueue->next;
        }
    }

    return size; 
};

void queue_print (char *name, queue_t *queue, void print_elem (void*) ){
    printf("%s: ", name);
    printf("[");
    
    if (queue != NULL){

        print_elem(queue);

        queue_t *auxQueue = queue->next;

        while (auxQueue != queue){ /// enquanto a lista circular nao voltou ao inicio 
            printf(" "); 
            print_elem(auxQueue);  
            auxQueue = auxQueue->next;
        } 
    }

    printf("] \n");   
};

int isElemInQueue(queue_t *queue,queue_t *elem){
    unsigned int isInQueue = 0;
    
    queue_t *auxQueue = queue->next;
    if (queue == elem){
        isInQueue = 1;
    }
    while (auxQueue != queue && !isInQueue){
        if (auxQueue == elem){
            isInQueue = 1;
        }
        auxQueue = auxQueue->next;
    }
    return isInQueue;
}

int queue_append (queue_t **queue, queue_t *elem){
    // - a fila deve existir
    if (queue == NULL) {
        fprintf(stderr, "ERRO: A fila nao existe!\n"); 
        return -1;
    }

    // - o elemento deve existir
    if (elem == NULL){ 
        fprintf(stderr, "ERRO: Elemento nao existe!\n"); 
        return -2;
    }
    
    // - o elemento nao deve estar em outra fila
    if (elem->prev != NULL || elem->next != NULL){ 
        fprintf(stderr, "ERRO: Elemento faz parte de outra fila!\n"); 
        return -3;
    }

    if (*queue == NULL) { // se a fila esta vazia
        elem->prev = elem->next = elem; 
        (*queue) = elem;
    }
    else {
        queue_t *lastElem = (*queue)->prev; // lastElem aponta para o ultimo elemento pela circularidade
        elem->next = lastElem->next; 
        lastElem->next = elem;
        elem->prev = lastElem;
        (*queue)->prev = elem;
    }

    return 0;
}; 

int queue_remove (queue_t **queue, queue_t *elem){
    // - a fila deve existir
    if ((*queue) == NULL){
        fprintf(stderr, "ERRO: A fila nao existe!\n"); 
        return -1;
    }

    // - a fila nao deve estar vazia
    if((*queue)->next == NULL){
        fprintf(stderr, "ERRO: A fila esta vazia!\n"); 
        return -2;
    }
    // - o elemento deve existir
    if (elem == NULL){
        fprintf(stderr, "ERRO: O elemento nao existe!\n"); 
        return -3;
    }
    // - o elemento deve pertencer a fila indicada
    if (!(isElemInQueue((*queue), elem))){
        fprintf(stderr, "ERRO: O elemento nao esta na fila!\n"); 
        return -4;
    }

    if ((*queue) == (*queue)->next){ // lista com um elemento
        (*queue)->prev = (*queue)->next = NULL;
        *queue = NULL;
    }
    else {                // elemento esta em primeiro na lista
        if (*queue == elem){
            *queue = (*queue)->next; 
        }
        queue_t *nextElem = elem->next; 
        queue_t *prevElem = elem->prev; 
        
        prevElem->next = nextElem;  
        nextElem->prev = prevElem;             // elemento inicial eh o proximo 
    }
    elem->next = elem->prev = NULL; 

    return 0;
}; 
