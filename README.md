# T1: Implementação de um Escalonador de Processos

Este projeto é a implementação de um simulador para o escalonamento de processos em um sistema operacional hipotético, desenvolvido como parte do Trabalho 1 da disciplina de Sistemas Operacionais. 

O simulador gerencia o ciclo de vida dos processos, que podem se encontrar nos estados "Pronto, Executando, Bloqueado ou Finalizado". 

## Integrantes
* Arthur Blasi
* Gabriela Roxo
* Luis Trein

## Funcionalidades

O simulador implementa um algoritmo de **Escalonamento Multinível com Feedback** contendo três filas de execução.

* **Fila 0 (Alta Prioridade):** Utiliza o algoritmo `Round Robin com um quantum menor` (ex: 1ms a 10ms). Todos os processos novos são admitidos nesta fila. 
* **Fila 1 (Média Prioridade):** Utiliza o algoritmo `Round Robin com um quantum maior` (ex: 11ms a 20ms). Processos que não concluem sua execução na Fila 0 são movidos para cá. 
* **Fila 2 (Baixa Prioridade):** Utiliza o algoritmo `FCFS (First-Come, First-Served)`. Processos que não concluem nas filas anteriores são movidos para esta fila e executam até o término. 

### Regras de Prioridade e Comportamento
* As filas de menor prioridade só executam quando as filas de maior prioridade estão vazias. 
* O sistema suporta operações de Entrada e Saída (E/S).Quando um processo realiza uma E/S, ele é movido para o estado Bloqueado. 
* Após a conclusão da E/S, o processo retorna para o final da mesma fila onde estava antes de ser bloqueado. 

## Tecnologias Utilizadas

* Linguagem C

## Como Compilar e Executar

Navegue até o diretório do projeto e execute o comando abaixo para compilar o arquivo `simulador.c` e gerar um executável chamado `simulador`.
```bash
gcc simulador.c -o simulador
```

Para executar o simulador e ver a saída em tempo real no terminal, use o comando:
```bash
./simulador
```

Para uma análise mais detalhada, é recomendado salvar o log completo da simulação em um arquivo de texto.
```bash
./simulador > resultado_simulacao.txt
```
