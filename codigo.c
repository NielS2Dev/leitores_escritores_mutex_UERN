/*
 * Leitores-Escritores com MUTEXES (POSIX threads)
 * Trio 04 - Sistemas Operacionais - UERN
 *
 * Estrategia: um mutex protege o estado de controle + variavel de condicao
 * para dormir (sem busy waiting). Cada thread pega uma "ficha" (FIFO),
 * entao ninguem fica esperando para sempre (sem starvation).
 *
 * Compilar:  gcc -Wall -pthread leitores_escritores.c -o le
 * Caos:      gcc -Wall -pthread -DSEM_SINCRONIZACAO leitores_escritores.c -o le_caos
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define N_LEITORES   5
#define N_ESCRITORES 2
#ifndef ITERACOES
#define ITERACOES    50      /* loops por thread */
#endif

/* ---- Recurso compartilhado: 'a' e 'b' devem ser sempre iguais ---- */
typedef struct { int a, b; } Dado;
static Dado dado = {0, 0};

/* ---- Controle de sincronizacao ---- */
static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  c = PTHREAD_COND_INITIALIZER;
static int leitores_ativos = 0, escritor_ativo = 0;
static unsigned prox_ficha = 0, vez = 0;   /* fila FIFO */
static int lendo = 0;                      /* so para o log */

static void pausa(unsigned *seed, int min_ms, int max_ms) {
    usleep((min_ms + rand_r(seed) % (max_ms - min_ms + 1)) * 1000);
}

static void entrar_leitura(void) {
#ifndef SEM_SINCRONIZACAO
    pthread_mutex_lock(&m);
    unsigned ficha = prox_ficha++;
    while (ficha != vez || escritor_ativo)       /* dorme, nao gasta CPU */
        pthread_cond_wait(&c, &m);
    leitores_ativos++;
    vez++;                                       /* libera o proximo da fila */
    pthread_cond_broadcast(&c);
    pthread_mutex_unlock(&m);
#endif
}

static void sair_leitura(void) {
#ifndef SEM_SINCRONIZACAO
    pthread_mutex_lock(&m);
    leitores_ativos--;
    pthread_cond_broadcast(&c);
    pthread_mutex_unlock(&m);
#endif
}

static void entrar_escrita(void) {
#ifndef SEM_SINCRONIZACAO
    pthread_mutex_lock(&m);
    unsigned ficha = prox_ficha++;
    while (ficha != vez || escritor_ativo || leitores_ativos > 0)
        pthread_cond_wait(&c, &m);
    escritor_ativo = 1;
    vez++;
    pthread_mutex_unlock(&m);
#endif
}

static void sair_escrita(void) {
#ifndef SEM_SINCRONIZACAO
    pthread_mutex_lock(&m);
    escritor_ativo = 0;
    pthread_cond_broadcast(&c);
    pthread_mutex_unlock(&m);
#endif
}

static void *leitor(void *arg) {
    int id = (int)(long)arg;
    unsigned seed = 100 + id;
    for (int i = 0; i < ITERACOES; i++) {
        pausa(&seed, 50, 200);                   /* tempo fora da secao critica */
        printf("[?] Leitor %d quer ler...\n", id);
        entrar_leitura();
        int n = __atomic_add_fetch(&lendo, 1, __ATOMIC_SEQ_CST);
        int a = dado.a;
        printf("[R] Leitor %d lendo (leitores simultaneos: %d)\n", id, n);
        pausa(&seed, 100, 200);                  /* tempo dentro da secao critica */
        int b = dado.b;
        if (a != b)
            printf("[X] Leitor %d: DADO INCONSISTENTE (a=%d, b=%d)\n", id, a, b);
        __atomic_sub_fetch(&lendo, 1, __ATOMIC_SEQ_CST);
        sair_leitura();
    }
    return NULL;
}

static void *escritor(void *arg) {
    int id = (int)(long)arg;
    unsigned seed = 500 + id;
    for (int i = 0; i < ITERACOES; i++) {
        pausa(&seed, 100, 300);
        printf("[?] Escritor %d quer escrever...\n", id);
        entrar_escrita();
        int valor = id * 1000 + i;
        printf("[W] Escritor %d ESCREVENDO %d (acesso exclusivo)\n", id, valor);
        dado.a = valor;
        pausa(&seed, 100, 200);                  /* escrita "demorada" em 2 etapas */
        dado.b = valor;
        printf("[+] Escritor %d terminou\n", id);
        sair_escrita();
    }
    return NULL;
}

int main(void) {
    pthread_t tl[N_LEITORES], te[N_ESCRITORES];
    setvbuf(stdout, NULL, _IOLBF, 0);
#ifdef SEM_SINCRONIZACAO
    printf("=== MODO CAOS: SEM MUTEX ===\n");
#else
    printf("=== Leitores-Escritores com Mutex ===\n");
#endif
    for (long i = 0; i < N_LEITORES; i++)   pthread_create(&tl[i], NULL, leitor, (void *)(i + 1));
    for (long i = 0; i < N_ESCRITORES; i++) pthread_create(&te[i], NULL, escritor, (void *)(i + 1));
    for (int i = 0; i < N_LEITORES; i++)    pthread_join(tl[i], NULL);
    for (int i = 0; i < N_ESCRITORES; i++)  pthread_join(te[i], NULL);
    printf("=== Fim: todas as threads terminaram (sem deadlock) ===\n");
    return 0;
}