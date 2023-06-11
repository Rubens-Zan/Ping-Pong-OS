#include "ppos_disk.h"


// void diskDriverBody (void * args)
// {
//    while (true) 
//    {
//       // obtém o semáforo de acesso ao disco
 
//       // se foi acordado devido a um sinal do disco
//       if (disco gerou um sinal)
//       {
//          // acorda a tarefa cujo pedido foi atendido
//       }
 
//       // se o disco estiver livre e houver pedidos de E/S na fila
//       if (disco_livre && (fila_disco != NULL))
//       {
//          // escolhe na fila o pedido a ser atendido, usando FCFS
//          // solicita ao disco a operação de E/S, usando disk_cmd()
//       }
 
//       // libera o semáforo de acesso ao disco
 
//       // suspende a tarefa corrente (retorna ao dispatcher)
//    }
// }

// disk_block_read (block, &buffer)
// {
//    // obtém o semáforo de acesso ao disco
 
//    // inclui o pedido na fila_disco
 
//    if (gerente de disco está dormindo)
//    {
//       // acorda o gerente de disco (põe ele na fila de prontas)
//    }
 
//    // libera semáforo de acesso ao disco
 
//    // suspende a tarefa corrente (retorna ao dispatcher)
// }

