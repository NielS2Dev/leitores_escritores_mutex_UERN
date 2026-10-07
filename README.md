# 🧵 Leitores e Escritores com Mutexes (POSIX Threads)

<p align="center">
  <img src="https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white" alt="C" />
  <img src="https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black" alt="Linux" />
  <img src="https://img.shields.io/badge/POSIX_pthreads-000000?style=for-the-badge&logo=gnu&logoColor=white" alt="POSIX" />
  <img src="https://img.shields.io/badge/Status-Conclu%C3%ADdo-brightgreen?style=for-the-badge" alt="Status" />
</p>

> **Projeto Prático de Concorrência e Sincronização**  
> Disciplina: Sistemas Operacionais (SO)  
> Universidade do Estado do Rio Grande do Norte (UERN) — Campus Natal  
> Professora: Artemísia Kimberlly Silva  

---

## 👨‍💻 Trio 04

| Foto / Avatar | Integrante | GitHub |
| :---: | :--- | :--- |
| <img src="https://github.com/github.png" width="50"> | **Antoniel da Silva Alves** | [@seu-user](https://github me) |
| <img src="https://github.com/github.png" width="50"> | **Geovane Guilherme do Nascimento** | [@geovane2606](https://github.com/geovane2606) |
| <img src="https://github.com/github.png" width="50"> | **Sayan Bruno da Silva Soares** | [@sayanbruno](https://github.com/sayanbruno) |

---

## 📌 Visão Geral do Problema

O problema dos **Leitores e Escritores** é um modelo clássico de sincronização onde múltiplos fluxos de execução (*threads*) compartilham um mesmo recurso em memória RAM:
* **Leitores ($R$):** Apenas consultam os dados. Podem executar concorrentemente sem restrições.
* **Escritores ($W$):** Modificam o recurso compartilhado. Exigem **acesso exclusivo** (sem outros leitores nem outros escritores na seção crítica).


    <h1>ALTERAR AINDA</h1>

    Ainda irei fazer as inforamções necesárias aqui
apenas salvando enquanto isso.


novas modificação save

Para rodar o "Teste do Caos" (Sem Mutex):
gcc -Wall -pthread -DSEM_SINCRONIZACAO codigo.c -o le_caos
./le_caos


Para rodar o modo Normal (Com Mutex):
gcc -Wall -pthread codigo.c -o le
./le

    <h1>TERMINO</h1>
Sem mecanismos apropriados de sincronização, a interlevação imprevisível de instruções leva a **Condições de Corrida** e **Inconsistência de Dados**.

---

## 💡 Estratégia de Solução & Arquitetura

Para superar a fragilidade das soluções clássicas (que frequentemente causam *Starvation* nos escritores), implementamos um algoritmo justo baseado em **Fichas FIFO (Ticketing System)** utilizando a biblioteca `pthreads`:

1. **1 Mutex Principal (`pthread_mutex_t`):** Garante acesso atômico à estrutura de estado e distribuição de fichas.
2. **1 Variável de Condição (`pthread_cond_t`):** Faz com que as *threads* aguardem suspensas via `pthread_cond_wait`, eliminando o consumo indevido de CPU (*Busy Waiting* / *Spinlock*).
3. **Fila de Fichas FIFO (`prox_ficha` / `vez`):** Atribui uma senha de atendimento sequencial por ordem de chegada. Nenhuma *thread* é ultrapassada, eliminando permanentemente a **Inanição (*Starvation*)** e garantindo **Progresso**.

---

## 📸 Demonstração do Sistema

### 1. Execução Sincronizada (Modo Normal)
> *O Mutex assegura a integridade das variáveis `a` e `b`. Os leitores consultam de forma simultânea e o escritor possui acesso exclusivo.*

<!-- Cole o caminho ou link da imagem do seu log normal abaixo -->
p align="center">
  <img src="./assets/log_normal.png" alt="Log da Execução Normal" width="800"/>
</p>

### 2. O Teste do Caos (Sincronização Desativada)
> *Ao compilar sem as primitivas de Mutex (`-DSEM_SINCRONIZACAO`), ocorrem trocas involuntárias de contexto durante as pausas, forçando anomalias de memória `[X] DADO INCONSISTENTE`.*

<!-- Cole o caminho ou link da imagem do teste do caos abaixo -->
<p align="center">
  <img src="./assets/teste_caos.png" alt="Log do Teste do Caos" width="800"/>
</p>

### 3. Monitoramento de Threads no `htop`
> *Observação das threads em repouso (`Sleeping`) com `0.0%` de uso de CPU, confirmando a ausência de espera ocupada.*

<!-- Cole o caminho da sua captura do htop abaixo -->
<p align="center">
  <img src="./assets/htop_threads.png" alt="Visualização das Threads no htop" width="800"/>
</p>

---

## ⚙️ Como Compilar e Executar

Certifique-se de estar em um ambiente baseado em Linux / POSIX com o compilador `gcc` e a biblioteca `pthreads` instalados.

### 🟢 1. Compilação e Execução Normal (Com Sincronização)
```bash
# Compilar o código fonte
gcc -Wall -pthread codigo.c -o le

# Executar o programa
./le