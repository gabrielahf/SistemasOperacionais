#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // Para a função usleep()

// --- 1. Definições de Estruturas e Constantes ---

// Enum para os estados dos processos
typedef enum {
    PRONTO,
    EXECUTANDO,
    BLOQUEADO,
    FINALIZADO
} EstadoProcesso;

// Estrutura para representar o Processo (PCB)
typedef struct Processo {
    char nome[50];
    int tempo_total_cpu;
    int surto_cpu;
    int tempo_es;
    int prioridade;
    
    int tempo_restante_cpu;
    EstadoProcesso estado;
    int tempo_no_surto_atual;
    int tempo_em_es;
    int fila_de_origem; // 0 para Fila0, 1 para Fila1, 2 para Fila2
} Processo;

// Nó da lista ligada para a fila
typedef struct Node {
    Processo* processo;
    struct Node* next;
} Node;

// Estrutura da Fila
typedef struct Queue {
    Node* front;
    Node* rear;
} Queue;

// Estrutura do Escalonador
typedef struct Escalonador {
    int quantum0;
    int quantum1;
    
    Queue* fila0;
    Queue* fila1;
    Queue* fila2;
    Queue* fila_bloqueados;
    
    Processo* processos_finalizados[100]; // Array estático para simplicidade
    int finalizados_count;
    
    Processo* processo_em_execucao;
    int tempo_quantum_atual;
} Escalonador;


// --- 2. Funções de Manipulação da Fila ---

// Cria uma nova fila vazia
Queue* criar_fila() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    return q;
}

// Adiciona um processo ao final da fila
void enqueue(Queue* q, Processo* p) {
    Node* novo_node = (Node*)malloc(sizeof(Node));
    novo_node->processo = p;
    novo_node->next = NULL;
    if (q->rear == NULL) {
        q->front = q->rear = novo_node;
        return;
    }
    q->rear->next = novo_node;
    q->rear = novo_node;
}

// Remove e retorna um processo do início da fila
Processo* dequeue(Queue* q) {
    if (q->front == NULL) return NULL;
    Node* temp = q->front;
    Processo* p = temp->processo;
    q->front = q->front->next;
    if (q->front == NULL) q->rear = NULL;
    free(temp);
    return p;
}

// Remove um processo específico da fila (usado para a fila de bloqueados)
void remover_da_fila(Queue* q, Processo* p) {
    Node *temp = q->front, *prev = NULL;
    if (temp != NULL && temp->processo == p) {
        q->front = temp->next;
        if (q->front == NULL) q->rear = NULL;
        free(temp);
        return;
    }
    while (temp != NULL && temp->processo != p) {
        prev = temp;
        temp = temp->next;
    }
    if (temp == NULL) return;
    prev->next = temp->next;
    if (prev->next == NULL) q->rear = prev;
    free(temp);
}

int fila_vazia(Queue* q) {
    return q->front == NULL;
}

void imprimir_fila(Queue* q) {
    Node* temp = q->front;
    while (temp != NULL) {
        printf("'%s' ", temp->processo->nome);
        temp = temp->next;
    }
}

// --- 3. Funções do Escalonador ---

// Inicializa o escalonador
Escalonador* escalonador_criar(int quantum0, int quantum1) {
    Escalonador* esc = (Escalonador*)malloc(sizeof(Escalonador));
    esc->quantum0 = quantum0;
    esc->quantum1 = quantum1;
    esc->fila0 = criar_fila();
    esc->fila1 = criar_fila();
    esc->fila2 = criar_fila();
    esc->fila_bloqueados = criar_fila();
    esc->finalizados_count = 0;
    esc->processo_em_execucao = NULL;
    esc->tempo_quantum_atual = 0;
    return esc;
}

// Adiciona processos iniciais, ordenando por prioridade
void escalonador_adicionar_processos(Escalonador* esc, Processo* processos[], int count) {
    // Ordenação simples (Bubble Sort) por prioridade
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (processos[j]->prioridade > processos[j + 1]->prioridade) {
                Processo* temp = processos[j];
                processos[j] = processos[j + 1];
                processos[j + 1] = temp;
            }
        }
    }
    for (int i = 0; i < count; i++) {
        enqueue(esc->fila0, processos[i]);
    }
}

void escalonador_escalonar_novo_processo(Escalonador* esc) {
    if (!fila_vazia(esc->fila0)) {
        esc->processo_em_execucao = dequeue(esc->fila0);
        esc->processo_em_execucao->fila_de_origem = 0;
    } else if (!fila_vazia(esc->fila1)) {
        esc->processo_em_execucao = dequeue(esc->fila1);
        esc->processo_em_execucao->fila_de_origem = 1;
    } else if (!fila_vazia(esc->fila2)) {
        esc->processo_em_execucao = dequeue(esc->fila2);
        esc->processo_em_execucao->fila_de_origem = 2;
    } else {
        esc->processo_em_execucao = NULL;
        return;
    }
    esc->processo_em_execucao->estado = EXECUTANDO;
    esc->tempo_quantum_atual = 0;
}

void escalonador_verificar_processos_bloqueados(Escalonador* esc) {
    if (fila_vazia(esc->fila_bloqueados)) return;
    
    Node* atual = esc->fila_bloqueados->front;
    while(atual != NULL) {
        Processo* p = atual->processo;
        p->tempo_em_es++;
        Node* proximo = atual->next; // Salva o próximo antes de possivelmente remover o atual
        
        if (p->tempo_em_es >= p->tempo_es) {
            p->estado = PRONTO;
            p->tempo_em_es = 0;
            remover_da_fila(esc->fila_bloqueados, p);

            if (p->fila_de_origem == 0) enqueue(esc->fila0, p);
            else if (p->fila_de_origem == 1) enqueue(esc->fila1, p);
            else enqueue(esc->fila2, p);
        }
        atual = proximo;
    }
}

int escalonador_tem_processos_ativos(Escalonador* esc) {
    return !fila_vazia(esc->fila0) || !fila_vazia(esc->fila1) || !fila_vazia(esc->fila2) || 
           !fila_vazia(esc->fila_bloqueados) || esc->processo_em_execucao != NULL;
}

void escalonador_executar_passo_de_tempo(Escalonador* esc) {
    escalonador_verificar_processos_bloqueados(esc);

    if (esc->processo_em_execucao == NULL) {
        escalonador_escalonar_novo_processo(esc);
        return;
    }

    Processo* p_exec = esc->processo_em_execucao;
    p_exec->tempo_restante_cpu--;
    p_exec->tempo_no_surto_atual++;
    esc->tempo_quantum_atual++;

    if (p_exec->tempo_restante_cpu == 0) {
        p_exec->estado = FINALIZADO;
        esc->processos_finalizados[esc->finalizados_count++] = p_exec;
        esc->processo_em_execucao = NULL;
        return;
    }

    if (p_exec->surto_cpu > 0 && p_exec->tempo_no_surto_atual == p_exec->surto_cpu) {
        p_exec->estado = BLOQUEADO;
        p_exec->tempo_no_surto_atual = 0;
        enqueue(esc->fila_bloqueados, p_exec);
        esc->processo_em_execucao = NULL;
        return;
    }

    int quantum_da_fila = 0;
    if (p_exec->fila_de_origem == 0) quantum_da_fila = esc->quantum0;
    else if (p_exec->fila_de_origem == 1) quantum_da_fila = esc->quantum1;
    
    if (quantum_da_fila > 0 && esc->tempo_quantum_atual == quantum_da_fila) {
        p_exec->estado = PRONTO;
        if (p_exec->fila_de_origem == 0) enqueue(esc->fila1, p_exec);
        else if (p_exec->fila_de_origem == 1) enqueue(esc->fila2, p_exec);
        esc->processo_em_execucao = NULL;
    }
}

void escalonador_destruir(Escalonador* esc) {
    // Libera memória de todos os processos e nós das filas
    // (Omissão para brevidade, mas crucial em um projeto real)
    free(esc->fila0);
    free(esc->fila1);
    free(esc->fila2);
    free(esc->fila_bloqueados);
    free(esc);
}

// --- 4. Função Main (Simulador) ---

int main() {
    // Configurações do simulador
    int QUANTUM_FILA0 = 6;
    int QUANTUM_FILA1 = 12;
    int NUM_PROCESSOS = 5;

    // Criação dos processos
    Processo p1 = {"P1", 20, 5, 10, 1, 20, PRONTO, 0, 0, 0};
    Processo p2 = {"P2", 25, 8, 15, 0, 25, PRONTO, 0, 0, 0}; // Prioridade maior
    Processo p3 = {"P3", 10, 0, 0,  2, 10, PRONTO, 0, 0, 0}; // CPU-bound
    Processo p4 = {"P4", 30, 7, 12, 1, 30, PRONTO, 0, 0, 0};
    Processo p5 = {"P5", 15, 4, 8,  3, 15, PRONTO, 0, 0, 0};

    Processo* lista_processos[] = {&p1, &p2, &p3, &p4, &p5};

    Escalonador* escalonador = escalonador_criar(QUANTUM_FILA0, QUANTUM_FILA1);
    escalonador_adicionar_processos(escalonador, lista_processos, NUM_PROCESSOS);
    
    int tempo_global = 0;
    printf("--- Início da Simulação ---\n");

    while (escalonador_tem_processos_ativos(escalonador)) {
        printf("\n--- Tempo: %dms ---\n", tempo_global);

        escalonador_executar_passo_de_tempo(escalonador);
        
        printf("CPU: %s\n", escalonador->processo_em_execucao ? escalonador->processo_em_execucao->nome : "Ociosa");
        printf("Fila 0 (RR Q=%d): ", escalonador->quantum0); imprimir_fila(escalonador->fila0); printf("\n");
        printf("Fila 1 (RR Q=%d): ", escalonador->quantum1); imprimir_fila(escalonador->fila1); printf("\n");
        printf("Fila 2 (FCFS): "); imprimir_fila(escalonador->fila2); printf("\n");
        printf("Bloqueados: "); imprimir_fila(escalonador->fila_bloqueados); printf("\n");

        tempo_global++;
        usleep(50000); // Pausa de 50ms para visualização
    }

    printf("\n--- Fim da Simulação ---\n");
    printf("Tempo total de execução: %dms\n", tempo_global - 1);
    printf("Processos finalizados:\n");
    for (int i = 0; i < escalonador->finalizados_count; i++) {
        printf("- %s\n", escalonador->processos_finalizados[i]->nome);
    }
    
    // Libera a memória alocada para o escalonador e suas filas
    escalonador_destruir(escalonador);

    return 0;
}
