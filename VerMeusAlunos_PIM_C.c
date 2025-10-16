#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
#endif

#define MAX_LINHA 512
#define ARQ_USUARIOS "usuarios.csv"
#define ARQ_TURMAS "turmas.csv"

void removerBOM(char *linha) {
    if ((unsigned char)linha[0] == 0xEF &&
        (unsigned char)linha[1] == 0xBB &&
        (unsigned char)linha[2] == 0xBF) {
        memmove(linha, linha + 3, strlen(linha + 3) + 1);
    }
}

/*
 * getPassword com eco de '*' (suporta backspace).
 * buflen é o tamanho total do buffer (incluindo terminador).
 */
void getPassword(char *buf, size_t buflen, const char *prompt) {
    if (!buf || buflen == 0) return;

    printf("%s", prompt);
    fflush(stdout);

#if defined(_WIN32) || defined(_WIN64)
    size_t idx = 0;
    int ch;
    while ((ch = _getch()) != '\r' && ch != '\n' && idx + 1 < buflen) {
        if (ch == 8) { // backspace (Windows: 8)
            if (idx > 0) {
                idx--;
                buf[idx] = '\0';
                // Apaga o caractere '*' no terminal
                printf("\b \b");
                fflush(stdout);
            }
        } else if (ch == 3) { // Ctrl-C
            // propagate
            exit(1);
        } else {
            buf[idx++] = (char)ch;
            printf("*");
            fflush(stdout);
        }
    }
    buf[idx] = '\0';
    printf("\n");
#else
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    // desliga ECHO e também desliga ICANON para ler caractere a caractere
    newt.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    size_t idx = 0;
    char ch;
    while (idx + 1 < buflen) {
        ssize_t r = read(STDIN_FILENO, &ch, 1);
        if (r <= 0) break;

        if (ch == '\n' || ch == '\r') {
            break;
        } else if (ch == 127 || ch == 8) { // backspace (Unix DEL=127, alguns terminais 8)
            if (idx > 0) {
                idx--;
                buf[idx] = '\0';
                // Apaga o caractere '*' no terminal
                printf("\b \b");
                fflush(stdout);
            }
        } else if (ch == 3) { // Ctrl-C
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
            exit(1);
        } else {
            buf[idx++] = ch;
            printf("*");
            fflush(stdout);
        }
    }

    buf[idx] = '\0';
    // restaura terminal
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\n");
#endif
}

int verificarLoginProfessor(const char *email, const char *senha, int *idProfessor, char *nomeProfessor) {
    FILE *f = fopen(ARQ_USUARIOS, "r");
    if (!f) {
        printf("Erro ao abrir %s\n", ARQ_USUARIOS);
        return 0;
    }

    char linha[MAX_LINHA];
    while (fgets(linha, sizeof(linha), f)) {
        removerBOM(linha);
        linha[strcspn(linha, "\n")] = 0;

        char *campos[12];
        int i = 0;
        char *token = strtok(linha, ";");
        while (token && i < 12) {
            campos[i++] = token;
            token = strtok(NULL, ";");
        }

        if (i >= 6 &&
            strcmp(campos[2], email) == 0 &&
            strcmp(campos[3], senha) == 0 &&
            strcmp(campos[5], "Professor") == 0) {
            *idProfessor = atoi(campos[0]);
            strcpy(nomeProfessor, campos[1]);
            fclose(f);
            return 1;
        }
    }

    fclose(f);
    return 0;
}

void exibirInfoAluno(int idAluno) {
    FILE *f = fopen(ARQ_USUARIOS, "r");
    if (!f) return;

    char linha[MAX_LINHA];
    while (fgets(linha, sizeof(linha), f)) {
        removerBOM(linha);
        linha[strcspn(linha, "\n")] = 0;

        char *campos[12];
        int i = 0;
        char *token = strtok(linha, ";");
        while (token && i < 12) {
            campos[i++] = token;
            token = strtok(NULL, ";");
        }

        if (i >= 6 && atoi(campos[0]) == idAluno && strcmp(campos[5], "Estudante") == 0) {
            printf("ID: %s | Nome: %s | Curso: %s | NP1: %s | NP2: %s | PIM: %s | Média: %s\n",
                   campos[0], campos[1], campos[6], campos[7], campos[8], campos[9], campos[10]);
            break;
        }
    }

    fclose(f);
}

void listarAlunosTurmasProfessor(int idProfessor) {
    FILE *f = fopen(ARQ_TURMAS, "r");
    if (!f) {
        printf("Erro ao abrir %s\n", ARQ_TURMAS);
        return;
    }

    char linha[MAX_LINHA];
    int encontrou = 0;

    while (fgets(linha, sizeof(linha), f)) {
        removerBOM(linha);
        linha[strcspn(linha, "\n")] = 0;

        char copia[MAX_LINHA];
        strcpy(copia, linha);

        char *campos[5];
        int i = 0;
        char *token = strtok(copia, ";");
        while (token && i < 5) {
            campos[i++] = token;
            token = strtok(NULL, ";");
        }

        if (i < 5) continue;

        int idProfTurma = atoi(campos[3]);
        if (idProfTurma == idProfessor) {
            encontrou = 1;
            printf("\n--- Turma: %s (%s) ---\n", campos[1], campos[2]);

            // precisamos usar uma cópia dos IDs para não perder a original
            char ids[MAX_LINHA];
            strncpy(ids, campos[4], sizeof(ids)-1);
            ids[sizeof(ids)-1] = '\0';

            char *idToken = strtok(ids, ",");
            while (idToken) {
                int idAluno = atoi(idToken);
                exibirInfoAluno(idAluno);
                idToken = strtok(NULL, ",");
            }
        }
    }

    fclose(f);

    if (!encontrou) {
        printf("Você não possui nenhuma turma cadastrada.\n");
    }
}

void listarAlunosDeUmaTurma(int idProfessor) {
    char nomeTurma[100];
    printf("Digite o nome da turma: ");
    fgets(nomeTurma, sizeof(nomeTurma), stdin);
    nomeTurma[strcspn(nomeTurma, "\n")] = 0;

    FILE *f = fopen(ARQ_TURMAS, "r");
    if (!f) {
        printf("Erro ao abrir %s\n", ARQ_TURMAS);
        return;
    }

    char linha[MAX_LINHA];
    int encontrou = 0;

    while (fgets(linha, sizeof(linha), f)) {
        removerBOM(linha);
        linha[strcspn(linha, "\n")] = 0;

        char copia[MAX_LINHA];
        strcpy(copia, linha);

        char *campos[5];
        int i = 0;
        char *token = strtok(copia, ";");
        while (token && i < 5) {
            campos[i++] = token;
            token = strtok(NULL, ";");
        }

        if (i < 5) continue;

        int idProfTurma = atoi(campos[3]);
        if (idProfTurma == idProfessor && strcmp(campos[1], nomeTurma) == 0) {
            encontrou = 1;
            printf("\n--- Alunos da Turma: %s (%s) ---\n", campos[1], campos[2]);

            char ids[MAX_LINHA];
            strncpy(ids, campos[4], sizeof(ids)-1);
            ids[sizeof(ids)-1] = '\0';

            char *idToken = strtok(ids, ",");
            while (idToken) {
                int idAluno = atoi(idToken);
                exibirInfoAluno(idAluno);
                idToken = strtok(NULL, ",");
            }
            break;
        }
    }

    fclose(f);

    if (!encontrou) {
        printf("Turma não encontrada ou você não é o responsável por ela.\n");
    }
}

int main() {
    setlocale(LC_ALL, "Portuguese");

    char email[50], senha[100], nome[100];
    int idProfessor;

    printf("=== Login do Professor ===\n");
    printf("Email: ");
    fgets(email, sizeof(email), stdin);
    email[strcspn(email, "\n")] = 0;

    getPassword(senha, sizeof(senha), "Senha: ");

    if (!verificarLoginProfessor(email, senha, &idProfessor, nome)) {
        printf("Acesso negado! Apenas professores podem acessar este sistema.\n");
        return 1;
    }

    printf("Bem-vindo, %s!\n", nome);

    char opcao;
    do {
        printf("\n--- MENU ---\n");
        printf("1 - Ver todos os alunos das minhas turmas\n");
        printf("2 - Ver alunos de uma turma específica\n");
        printf("3 - Sair\n");
        printf("Escolha: ");
        scanf(" %c", &opcao);
        getchar(); // limpar buffer

        switch (opcao) {
            case '1':
                listarAlunosTurmasProfessor(idProfessor);
                break;
            case '2':
                listarAlunosDeUmaTurma(idProfessor);
                break;
            case '3':
                printf("Saindo...\n");
                break;
            default:
                printf("Opção inválida.\n");
        }

    } while (opcao != '3');

    return 0;
}
