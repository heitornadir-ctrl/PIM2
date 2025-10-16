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

void getPassword(char *buf, size_t buflen, const char *prompt) {
    if (!buf || buflen == 0) return;

    printf("%s", prompt);
    fflush(stdout);

#if defined(_WIN32) || defined(_WIN64)
    size_t idx = 0;
    int ch;
    while ((ch = _getch()) != '\r' && ch != '\n' && idx + 1 < buflen) {
        if (ch == 8) { // backspace
            if (idx > 0) {
                idx--;
                buf[idx] = '\0';
                printf("\b \b");
                fflush(stdout);
            }
        } else if (ch == 3) { // Ctrl-C
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
    if (tcgetattr(STDIN_FILENO, &oldt) != 0) {
        // fallback simples
        fgets(buf, buflen, stdin);
        buf[strcspn(buf, "\n")] = 0;
        return;
    }
    newt = oldt;
    newt.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    size_t idx = 0;
    char ch;
    while (idx + 1 < buflen) {
        ssize_t r = read(STDIN_FILENO, &ch, 1);
        if (r <= 0) break;

        if (ch == '\n' || ch == '\r') {
            break;
        } else if (ch == 127 || ch == 8) {
            if (idx > 0) {
                idx--;
                buf[idx] = '\0';
                printf("\b \b");
                fflush(stdout);
            }
        } else if (ch == 3) {
            tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
            exit(1);
        } else {
            buf[idx++] = ch;
            printf("*");
            fflush(stdout);
        }
    }

    buf[idx] = '\0';
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\n");
#endif
}

int verificarLogin(const char *email, const char *senha, int *idUser, char *nome, char *nivel) {
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
            strcmp(campos[3], senha) == 0) {
            *idUser = atoi(campos[0]);
            strncpy(nome, campos[1], 99); nome[99] = '\0';
            strncpy(nivel, campos[5], 19); nivel[19] = '\0';
            fclose(f);
            return 1;
        }
    }

    fclose(f);
    return 0;
}

void obterNomeAlunoPorID(int idAluno, char *nomeAluno) {
    nomeAluno[0] = '\0';
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

        if (atoi(campos[0]) == idAluno) {
            strncpy(nomeAluno, campos[1], 99);
            nomeAluno[99] = '\0';
            break;
        }
    }

    fclose(f);
}

void atualizarNotaAluno(int idAluno) {
    FILE *f = fopen(ARQ_USUARIOS, "r");
    if (!f) {
        printf("Erro ao abrir arquivo de usuários.\n");
        return;
    }

    char linhas[2000][MAX_LINHA];
    int total = 0;

    while (fgets(linhas[total], MAX_LINHA, f) && total < 2000) {
        removerBOM(linhas[total]);
        linhas[total][strcspn(linhas[total], "\n")] = 0;
        total++;
    }
    fclose(f);

    int encontrou = 0;
    for (int i = 0; i < total; i++) {
        char copia[MAX_LINHA];
        strcpy(copia, linhas[i]);

        char *campos[12];
        int j = 0;
        char *token = strtok(copia, ";");
        while (token && j < 12) {
            campos[j++] = token;
            token = strtok(NULL, ";");
        }

        if (j >= 6 && atoi(campos[0]) == idAluno && strcmp(campos[5], "Estudante") == 0) {
            float np1 = 0.0f, np2 = 0.0f, pim = 0.0f;
            char nomeAluno[128] = "Desconhecido";
            obterNomeAlunoPorID(idAluno, nomeAluno);

            printf("\nLançando notas para: %s (ID %d)\n", nomeAluno, idAluno);

            // Validação: 0 a 10
            do {
                printf("Digite NP1 (0–10): ");
                if (scanf("%f", &np1) != 1 || np1 < 0.0f || np1 > 10.0f) {
                    getchar();
                    printf("Valor inválido. Tente novamente.\n");
                } else break;
            } while (1);

            do {
                printf("Digite NP2 (0–10): ");
                if (scanf("%f", &np2) != 1 || np2 < 0.0f || np2 > 10.0f) {
                    getchar();
                    printf("Valor inválido. Tente novamente.\n");
                } else break;
            } while (1);

            do {
                printf("Digite PIM (0–10): ");
                if (scanf("%f", &pim) != 1 || pim < 0.0f || pim > 10.0f) {
                    getchar();
                    printf("Valor inválido. Tente novamente.\n");
                } else break;
            } while (1);

            getchar();  // limpar resto da linha

            float media = (np1 + np2 + pim) / 3.0f;

            char novaLinha[MAX_LINHA];
            const char *atividade = (j >= 12 ? campos[11] : "Ativo");
            snprintf(novaLinha, sizeof(novaLinha),
                     "%s;%s;%s;%s;%s;%s;%s;%.2f;%.2f;%.2f;%.2f;%s",
                     campos[0], campos[1],
                     (j > 2 ? campos[2] : ""),
                     (j > 3 ? campos[3] : ""),
                     (j > 4 ? campos[4] : ""),
                     campos[5],
                     (j > 6 ? campos[6] : ""),
                     np1, np2, pim, media,
                     atividade);

            strncpy(linhas[i], novaLinha, MAX_LINHA - 1);
            linhas[i][MAX_LINHA - 1] = '\0';

            encontrou = 1;
            break;
        }
    }

    if (!encontrou) {
        printf("Aluno não encontrado ou não é estudante.\n");
        return;
    }

    f = fopen(ARQ_USUARIOS, "w");
    if (!f) {
        printf("Erro ao gravar alterações.\n");
        return;
    }

    unsigned char bom[] = {0xEF, 0xBB, 0xBF};
    fwrite(bom, sizeof(bom), 1, f);

    for (int i = 0; i < total; i++) {
        fprintf(f, "%s\n", linhas[i]);
    }
    fclose(f);

    printf("Notas atualizadas com sucesso!\n");
}

void lancarNotasTurma(int idUsuario, const char *nivelAcesso) {
    char nomeTurma[128] = "";
    char linha[MAX_LINHA];

    if (strcmp(nivelAcesso, "Professor") == 0) {
        FILE *f = fopen(ARQ_TURMAS, "r");
        if (!f) {
            printf("Erro ao abrir %s\n", ARQ_TURMAS);
            return;
        }

        printf("\n--- Suas turmas ---\n");
        int count = 0;
        char turmas[100][128];

        while (fgets(linha, sizeof(linha), f) && count < 100) {
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

            if (i >= 4 && atoi(campos[3]) == idUsuario) {
                printf("%d - %s (%s)\n", count + 1,
                       campos[1], (i > 2 ? campos[2] : ""));
                strncpy(turmas[count], campos[1], sizeof(turmas[count]) - 1);
                turmas[count][sizeof(turmas[count]) - 1] = '\0';
                count++;
            }
        }
        fclose(f);

        if (count == 0) {
            printf("Nenhuma turma encontrada para este professor.\n");
            return;
        }

        int escolha = 0;
        printf("Escolha uma turma (número): ");
        if (scanf("%d", &escolha) != 1) { getchar(); printf("Entrada inválida.\n"); return; }
        getchar();
        if (escolha < 1 || escolha > count) {
            printf("Opção inválida.\n");
            return;
        }
        strncpy(nomeTurma, turmas[escolha - 1], sizeof(nomeTurma) - 1);
        nomeTurma[sizeof(nomeTurma) - 1] = '\0';
    } else {
        printf("Digite o nome da turma: ");
        if (!fgets(nomeTurma, sizeof(nomeTurma), stdin)) {
            return;
        }
        nomeTurma[strcspn(nomeTurma, "\n")] = 0;
    }

    FILE *f = fopen(ARQ_TURMAS, "r");
    if (!f) {
        printf("Erro ao abrir %s\n", ARQ_TURMAS);
        return;
    }

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

        if (i >= 5 && strcmp(campos[1], nomeTurma) == 0) {
            encontrou = 1;
            char ids[MAX_LINHA];
            strncpy(ids, campos[4], sizeof(ids) - 1);
            ids[sizeof(ids) - 1] = '\0';

            char *idToken = strtok(ids, ",");
            while (idToken) {
                int idAluno = atoi(idToken);
                char nomeAluno[128] = "";
                obterNomeAlunoPorID(idAluno, nomeAluno);
                printf("\nPróximo aluno: %s\n", (nomeAluno[0] ? nomeAluno : "Desconhecido"));
                atualizarNotaAluno(idAluno);
                idToken = strtok(NULL, ",");
            }
            break;
        }
    }

    fclose(f);

    if (!encontrou) {
        printf("Turma '%s' não encontrada.\n", nomeTurma);
    }
}

int main() {
    setlocale(LC_ALL, "Portuguese");

    char email[128], senha[128], nome[128], nivel[64];
    int idUsuario = 0;

    printf("=== Login ===\n");
    printf("Email: ");
    if (!fgets(email, sizeof(email), stdin)) {
        return 1;
    }
    email[strcspn(email, "\n")] = 0;

    getPassword(senha, sizeof(senha), "Senha: ");

    if (!verificarLogin(email, senha, &idUsuario, nome, nivel)) {
        printf("Login inválido ou usuário não encontrado.\n");
        return 1;
    }

    printf("Bem-vindo(a), %s (%s)\n", nome, nivel);

    char opcao;
    do {
        printf("\n--- MENU ---\n");
        printf("1 - Lançar nota para aluno por ID\n");
        printf("2 - Lançar notas para todos de uma turma\n");
        printf("3 - Sair\n");
        printf("Escolha: ");
        if (scanf(" %c", &opcao) != 1) {
            getchar();
            continue;
        }
        getchar();

        switch (opcao) {
            case '1': {
                int idAluno;
                printf("Digite o ID do aluno: ");
                if (scanf("%d", &idAluno) != 1) {
                    getchar();
                    printf("Entrada inválida.\n");
                } else {
                    getchar();
                    atualizarNotaAluno(idAluno);
                }
                break;
            }
            case '2':
                lancarNotasTurma(idUsuario, nivel);
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
