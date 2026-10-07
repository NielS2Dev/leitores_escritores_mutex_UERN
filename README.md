# 🧵 Leitores e Escritores com Mutexes (POSIX Threads)

<p align="center">
  <img src="https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white" alt="C" />
  <img src="https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black" alt="Linux" />
  <img src="https://img.shields.io/badge/POSIX_pthreads-000000?style=for-the-badge&logo=gnu&logoColor=white" alt="POSIX" />
  <img src="https://img.shields.io/badge/Status-Em%20Desenvolvimento-yellow?style=for-the-badge" alt="Status" />
</p>

> [!WARNING]
> ### ⚠️ TRABALHO EM ANDAMENTO
> **Aviso:** Esse repositório ainda está em construção!
> - Nós ainda estamos desenvolvendo e testando o código principal em C (`codigo.c`).
> - O texto do `README.md` também está incompleto e vamos adicionar mais coisas.
> - Estamos arrumando os últimos detalhes para fechar o projeto e entregar até esta quinta-feira.

> **Projeto Prático de Concorrência e Sincronização**  
> Disciplina: Sistemas Operacionais (SO)  
> Universidade do Estado do Rio Grande do Norte (UERN) — Campus Natal  
> Professoras: Me. Gláucia Melissa Medeiros Campos e Artemísia Kimberlly Silva  

---

## 👨‍💻 Trio 04

| Foto / Avatar | Integrante | GitHub |
| :---: | :--- | :--- |
| <img src="https://github.com/github.png" width="50"> | **Antoniel da Silva Alves** | [@NielS2Dev](https://github.com/NielS2Dev) |
| <img src="https://github.com/github.png" width="50"> | **Geovane Guilherme do Nascimento** | [@geovane2606](https://github.com/geovane2606) |
| <img src="https://github.com/github.png" width="50"> | **Sayan Bruno da Silva Soares** | [@sayanbruno](https://github.com/sayanbruno) |

---

## 📌 Visão Geral do Problema

O problema dos **Leitores e Escritores** é um clássico de concorrência. Nele, várias *threads* tentam acessar o mesmo dado na memória RAM ao mesmo tempo:
* **Leitores ($R$):** Só leem o dado. Vários leitores podem ler juntos sem problema.
* **Escritores ($W$):** Eles alteram o dado. Por isso, exigem **acesso exclusivo** (não pode ter outro escritor nem leitor lá dentro enquanto ele escreve).

Se a gente não sincronizar isso direito, as *threads* se atropelam. Isso gera a famosa **Condição de Corrida** e os dados ficam inconsistentes.

---

## 💡 Nossa Estratégia

A solução clássica tem um problema chato: se novos leitores não pararem de chegar, o escritor nunca consegue entrar e fica travado para sempre (o famoso *Starvation* / Inanição).

Para resolver isso, montamos uma lógica baseada em **Fichas FIFO** (como uma fila de banco) usando a biblioteca `pthreads` em C:

1. **1 Mutex Principal (`pthread_mutex_t`):** Funciona como a nossa trava. Ele protege as variáveis e controla quem pode pegar a ficha.
2. **1 Variável de Condição (`pthread_cond_t`):** Serve para fazer as *threads* dormirem (`pthread_cond_wait`) enquanto esperam a vez delas. Isso evita que o programa fique gastando CPU à toa em um loop infinito (*sem busy waiting*).
3. **Fila de Fichas FIFO:** Cada *thread* pega uma ficha por ordem de chegada. Como ninguém fura a fila, garantimos que todo mundo vai ser atendido e o escritor não sofre *Starvation*.

---

## 📸 Demonstração do Sistema

### 1. Execução Normal (Com Mutex)
> *Aqui a trava funciona perfeitamente. Os leitores conseguem ler juntos e o escritor tem acesso exclusivo na vez dele.*

<p align="center">
  <img src="https://i.ibb.co/xtYB5QHR/foto-001.png" alt="Log da Execução Normal" width="800"/>
</p>

---

### 2. O Teste do Caos (Sem Mutex)
> *Se a gente compilar desativando a nossa trava (`-DSEM_SINCRONIZACAO`), vira bagunça. O leitor entra na hora errada e o terminal começa a gritar `[X] DADO INCONSISTENTE`.*

<p align="center">
  <img src="https://i.ibb.co/N6pfhbq4/novo.png" alt="Log do Teste do Caos" width="800"/>
</p>

---

### 3. Monitoramento no `htop`
> *Olhando as threads do programa no htop, a gente prova que elas ficam em repouso (`Sleeping`) com `0.0%` de CPU enquanto esperam. Ou seja, zero espera ocupada.*

<p align="center">
  <img src="https://i.ibb.co/M5VfmLG4/foto-002.png" alt="Visualização das Threads no htop" width="800"/>
</p>

---

## ⚙️ Como Compilar e Executar no Linux

Se você for testar o código, é só usar os comandos abaixo no terminal:

### 🟢 1. Modo Normal (Tudo funcionando)
```bash
# Compilar o código
gcc -Wall -pthread codigo.c -o le

# Rodar o programa
./le