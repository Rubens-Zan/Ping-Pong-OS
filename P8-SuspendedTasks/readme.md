O objetivo deste projeto é construir funções para suspender e acordar tarefas. A principal função a implementar é task_wait, que permite a uma tarefa suspender-se para esperar a conclusão de outra, de forma similar às chamadas POSIX wait e pthread_join.

A chamada task_wait (b) faz com que a tarefa atual (corrente) seja suspensa até a conclusão da tarefa b. Mais tarde, quando a tarefa b encerrar (usando a chamada task_exit), a tarefa suspensa deve retornar à fila de tarefas prontas. Lembre-se que várias tarefas podem ficar aguardando que a tarefa b encerre, então todas elas têm de ser acordadas quando isso ocorrer.

