#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> // Uso essa biblioteca para ter acesso a Mutex e Variáveis de Condição
#include <unistd.h>  // Necessário para a função usleep() que usei nas pausas

// Defini essas macros de cores ANSI pra deixar o log no terminal mais legível e bonito
#define RESET   "\x1b[0m"
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"

// Configurei o teste para 5 leitores e 2 escritores
#define N_LEITORES   5
#define N_ESCRITORES 2
#ifndef ITERACOES
#define ITERACOES    50 // Escolhemos 50 repetições para provar que o código não cai em deadlock
#endif

// AQUI FICA O NOSSO RECURSO COMPARTILHADO
// Criei essa struct com duas variáveis para ficar fácil de corromper e mostrar o erro no Modo Caos
typedef struct { int a, b; } Dado;
static Dado dado = {0, 0};

// NOSSAS PRIMITIVAS DE SINCRONIZAÇÃO
// Escolhi usar apenas UM mutex focado em proteger nosso estado interno
static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
// E uma variável de condição pra fazer a thread dormir em vez de gastar CPU com busy waiting
static pthread_cond_t  c = PTHREAD_COND_INITIALIZER;

// NOSSAS VARIÁVEIS DE ESTADO
static int leitores_ativos = 0, escritor_ativo = 0;

// SISTEMA DE FICHAS FIFO (A NOSSA SOLUÇÃO ANTI-INANIÇÃO)
// Pensei nisso como uma fila de banco: prox_ficha é o dispensador e vez é o painel
static unsigned prox_ficha = 0, vez = 0; 

// Uso essa variável só pra contar quantos leitores estão lendo juntos na hora do print
static int lendo = 0;

// Fiz essa função só pra limpar a tela e mostrar a nossa capa antes de começar
void mostrar_intro() {
    system("clear"); 
    printf(RED "Prof.ª Me. Gláucia Melissa Medeiros Campos\n");
    printf("Prof.ª Artemísia Kimberlly Silva\n" RESET);
    printf("\n");
    printf(GREEN "  _    _ ______ _____  _    _ \n");
    printf(" | |  | |  ____|  __ \\| \\ | |\n");
    printf(" | |  | | |__  | |__) |  \\| |\n");
    printf(" | |  | |  __| |  _  /| . ` |\n");
    printf(" | |__| | |____| | \\ \\| |\\  |\n");
    printf("  \\____/|______|_|  \\_\\_| \\_|\n" RESET);
    printf("\n");
    printf(YELLOW "Alunos:\n");
    printf(RESET "- Antoniel\n");
    printf("- Geovanne\n");
    printf("- Sayna\n");
    printf("\n");
    printf(MAGENTA "Pressione [ENTER] para iniciar a execucao..." RESET);
    getchar(); 
    printf("\n");
}

// Criei essa função de pausa propositalmente para forçar o Sistema Operacional a trocar o contexto.
// Sem isso, é muito difícil a condição de corrida aparecer no terminal.
static void pausa(unsigned *seed, int min_ms, int max_ms) {
    usleep((min_ms + rand_r(seed) % (max_ms - min_ms + 1)) * 1000);
}

// ==========================================
// A NOSSA LÓGICA DO LEITOR
// ==========================================
static void entrar_leitura(void) {
#ifndef SEM_SINCRONIZACAO
    pthread_mutex_lock(&m); // 1. Primeiro eu travo o mutex para pegar a ficha com segurança
    
    unsigned ficha = prox_ficha++; // 2. O leitor pega a ficha dele
    
    // 3. Aqui é o nosso teste: se não for a vez dele ou tiver um escritor, eu mando ele dormir
    while (ficha != vez || escritor_ativo)
        pthread_cond_wait(&c, &m); 
        
    // 4. Se ele passou do while, é a vez dele. Eu registro a entrada e avanço a fila
    leitores_ativos++;
    vez++; 
    
    // 5. Dou um broadcast pra acordar a próxima thread que pode estar esperando a vez dela
    pthread_cond_broadcast(&c); 
    
    // 6. O PULO DO GATO: Eu escolhi dar o unlock AQUI, antes de ler de fato. 
    // É isso que permite que a nossa solução aceite vários leitores ao mesmo tempo.
    pthread_mutex_unlock(&m); 
#endif
}

static void sair_leitura(void) {
#ifndef SEM_SINCRONIZACAO
    pthread_mutex_lock(&m);      // Travo pra alterar os contadores
    leitores_ativos--;           // O leitor avisa que terminou
    pthread_cond_broadcast(&c);  // Grito pro SO acordar quem tá esperando (pode ser um escritor)
    pthread_mutex_unlock(&m);    // Libero o mutex
#endif
}

// ==========================================
// A NOSSA LÓGICA DO ESCRITOR
// ==========================================
static void entrar_escrita(void) {
#ifndef SEM_SINCRONIZACAO
    pthread_mutex_lock(&m); 
    
    unsigned ficha = prox_ficha++; 
    
    // Pro escritor eu fui mais rigoroso: ele só sai do while se for a vez dele
    // E (muito importante) não pode ter NENHUM escritor nem NENHUM leitor ativo.
    while (ficha != vez || escritor_ativo || leitores_ativos > 0)
        pthread_cond_wait(&c, &m);
        
    // Se chegou aqui, ele tem acesso exclusivo. Travo a base para os outros.
    escritor_ativo = 1; 
    vez++;              // Avanço a fila
    pthread_mutex_unlock(&m); 
#endif
}

static void sair_escrita(void) {
#ifndef SEM_SINCRONIZACAO
    pthread_mutex_lock(&m);      
    escritor_ativo = 0;          // Libero a flag do escritor
    pthread_cond_broadcast(&c);  // Acordo a galera da fila
    pthread_mutex_unlock(&m);    
#endif
}

// ==========================================
// O QUE AS THREADS REALMENTE FAZEM
// ==========================================
static void *leitor(void *arg) {
    int id = (int)(long)arg; 
    unsigned seed = 100 + id;
    
    for (int i = 0; i < ITERACOES; i++) {
        pausa(&seed, 50, 200); 
        printf(YELLOW "[?] Leitor %d quer ler...\n" RESET, id);
        
        entrar_leitura(); // Pede permissão pela nossa arquitetura
        
        // --- SEÇÃO CRÍTICA (LENDO) ---
        int n = __atomic_add_fetch(&lendo, 1, __ATOMIC_SEQ_CST); 
        
        int a = dado.a; // Leio a primeira metade
        printf(CYAN "[R] Leitor %d lendo (leitores simultaneos: %d)\n" RESET, id, n);
        
        pausa(&seed, 100, 200); // Aqui é onde eu abro brecha pro Modo Caos corromper o dado
        
        int b = dado.b; // Leio a segunda metade
        
        // Se 'a' for diferente de 'b', provo que nossa sincronização falhou (útil pro Teste do Caos)
        if (a != b)
            printf(RED "[X] Leitor %d: DADO INCONSISTENTE (a=%d, b=%d)\n" RESET, id, a, b);
            
        __atomic_sub_fetch(&lendo, 1, __ATOMIC_SEQ_CST); 
        // --- FIM DA SEÇÃO CRÍTICA ---
        
        sair_leitura(); 
    }
    return NULL;
}

static void *escritor(void *arg) {
    int id = (int)(long)arg; 
    unsigned seed = 500 + id;
    
    for (int i = 0; i < ITERACOES; i++) {
        pausa(&seed, 100, 300); 
        printf(MAGENTA "[?] Escritor %d quer escrever...\n" RESET, id);
        
        entrar_escrita(); // Garante que só ele vai passar daqui
        
        // --- SEÇÃO CRÍTICA EXCLUSIVA (ESCREVENDO) ---
        int valor = id * 1000 + i; 
        printf(GREEN "[W] Escritor %d ESCREVENDO %d (acesso exclusivo)\n" RESET, id, valor);
        
        dado.a = valor; // Grava a primeira metade
        pausa(&seed, 100, 200); // Sofre o risco de preempção do SO
        dado.b = valor; // Grava a segunda metade
        // --- FIM DA SEÇÃO CRÍTICA ---
        
        printf(GREEN "[+] Escritor %d terminou\n" RESET, id);
        sair_escrita(); 
    }
    return NULL;
}

// ==========================================
// FUNÇÃO MAIN
// ==========================================
int main(void) {
    mostrar_intro(); 
    
    pthread_t tl[N_LEITORES], te[N_ESCRITORES];
    setvbuf(stdout, NULL, _IOLBF, 0); // Ajuste que fiz pro print não bugar com as threads
    
// Coloquei esse ifdef pra gente poder compilar o código no Modo Caos sem precisar alterar o código fonte
#ifdef SEM_SINCRONIZACAO 
    printf(RED "=== MODO CAOS: SEM MUTEX ===\n" RESET);
#else
    printf(BLUE "=== Leitores-Escritores com Mutex ===\n" RESET);
#endif

    // Disparando as threads dos leitores e escritores
    for (long i = 0; i < N_LEITORES; i++)   
        pthread_create(&tl[i], NULL, leitor, (void *)(i + 1));
        
    for (long i = 0; i < N_ESCRITORES; i++) 
        pthread_create(&te[i], NULL, escritor, (void *)(i + 1));

    // Uso o join pra amarrar a main aqui, obrigando ela a esperar todo mundo terminar
    for (int i = 0; i < N_LEITORES; i++)    
        pthread_join(tl[i], NULL);
        
    for (int i = 0; i < N_ESCRITORES; i++)  
        pthread_join(te[i], NULL);
    
    // Se a execução chegou nessa linha, significa que nosso algoritmo deu certo:
    // Ninguém sofreu inanição (starvation) e não rolou Deadlock!
    printf(BLUE "=== Fim: todas as threads terminaram ===\n" RESET);
    return 0;
}