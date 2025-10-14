#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h> // Para suporte a acentuação

#define MAX_LINHA 512
#define ARQ_USUARIOS "usuarios.csv"
#define ARQ_TURMAS "turmas.csv"

// Remove BOM UTF-8
void removerBOM(char *linha) {
    if ((unsigned char)linha[0] == 0xEF &&
        (unsigned char)linha[1] == 0xBB &&
        (unsigned char)linha[2] == 0xBF) {
        memmove(linha, linha + 3, strlen(linha + 3) + 1);
    }
}

// Verifica login (somente Admin e Coordenador)
int verificarLogin(const char *email, const char *senha, char *nome, char *nivel) {
    FILE *f = fopen(ARQ_USUARIOS, "r");
    if (!f) {
        printf("Erro ao abrir %s\n", ARQ_USUARIOS);
        return 0;
    }

    char linha[MAX_LINHA];
    while (fgets(linha, sizeof(linha), f)) {
        removerBOM(linha);
        linha[strcspn(linha, "\n")] = 0;

        char *token = strtok(linha, ";");
        if (!token) continue;

        char *campos[7];
        campos[0] = token;
        for (int i = 1; i < 7; i++) {
            campos[i] = strtok(NULL, ";");
            if (!campos[i]) break;
        }

        if (strcmp(campos[2], email) == 0 && strcmp(campos[3], senha) == 0 &&
            (strcmp(campos[5], "Administrador") == 0 || strcmp(campos[5], "Coordenador") == 0)) {
            strcpy(nome, campos[1]);
            strcpy(nivel, campos[5]);
            fclose(f);
            return 1;
        }
    }

    fclose(f);
    return 0;
}

// Verifica se ID existe e corresponde ao nível desejado
int idExisteEValido(int idBusca, const char *nivelEsperado) {
    FILE *f = fopen(ARQ_USUARIOS, "r");
    if (!f) return 0;
    char linha[MAX_LINHA];
    while (fgets(linha, sizeof(linha), f)) {
        removerBOM(linha);
        linha[strcspn(linha, "\n")] = 0;

        int id;
        char *campos[7];
        char *token = strtok(linha, ";");
        if (!token) continue;

        id = atoi(token);
        campos[0] = token;
        for (int i = 1; i < 7; i++) {
            campos[i] = strtok(NULL, ";");
            if (!campos[i]) break;
        }

        if (id == idBusca && strcmp(campos[5], nivelEsperado) == 0) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

// Obtém o último ID usado em turmas.csv
int obterUltimoIDTurma() {
    FILE *f = fopen(ARQ_TURMAS, "r");
    if (!f) return 0;

    int id = 0;
    char linha[MAX_LINHA];
    while (fgets(linha, sizeof(linha), f)) {
        removerBOM(linha);
        int idAtual = atoi(strtok(linha, ";"));
        if (idAtual > id) id = idAtual;
    }
    fclose(f);
    return id;
}

// Criar nova turma
void criarTurma() {
    FILE *f = fopen(ARQ_TURMAS, "a+");
    if (!f) {
        printf("Erro ao abrir %s\n", ARQ_TURMAS);
        return;
    }

    int novoID = obterUltimoIDTurma() + 1;
    char nome[50], codigo[20];
    int idProfessor;
    char idsAlunos[200];

    printf("Nome da turma: ");
    fgets(nome, sizeof(nome), stdin);
    nome[strcspn(nome, "\n")] = 0;

    printf("Código da turma: ");
    fgets(codigo, sizeof(codigo), stdin);
    codigo[strcspn(codigo, "\n")] = 0;

    printf("ID do professor: ");
    scanf("%d", &idProfessor); getchar();

    if (!idExisteEValido(idProfessor, "Professor")) {
        printf("ID inválido ou não é professor.\n");
        fclose(f);
        return;
    }

    printf("IDs dos alunos (separados por vírgula): ");
    fgets(idsAlunos, sizeof(idsAlunos), stdin);
    idsAlunos[strcspn(idsAlunos, "\n")] = 0;

    // Validar alunos
    char idsCopia[200];
    strcpy(idsCopia, idsAlunos);
    char *idToken = strtok(idsCopia, ",");
    while (idToken) {
        int idAluno = atoi(idToken);
        if (!idExisteEValido(idAluno, "Estudante")) {
            printf("ID de aluno inválido: %d\n", idAluno);
            fclose(f);
            return;
        }
        idToken = strtok(NULL, ",");
    }

    fprintf(f, "%d;%s;%s;%d;%s\n", novoID, nome, codigo, idProfessor, idsAlunos);
    fclose(f);
    printf("Turma criada com sucesso!\n");
}

// Listar turmas
void listarTurmas() {
    FILE *f = fopen(ARQ_TURMAS, "r");
    if (!f) {
        printf("Nenhuma turma encontrada.\n");
        return;
    }

    char linha[MAX_LINHA];
    printf("\n--- TURMAS ---\n");
    printf("ID | Nome | Código | ID Professor | IDs Alunos\n");
    printf("---------------------------------------------------------\n");
    while (fgets(linha, sizeof(linha), f)) {
        removerBOM(linha);
        printf("%s", linha);
    }
    fclose(f);
}

// Excluir turma por ID
void excluirTurma() {
    int idExcluir;
    printf("ID da turma a excluir: ");
    scanf("%d", &idExcluir); getchar();

    FILE *f = fopen(ARQ_TURMAS, "r");
    if (!f) {
        printf("Erro ao abrir %s\n", ARQ_TURMAS);
        return;
    }

    FILE *tmp = fopen("tmp.csv", "w");
    if (!tmp) {
        fclose(f);
        printf("Erro ao criar arquivo temporário.\n");
        return;
    }

    char linha[MAX_LINHA];
    int excluido = 0;
    while (fgets(linha, sizeof(linha), f)) {
        removerBOM(linha);

        char linhaCopia[MAX_LINHA];
        strcpy(linhaCopia, linha);
        int id = atoi(strtok(linhaCopia, ";"));

        if (id == idExcluir) {
            excluido = 1;
            continue;
        }
        fputs(linha, tmp);
    }

    fclose(f);
    fclose(tmp);

    remove(ARQ_TURMAS);
    rename("tmp.csv", ARQ_TURMAS);

    if (excluido)
        printf("Turma excluída com sucesso.\n");
    else
        printf("Turma não encontrada.\n");
}

// Alterar turma por ID
void alterarTurma() {
    int idAlterar;
    printf("ID da turma a alterar: ");
    scanf("%d", &idAlterar);
    getchar();  // Limpar o '\n' do buffer

    FILE *f = fopen(ARQ_TURMAS, "r");
    if (!f) {
        printf("Erro ao abrir %s\n", ARQ_TURMAS);
        return;
    }

    FILE *tmp = fopen("tmp.csv", "w");
    if (!tmp) {
        fclose(f);
        printf("Erro ao criar arquivo temporário.\n");
        return;
    }

    char linha[MAX_LINHA];
    int encontrada = 0;
    while (fgets(linha, sizeof(linha), f)) {
        removerBOM(linha);
        char linhaCopia[MAX_LINHA];
        strcpy(linhaCopia, linha);
        int id = atoi(strtok(linhaCopia, ";"));

        if (id == idAlterar) {
            encontrada = 1;

            char nome[50], codigo[20];
            int idProfessor;
            char idsAlunos[200];

            // Obter dados atuais da turma
            char *token = strtok(linha, ";"); // id
            token = strtok(NULL, ";"); // nome atual
            char nomeAtual[50];
            strcpy(nomeAtual, token);

            token = strtok(NULL, ";"); // código atual
            char codigoAtual[20];
            strcpy(codigoAtual, token);

            token = strtok(NULL, ";"); // id professor atual
            int idProfessorAtual = atoi(token);

            token = strtok(NULL, "\n"); // ids alunos atuais
            char idsAlunosAtuais[200];
            strcpy(idsAlunosAtuais, token);

            // Perguntar o que alterar
            char opcaoAlterar;
            char buffer[200];

            // Nome
            printf("Deseja alterar o nome da turma? (s/n): ");
            opcaoAlterar = getchar();
            getchar();  // Limpar buffer '\n'
            if (opcaoAlterar == 's' || opcaoAlterar == 'S') {
                printf("Novo nome: ");
                fgets(nome, sizeof(nome), stdin);
                nome[strcspn(nome, "\n")] = 0;
                if (strlen(nome) == 0) strcpy(nome, nomeAtual);
            } else {
                strcpy(nome, nomeAtual);
            }

            // Código
            printf("Deseja alterar o código da turma? (s/n): ");
            opcaoAlterar = getchar();
            getchar();
            if (opcaoAlterar == 's' || opcaoAlterar == 'S') {
                printf("Novo código: ");
                fgets(codigo, sizeof(codigo), stdin);
                codigo[strcspn(codigo, "\n")] = 0;
                if (strlen(codigo) == 0) strcpy(codigo, codigoAtual);
            } else {
                strcpy(codigo, codigoAtual);
            }

            // ID professor
            printf("Deseja alterar o ID do professor? (s/n): ");
            opcaoAlterar = getchar();
            getchar();
            if (opcaoAlterar == 's' || opcaoAlterar == 'S') {
                printf("Novo ID do professor: ");
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = 0;
                if (strlen(buffer) == 0) {
                    idProfessor = idProfessorAtual;
                } else {
                    idProfessor = atoi(buffer);
                    if (!idExisteEValido(idProfessor, "Professor")) {
                        printf("ID inválido ou não é professor. Alteração abortada.\n");
                        fclose(f);
                        fclose(tmp);
                        remove("tmp.csv");
                        return;
                    }
                }
            } else {
                idProfessor = idProfessorAtual;
            }

            // IDs alunos
            printf("Deseja alterar os IDs dos alunos? (s/n): ");
            opcaoAlterar = getchar();
            getchar();
            if (opcaoAlterar == 's' || opcaoAlterar == 'S') {
                printf("Novos IDs dos alunos (separados por vírgula): ");
                fgets(idsAlunos, sizeof(idsAlunos), stdin);
                idsAlunos[strcspn(idsAlunos, "\n")] = 0;
                if (strlen(idsAlunos) == 0) strcpy(idsAlunos, idsAlunosAtuais);
            } else {
                strcpy(idsAlunos, idsAlunosAtuais);
            }

            // Validar alunos
            char idsCopia[200];
            strcpy(idsCopia, idsAlunos);
            char *idToken = strtok(idsCopia, ",");
            while (idToken) {
                int idAluno = atoi(idToken);
                if (!idExisteEValido(idAluno, "Estudante")) {
                    printf("ID de aluno inválido: %d. Alteração abortada.\n", idAluno);
                    fclose(f);
                    fclose(tmp);
                    remove("tmp.csv");
                    return;
                }
                idToken = strtok(NULL, ",");
            }

            fprintf(tmp, "%d;%s;%s;%d;%s\n", id, nome, codigo, idProfessor, idsAlunos);
        } else {
            fputs(linha, tmp);
        }
    }

    fclose(f);
    fclose(tmp);

    if (!encontrada) {
        printf("Turma não encontrada.\n");
        remove("tmp.csv");
        return;
    }

    remove(ARQ_TURMAS);
    rename("tmp.csv", ARQ_TURMAS);

    printf("Turma alterada com sucesso.\n");
}

int main() {
    setlocale(LC_ALL, "Portuguese_Brazil.1252");

    char email[50], senha[50], nome[50], nivel[20];
    printf("=== LOGIN ===\n");
    printf("Email: ");
    fgets(email, sizeof(email), stdin);
    email[strcspn(email, "\n")] = 0;

    printf("Senha: ");
    fgets(senha, sizeof(senha), stdin);
    senha[strcspn(senha, "\n")] = 0;

    if (!verificarLogin(email, senha, nome, nivel)) {
        printf("Acesso negado.\n");
        return 1;
    }

    printf("Bem-vindo, %s (%s)\n", nome, nivel);

    char opcao;
    do {
        printf("\n--- MENU ---\n");
        printf("1 - Listar turmas\n");
        printf("2 - Criar turma\n");
        printf("3 - Excluir turma\n");
        printf("4 - Alterar turma\n");
        printf("5 - Sair\n");
        printf("Escolha: ");
        scanf(" %c", &opcao); // Espaço para limpar buffer

        getchar(); // limpar '\n' do buffer após scanf

        switch (opcao) {
            case '1': listarTurmas(); break;
            case '2': criarTurma(); break;
            case '3': excluirTurma(); break;
            case '4': alterarTurma(); break;
            case '5': printf("Saindo...\n"); break;
            default: printf("Opção inválida.\n");
        }
    } while (opcao != '5');

    return 0;
}
