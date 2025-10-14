#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <conio.h>  // Para _getch no Windows

#define MAX_LINHA 256
#define NOME_ARQUIVO "usuarios.csv"

// Função para ocultar a senha ao digitar
void lerSenha(char *senha, int max) {
    int i = 0;
    char ch;
    while ((ch = _getch()) != '\r' && i < max - 1) {
        if (ch == '\b') { // backspace
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

void removerBOM(char *linha) {
    if ((unsigned char)linha[0] == 0xEF &&
        (unsigned char)linha[1] == 0xBB &&
        (unsigned char)linha[2] == 0xBF) {
        memmove(linha, linha + 3, strlen(linha + 3) + 1);
    }
}

int obterUltimoID(FILE *arquivo) {
    int id = 0;
    char linha[MAX_LINHA];
    rewind(arquivo);

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        removerBOM(linha);
        char *token = strtok(linha, ";");
        if (token != NULL) {
            id = atoi(token);
        }
    }
    return id;
}

int verificarLogin(FILE *arquivo, const char *emailInput, const char *senhaInput,
                   char *nivelAcessoRetornado, char *nomeRetornado) {
    char linha[MAX_LINHA];
    rewind(arquivo);

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
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

        if (strcmp(email, emailInput) == 0 && strcmp(senha, senhaInput) == 0 && strcmp(atividade, "Ativo") == 0) {
            strcpy(nivelAcessoRetornado, nivelAcesso);
            strcpy(nomeRetornado, nome);
            return 1;
        }
    }
    return 0;
}

void imprimirUsuarios(FILE *arquivo) {
    char linha[MAX_LINHA];
    rewind(arquivo);

    printf("\n--- Lista de Usuários ---\n");
    printf("ID | Nome | Email | Senha | Idade | Nível | Curso | NP1 | NP2 | PIM | Média | Atividade\n");
    printf("--------------------------------------------------------------------------------------------\n");

    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        removerBOM(linha);
        printf("%s", linha);
    }
    printf("\n");
}

void alterarDadosUsuario(const char *nomeArquivo) {
    FILE *arquivo = fopen(nomeArquivo, "r");
    if (!arquivo) {
        printf("Erro ao abrir o arquivo para alteração.\n");
        return;
    }

    char linhas[100][MAX_LINHA];
    int totalLinhas = 0;

    while (fgets(linhas[totalLinhas], sizeof(linhas[totalLinhas]), arquivo) != NULL) {
        linhas[totalLinhas][strcspn(linhas[totalLinhas], "\n")] = '\0';
        removerBOM(linhas[totalLinhas]);
        totalLinhas++;
    }
    fclose(arquivo);

    int numUsuario, encontrado = 0;
    printf("Digite o número do usuário que deseja alterar: ");
    scanf("%d", &numUsuario);
    getchar();

    for (int i = 0; i < totalLinhas; i++) {
        int id;
        char tmpLinha[MAX_LINHA];
        strcpy(tmpLinha, linhas[i]);

        char *token = strtok(tmpLinha, ";");
        if (token) id = atoi(token);
        else continue;

        if (id == numUsuario) {
            encontrado = 1;
            char *campos[12];
            char linhaCpy[MAX_LINHA];
            strcpy(linhaCpy, linhas[i]);
            int j = 0;

            char *tk = strtok(linhaCpy, ";");
            while (tk && j < 12) {
                campos[j++] = tk;
                tk = strtok(NULL, ";");
            }

            if (j < 12) {
                printf("Formato de dados inválido.\n");
                return;
            }

            printf("Dados atuais do usuário %d:\n", numUsuario);
            for (int c = 1; c <= 10; c++) {
                printf("%d - %s\n", c, campos[c]);
            }
            printf("11 - Atividade: %s\n", campos[11]);

            int campoAlterar;
            printf("Qual campo deseja alterar? (1-11): ");
            scanf("%d", &campoAlterar);
            getchar();

            if (campoAlterar < 1 || campoAlterar > 11) {
                printf("Campo inválido.\n");
                return;
            }

            char novoValor[50];
            printf("Digite o novo valor: ");
            fgets(novoValor, sizeof(novoValor), stdin);
            novoValor[strcspn(novoValor, "\n")] = '\0';

            strcpy(campos[campoAlterar], novoValor);

            if (campoAlterar >= 7 && campoAlterar <= 9) {
                float np1 = atof(campos[7]);
                float np2 = atof(campos[8]);
                float pim = atof(campos[9]);
                float media = (np1 + np2 + pim) / 3;
                static char mediaStr[20];
                snprintf(mediaStr, sizeof(mediaStr), "%.2f", media);
                campos[10] = mediaStr;
            }

            snprintf(linhas[i], MAX_LINHA, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s",
                     campos[0], campos[1], campos[2], campos[3], campos[4],
                     campos[5], campos[6], campos[7], campos[8], campos[9],
                     campos[10], campos[11]);
            printf("Usuário atualizado com sucesso!\n");
            break;
        }
    }

    if (!encontrado) {
        printf("Usuário com ID %d não encontrado.\n", numUsuario);
        return;
    }

    arquivo = fopen(nomeArquivo, "w");
    if (!arquivo) {
        printf("Erro ao salvar alterações.\n");
        return;
    }

    unsigned char bom[] = {0xEF, 0xBB, 0xBF};
    fwrite(bom, sizeof(bom), 1, arquivo);

    for (int i = 0; i < totalLinhas; i++) {
        fprintf(arquivo, "%s\n", linhas[i]);
    }

    fclose(arquivo);
}

int main() {
    setlocale(LC_ALL, "Portuguese_Brazil");

    FILE *arquivo;
    char nome[50], email[50], senha[50], nivelAcesso[20], cursoOuMateria[50], atividade[10];
    int idade, idUsuario = 0;

    arquivo = fopen(NOME_ARQUIVO, "r");
    if (arquivo == NULL) {
        printf("Arquivo '%s' não encontrado. Criando primeiro usuário (Administrador).\n", NOME_ARQUIVO);
        arquivo = fopen(NOME_ARQUIVO, "w+b");
        if (arquivo == NULL) {
            printf("Erro ao criar o arquivo!\n");
            return 1;
        }

        unsigned char bom[] = {0xEF, 0xBB, 0xBF};
        fwrite(bom, sizeof(bom), 1, arquivo);

        printf("Digite o nome: ");
        fgets(nome, sizeof(nome), stdin);
        nome[strcspn(nome, "\n")] = '\0';

        printf("Digite o email: ");
        fgets(email, sizeof(email), stdin);
        email[strcspn(email, "\n")] = '\0';

        printf("Digite a senha: ");
        lerSenha(senha, sizeof(senha));

        printf("Digite a idade: ");
        scanf("%d", &idade);
        getchar();

        strcpy(nivelAcesso, "Administrador");
        strcpy(cursoOuMateria, "-");
        strcpy(atividade, "Ativo");

        fprintf(arquivo, "1;%s;%s;%s;%d;%s;%s;0;0;0;0;%s\n", nome, email, senha, idade, nivelAcesso, cursoOuMateria, atividade);
        fclose(arquivo);

        printf("Administrador criado com sucesso!\n");
        return 0;
    }
    fclose(arquivo);

    arquivo = fopen(NOME_ARQUIVO, "a+");
    if (!arquivo) {
        printf("Erro ao abrir o arquivo!\n");
        return 1;
    }

    char emailLogin[50], senhaLogin[50], nivelAcessoUsuario[20], nomeUsuario[50];
    printf("=== Login ===\n");
    printf("Email: ");
    fgets(emailLogin, sizeof(emailLogin), stdin);
    emailLogin[strcspn(emailLogin, "\n")] = '\0';

    printf("Senha: ");
    lerSenha(senhaLogin, sizeof(senhaLogin));

    if (!verificarLogin(arquivo, emailLogin, senhaLogin, nivelAcessoUsuario, nomeUsuario)) {
        printf("Login inválido ou usuário inativo! Encerrando.\n");
        fclose(arquivo);
        return 1;
    }

    if (strcmp(nivelAcessoUsuario, "Administrador") != 0) {
        printf("Acesso negado. Apenas Administradores podem usar o sistema.\n");
        fclose(arquivo);
        return 1;
    }

    printf("Bem-vindo(a), %s (%s)!\n", nomeUsuario, nivelAcessoUsuario);

    rewind(arquivo);
    idUsuario = obterUltimoID(arquivo);
    fseek(arquivo, 0, SEEK_END);

    char opcao;
    do {
        printf("\n=== Menu ===\n");
        printf("1 - Imprimir usuários\n");
        printf("2 - Alterar dados\n");
        printf("3 - Adicionar usuário\n");
        printf("4 - Sair\n");
        printf("Escolha: ");
        opcao = getchar();
        getchar();

        switch (opcao) {
            case '1':
                imprimirUsuarios(arquivo);
                break;

            case '2':
                fclose(arquivo);
                alterarDadosUsuario(NOME_ARQUIVO);
                arquivo = fopen(NOME_ARQUIVO, "a+");
                rewind(arquivo);
                idUsuario = obterUltimoID(arquivo);
                fseek(arquivo, 0, SEEK_END);
                break;

            case '3':
                idUsuario++;

                printf("Digite o nome: ");
                fgets(nome, sizeof(nome), stdin);
                nome[strcspn(nome, "\n")] = '\0';

                printf("Digite o email: ");
                fgets(email, sizeof(email), stdin);
                email[strcspn(email, "\n")] = '\0';

                printf("Digite a senha: ");
                lerSenha(senha, sizeof(senha));

                printf("Digite a idade: ");
                scanf("%d", &idade);
                getchar();

                printf("Digite o nível de acesso (Administrador, Professor, Estudante, Coordenador): ");
                fgets(nivelAcesso, sizeof(nivelAcesso), stdin);
                nivelAcesso[strcspn(nivelAcesso, "\n")] = '\0';

                int acessoValido = strcmp(nivelAcesso, "Administrador") == 0 ||
                                   strcmp(nivelAcesso, "Professor") == 0 ||
                                   strcmp(nivelAcesso, "Estudante") == 0 ||
                                   strcmp(nivelAcesso, "Coordenador") == 0;

                if (!acessoValido) {
                    printf("Nível de acesso inválido!\n");
                    break;
                }

                float np1 = 0, np2 = 0, pim = 0, media = 0;
                strcpy(cursoOuMateria, "-");

                if (strcmp(nivelAcesso, "Estudante") == 0) {
                    printf("Digite o curso: ");
                    fgets(cursoOuMateria, sizeof(cursoOuMateria), stdin);
                    cursoOuMateria[strcspn(cursoOuMateria, "\n")] = '\0';

                    printf("Digite a nota NP1: ");
                    scanf("%f", &np1); getchar();
                    printf("Digite a nota NP2: ");
                    scanf("%f", &np2); getchar();
                    printf("Digite a nota PIM: ");
                    scanf("%f", &pim); getchar();

                    media = (np1 + np2 + pim) / 3;
                } else if (strcmp(nivelAcesso, "Professor") == 0) {
                    printf("Digite a matéria: ");
                    fgets(cursoOuMateria, sizeof(cursoOuMateria), stdin);
                    cursoOuMateria[strcspn(cursoOuMateria, "\n")] = '\0';
                }

                strcpy(atividade, "Ativo");

                fprintf(arquivo, "%d;%s;%s;%s;%d;%s;%s;%.2f;%.2f;%.2f;%.2f;%s\n",
                        idUsuario, nome, email, senha, idade, nivelAcesso,
                        cursoOuMateria, np1, np2, pim, media, atividade);
                fflush(arquivo);

                printf("Usuário adicionado com sucesso!\n");
                break;

            case '4':
                printf("Saindo...\n");
                break;

            default:
                printf("Opção inválida!\n");
        }
    } while (opcao != '4');

    fclose(arquivo);
    return 0;
}
