 Este projeto tem por objetivo implementar operações de entrada/saída (leitura e escrita) de blocos de dados sobre um disco rígido virtual. A execução dessas operações estarão a cargo de um gerente de disco, que cumpre a função de driver de acesso ao disco. 

  A gerência das operações de entrada/saída em disco consiste em implementar:

    Uma tarefa gerenciadora do disco;
    Uma função para tratar os sinais SIGUSR1 gerados pelo disco, que acorda a tarefa gerenciadora de disco quando necessário;
    Uma fila de pedidos de acesso ao disco; cada pedido indica a tarefa solicitante, o tipo de pedido (leitura ou escrita), o bloco desejado e o endereço do buffer de dados;
    As funções de acesso ao disco oferecidas às tarefas (disk_mgr_init, disk_block_read e disk_block_write).
