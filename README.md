# 🧵 Leitores e Escritores com Mutexes (POSIX Threads)

<p align="center">
  <img src="https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white" alt="C" />
  <img src="https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black" alt="Linux" />
  <img src="https://img.shields.io/badge/POSIX_pthreads-000000?style=for-the-badge&logo=gnu&logoColor=white" alt="POSIX" />
  <img src="https://img.shields.io/badge/Status-Concluído-success?style=for-the-badge" alt="Status" />
</p>

> [!NOTE]
> ### ✅ PROJETO FINALIZADO
> **Atualização:** 08/10/2026 às 09:43  
> Finalizamos o desenvolvimento e os testes do código. Chegamos à conclusão de que o nosso projeto está muito bom, cobrindo todos os requisitos de sincronização, e o repositório já está pronto para avaliação!

> **Projeto Prático de Concorrência e Sincronização**  
> Disciplina: Sistemas Operacionais (SO)  
> Universidade do Estado do Rio Grande do Norte (UERN) — Campus Natal  
> Professoras: Me. Gláucia Melissa Medeiros Campos e Artemísia Kimberlly Silva  

---

## 🎥 Vídeo de Apresentação

> ⚠️ **Aviso:** Pode ser que o áudio do vídeo esteja um pouquinho baixo ou com um leve ruído ambiente, mas dá para acompanhar perfeitamente toda a explicação da lógica, do código e a demonstração rodando no terminal!

<p align="center">
  <a href="https://youtu.be/ZQSDpOZDJJ0" target="_blank">
    <img src="https://img.youtube.com/vi/ZQSDpOZDJJ0/maxresdefault.jpg" alt="Assistir Apresentação no YouTube" width="800" />
  </a>
</p>
<p align="center">
  <strong>🔗 <a href="https://youtu.be/ZQSDpOZDJJ0" target="_blank">Clique aqui ou na imagem acima para assistir à nossa apresentação no YouTube!</a></strong>
</p>

---

## 👨‍💻 Trio 03

| Foto / Avatar | Integrante | GitHub |
| :---: | :--- | :--- |
| <img src="https://github.com/github.png" width="50"> | **Antoniel da Silva Alves** | [@NielS2Dev](https://github.com/NielS2Dev) |
| <img src="https://github.com/github.png" width="50"> | **Geovane Guilherme do Nascimento** | [@geovane2606](https://github.com/geovane2606) |
| <img src="https://github.com/github.png" width="50"> | **Sayan Bruno da Silva Soares** | [@sayanbruno](https://github.com/sayanbruno) |

---

## 📌 Visão Geral do Problema

O problema dos **Leitores e Escritores** é um clássico de concorrência. Nele, várias *threads* tentam acessar o mesmo dado na memória RAM ao mesmo tempo:
- **Leitores ($R$):** Só leem o dado. Vários leitores podem ler juntos sem problema.
- **Escritores ($W$):** Eles alteram o dado. Por isso, exigem **acesso exclusivo** (não pode ter outro escritor nem leitor lá dentro enquanto ele escreve).

Se a gente não sincronizar isso direito, as *threads* se atropelam. Isso gera a famosa **Condição de Corrida** e os dados ficam inconsistentes.

---

##