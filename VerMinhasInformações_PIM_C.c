#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <conio.h>

#define MAX_LINHA 256
#define NOME_ARQUIVO "usuarios.csv"

// Função para ocultar senha com asteriscos
void lerSenha(char *senha, int max) {
    int i = 0;
    char ch;
    while ((ch = _getch()) != '\r' && i < max - 1) {
        if (ch == '\b') {
            if (i > 0) {
                i--;
                printf("\b \b");
            }
        } else {
            senha[i++] = ch;
            printf("*");
        }
    }
    senha[i] = '\0';
    printf("\n");
}

// Remove BOM caso exista
void removerBOM(char *linha) {
    if ((unsigned char)linha[0] == 0xEF &&
        (unsigned char)linha[1] == 0xBB &&
        (unsigned char)linha[2] == 0xBF) {
        memmove(linha, linha + 3, strlen(linha + 3) + 1);
    }
}

int main() {
    setlocale(LC_ALL, "Portuguese_Brazil");

    FILE *arquivo = fopen(NOME_ARQUIVO, "r");
    if (!arquivo) {
        printf("Arquivo de usuários não encontrado.\n");
        return 1;
    }

    char emailInput[50], senhaInput[50];
    printf("=== Login do Usuário ===\n");
    printf("Email: ");
    fgets(emailInput, sizeof(emailInput), stdin);
    emailInput[strcspn(emailInput, "\n")] = '\0';

    printf("Senha: ");
    lerSenha(senhaInput, sizeof(senhaInput));

    char linha[MAX_LINHA];
    int encontrado = 0;

    while (fgets(linha, sizeof(linha), arquivo)) {
        removerBOM(linha);
        linha[strcspn(linha, "\n")] = '\0';

        int id, idade;
        char nome[50], email[50], senha[50], nivelAcesso[20], cursoOuMateria[50], atividade[10];
        float np1, np2, pim, media;

        char *token = strtok(linha, ";");
        if (!token) continue; id = atoi(token);
        token = strtok(NULL, ";"); if (!token) continue; strcpy(nome, token);
        token = strtok(NULL, ";"); if (!token) continue; strcpy(email, token);
        token = strtok(NULL, ";"); if (!token) continue; strcpy(senha, token);
        token = strtok(NULL, ";"); if (!token) continue; idade = atoi(token);
        token = strtok(NULL, ";"); if (!token) continue; strcpy(nivelAcesso, token);
        token = strtok(NULL, ";"); if (!token) continue; strcpy(cursoOuMateria, token);
        token = strtok(NULL, ";"); if (!token) continue; np1 = atof(token);
        token = strtok(NULL, ";"); if (!token) continue; np2 = atof(token);
        token = strtok(NULL, ";"); if (!token) continue; pim = atof(token);
        token = strtok(NULL, ";"); if (!token) continue; media = atof(token);
        token = strtok(NULL, ";"); if (!token) continue; strcpy(atividade, token);

        if (strcmp(email, emailInput) == 0 && strcmp(senha, senhaInput) == 0) {
            if (strcmp(atividade, "Ativo") != 0) {
                printf("Usuário inativo. Acesso negado.\n");
                fclose(arquivo);
                return 1;
            }

            printf("\n=== Informações do Usuário ===\n");
            printf("ID: %d\n", id);
            printf("Nome: %s\n", nome);
            printf("Email: %s\n", email);
            printf("Idade: %d\n", idade);
            printf("Nível de Acesso: %s\n", nivelAcesso);
            printf("Curso/Matéria: %s\n", cursoOuMateria);

            // Mostrar notas apenas para estudantes
            if (strcmp(nivelAcesso, "Estudante") == 0) {
                printf("Nota NP1: %.2f\n", np1);
                printf("Nota NP2: %.2f\n", np2);
                printf("Nota PIM: %.2f\n", pim);
                printf("Média: %.2f\n", media);
            }

            printf("Status: %s\n", atividade);
            encontrado = 1;
            break;
        }
    }

    fclose(arquivo);

    if (!encontrado) {
        printf("Login inválido. Verifique suas credenciais.\n");
        return 1;
    }

    return 0;
}
