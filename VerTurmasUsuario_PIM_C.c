#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <windows.h>
#include <conio.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
#endif

#define MAX_LINHA 512
#define ARQ_USUARIOS "usuarios.csv"
#define ARQ_TURMAS "turmas.csv"

// Remove BOM UTF‑8 no início da linha, se existir
void removerBOM(char *linha) {
    if ((unsigned char)linha[0] == 0xEF &&
        (unsigned char)linha[1] == 0xBB &&
        (unsigned char)linha[2] == 0xBF) {
        memmove(linha, linha + 3, strlen(linha + 3) + 1);
    }
}

// Leitura de senha com eco de asteriscos (Windows) ou invisível (Linux)
void getPassword(char *buf, size_t buflen, const char *prompt) {
    if (!buf || buflen == 0) return;

    printf("%s", prompt);
    fflush(stdout);

#if defined(_WIN32) || defined(_WIN64)
    size_t idx = 0;
    int ch;
    while ((ch = _getch()) != '\r' && ch != '\n' && idx + 1 < buflen) {
        if (ch == '\b') {
            if (idx > 0) {
                idx--;
                printf("\b \b");
            }
        } else {
            buf[idx++] = (char)ch;
            printf("*");
        }
    }
    buf[idx] = '\0';
    printf("\n");
#else
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    if (fgets(buf, (int)buflen, stdin) == NULL) {
        buf[0] = '\0';
    } else {
        buf[strcspn(buf, "\n")] = 0;
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\n");
#endif
}

// Verifica login e retorna 1 se válido
int verificarLogin(const char *email, const char *senha, int *idRetornado, char *nomeRetornado, char *nivelRetornado) {
    FILE *f = fopen(ARQ_USUARIOS, "r");
    if (!f) {
        printf("Erro ao abrir %s\n", ARQ_USUARIOS);
        return 0;
    }

    char linha[MAX_LINHA];
    while (fgets(linha, sizeof(linha), f)) {
        removerBOM(linha);
        linha[strcspn(linha, "\n")] = 0;

        char copia[MAX_LINHA];
        strcpy(copia, linha);

        char *campos[20];
        int i = 0;
        char *tk = strtok(copia, ";");
        while (tk && i < 20) {
            campos[i++] = tk;
            tk = strtok(NULL, ";");
        }
        if (i < 6) continue;

        int id = atoi(campos[0]);
        char *nome = campos[1];
        char *em = campos[2];
        char *sen = campos[3];
        char *nivel = campos[5];

        if (strcmp(em, email) == 0 && strcmp(sen, senha) == 0) {
            *idRetornado = id;
            strcpy(nomeRetornado, nome);
            strcpy(nivelRetornado, nivel);
            fclose(f);
            return 1;
        }
    }

    fclose(f);
    return 0;
}

// Mostra as turmas em que o usuário está inscrito
void mostrarTurmasUsuario(int idUsuario) {
    FILE *f = fopen(ARQ_TURMAS, "r");
    if (!f) {
        printf("Arquivo de turmas (%s) não encontrado.\n", ARQ_TURMAS);
        return;
    }

    char linha[MAX_LINHA];
    int encontrou = 0;

    printf("\n=== Turmas do usuário ===\n");
    printf("ID_Turma | Nome | Código | Seu Papel\n");
    printf("--------------------------------------------\n");
    while (fgets(linha, sizeof(linha), f)) {
        removerBOM(linha);
        linha[strcspn(linha, "\n")] = 0;

        char copia[MAX_LINHA];
        strcpy(copia, linha);

        char *campos[10];
        int cnt = 0;
        char *tk = strtok(copia, ";");
        while (tk && cnt < 10) {
            campos[cnt++] = tk;
            tk = strtok(NULL, ";");
        }
        if (cnt < 5) continue;

        int idTurma = atoi(campos[0]);
        char *nomeTurma = campos[1];
        char *codigo = campos[2];
        int idProf = atoi(campos[3]);
        char *listaAlunos = campos[4];

        if (idProf == idUsuario) {
            printf("%d | %s | %s | Professor\n", idTurma, nomeTurma, codigo);
            encontrou = 1;
        } else {
            char copiaAlunos[256];
            strcpy(copiaAlunos, listaAlunos);
            char *tokAluno = strtok(copiaAlunos, ",");
            while (tokAluno) {
                int idAl = atoi(tokAluno);
                if (idAl == idUsuario) {
                    printf("%d | %s | %s | Aluno\n", idTurma, nomeTurma, codigo);
                    encontrou = 1;
                    break;
                }
                tokAluno = strtok(NULL, ",");
            }
        }
    }

    if (!encontrou) {
        printf("Você não está em nenhuma turma.\n");
    }

    fclose(f);
}

int main() {
    // Configura console e locale para UTF-8 (essencial no Windows)
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    setlocale(LC_ALL, "pt_BR.UTF-8");

    char email[100], senha[100];
    int idUsuario;
    char nome[100], nivel[50];

    printf("=== LOGIN ===\n");
    printf("Email: ");
    fgets(email, sizeof(email), stdin);
    email[strcspn(email, "\n")] = 0;

    getPassword(senha, sizeof(senha), "Senha: ");

    if (!verificarLogin(email, senha, &idUsuario, nome, nivel)) {
        printf("Login inválido.\n");
        return 1;
    }

    // Saída sem mostrar ID e nível de acesso
    printf("Bem-vindo, %s!\n", nome);

    mostrarTurmasUsuario(idUsuario);

    return 0;
}
