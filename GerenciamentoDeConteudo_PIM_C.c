// Gerenciador de arquivos por curso e matéria para administradores e professores
// Codificação: UTF-8

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>    // Para manipulação de diretórios
#include <sys/stat.h>  // Para verificar existência de pastas
#include <locale.h>    // Para suporte a caracteres especiais

#ifdef _WIN32
#include <direct.h>             // Para mkdir no Windows
#include <windows.h>            // Para configurar console UTF-8 no Windows
#include <conio.h>              // Para _getch()
#define MKDIR(path) _mkdir(path)
#else
#include <unistd.h>
#define MKDIR(path) mkdir(path, 0700)  // Permissão 0700 para UNIX
#endif

#define MAX_LINHA 512
#define ARQUIVO_USUARIOS "usuarios.csv"    // Alterado para CSV
#define DADOS_ARQUIVOS "dados_arquivos.txt"
#define BASE_PASTA "cursos"                 // Pasta base para armazenar cursos

// Função para ler senha escondida (Windows)
#ifdef _WIN32
void lerSenha(char *senha, int maxLen) {
    int i = 0;
    char ch;

    while ((ch = _getch()) != '\r' && i < maxLen - 1) { // '\r' é Enter
        if (ch == '\b') { // Backspace
            if (i > 0) {
                i--;
                printf("\b \b"); // Apaga o asterisco no console
            }
        } else if (ch >= 32 && ch <= 126) { // caracteres imprimíveis
            senha[i++] = ch;
            printf("*"); // imprime asterisco
        }
    }
    senha[i] = '\0';
    printf("\n");
}
#else
// Para sistemas Unix, você pode implementar outra versão usando termios
#endif

void criarPastaSeNaoExistir(const char *pasta) {
    struct stat st = {0};
    if (stat(pasta, &st) == -1) {
        MKDIR(pasta);
    }
}

int verificarLogin(const char *emailInput, const char *senhaInput, char *nivelAcessoRetornado, char *nomeRetornado) {
    FILE *arquivo = fopen(ARQUIVO_USUARIOS, "r");
    if (!arquivo) {
        printf("Erro ao abrir o arquivo de usuários '%s'.\n", ARQUIVO_USUARIOS);
        return 0;
    }

    char linha[MAX_LINHA];

    while (fgets(linha, sizeof(linha), arquivo)) {
        linha[strcspn(linha, "\n")] = '\0';

        char *token = strtok(linha, ";");
        if (!token) continue;

        token = strtok(NULL, ";");
        if (!token) continue;
        strcpy(nomeRetornado, token);

        token = strtok(NULL, ";");
        if (!token) continue;
        char email[100];
        strcpy(email, token);

        token = strtok(NULL, ";");
        if (!token) continue;
        char senha[100];
        strcpy(senha, token);

        token = strtok(NULL, ";"); // Idade
        if (!token) continue;

        token = strtok(NULL, ";");
        if (!token) continue;
        strcpy(nivelAcessoRetornado, token);

        if (strcmp(email, emailInput) == 0 && strcmp(senha, senhaInput) == 0) {
            fclose(arquivo);
            return 1;
        }
    }

    fclose(arquivo);
    return 0;
}

void listarArquivosDiretorioAtual() {
    DIR *d = opendir(".");
    struct dirent *dir;

    if (d) {
        printf("\nTipos de arquivos suportados: .png, .jpg, .mp3, .mp4, .txt, .doc, .docx, .pdf, .csv\n");
        printf("Arquivos disponíveis na pasta atual:\n");

        while ((dir = readdir(d)) != NULL) {
            const char *ext = strrchr(dir->d_name, '.');
            if (ext && (
                strcasecmp(ext, ".png") == 0 || strcasecmp(ext, ".jpg") == 0 ||
                strcasecmp(ext, ".mp3") == 0 || strcasecmp(ext, ".mp4") == 0 ||
                strcasecmp(ext, ".txt") == 0 || strcasecmp(ext, ".doc") == 0 ||
                strcasecmp(ext, ".docx") == 0 || strcasecmp(ext, ".pdf") == 0 ||
                strcasecmp(ext, ".csv") == 0)) {
                printf(" - %s\n", dir->d_name);
            }
        }
        closedir(d);
    } else {
        printf("Erro ao abrir diretório.\n");
    }
}

int copiarArquivo(const char *origem, const char *destino) {
    FILE *src = fopen(origem, "rb");
    if (!src) return 0;

    FILE *dst = fopen(destino, "wb");
    if (!dst) {
        fclose(src);
        return 0;
    }

    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, bytes, dst);
    }

    fclose(src);
    fclose(dst);
    return 1;
}

void listarArquivosGuardadosPorMateria(const char *curso, const char *materia) {
    char pastaConteudo[400];
    snprintf(pastaConteudo, sizeof(pastaConteudo), "%s/%s/materias/%s/conteudo", BASE_PASTA, curso, materia);

    DIR *d = opendir(pastaConteudo);
    struct dirent *dir;

    if (!d) {
        printf("A pasta do conteúdo para a matéria '%s' no curso '%s' não existe ou não pode ser aberta.\n", materia, curso);
        return;
    }

    printf("\nArquivos na pasta de conteúdo da matéria '%s' no curso '%s':\n", materia, curso);
    int achou = 0;
    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
            printf(" - %s\n", dir->d_name);
            achou = 1;
        }
    }

    if (!achou) {
        printf("Nenhum arquivo encontrado na pasta de conteúdo dessa matéria.\n");
    }

    closedir(d);
}

int main() {
    setlocale(LC_ALL, "");

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    char email[100], senha[100], nivelAcesso[50], nome[100];

    printf("=== Login ===\n");
    printf("Email: ");
    fgets(email, sizeof(email), stdin);
    email[strcspn(email, "\n")] = '\0';

    printf("Senha: ");
    lerSenha(senha, sizeof(senha));  // Aqui usa a função que esconde a senha

    if (!verificarLogin(email, senha, nivelAcesso, nome)) {
        printf("Login inválido!\n");
        return 1;
    }

    if (strcmp(nivelAcesso, "Administrador") != 0 && strcmp(nivelAcesso, "Professor") != 0) {
        printf("Acesso negado. Apenas Administradores e Professores têm permissão.\n");
        return 1;
    }

    printf("Bem-vindo(a), %s (%s)\n", nome, nivelAcesso);

    criarPastaSeNaoExistir(BASE_PASTA);

    char opcao;
    do {
        printf("\nMenu:\n");
        printf("1 - Listar arquivos da pasta atual\n");
        printf("2 - Adicionar arquivo por curso e matéria\n");
        printf("3 - Excluir arquivo\n");
        printf("4 - Renomear arquivo\n");
        printf("5 - Listar arquivos guardados por curso e matéria\n");
        printf("6 - Sair\n");
        printf("Escolha: ");
        opcao = getchar();
        getchar();

        if (opcao == '1') {
            listarArquivosDiretorioAtual();
        }
        else if (opcao == '2') {
            char caminho[260], nomeFinal[100], curso[100], materia[100];

            printf("Digite o caminho completo do arquivo a ser adicionado: ");
            fgets(caminho, sizeof(caminho), stdin);
            caminho[strcspn(caminho, "\n")] = '\0';

            printf("Digite o nome final do arquivo (ex: apostila.docx): ");
            fgets(nomeFinal, sizeof(nomeFinal), stdin);
            nomeFinal[strcspn(nomeFinal, "\n")] = '\0';

            printf("Digite o curso relacionado (ex: Engenharia): ");
            fgets(curso, sizeof(curso), stdin);
            curso[strcspn(curso, "\n")] = '\0';

            printf("Digite a matéria relacionada (ex: Matematica): ");
            fgets(materia, sizeof(materia), stdin);
            materia[strcspn(materia, "\n")] = '\0';

            char pastaConteudo[400];
            snprintf(pastaConteudo, sizeof(pastaConteudo), "%s/%s/materias/%s/conteudo", BASE_PASTA, curso, materia);

            criarPastaSeNaoExistir(BASE_PASTA);
            char pastaCurso[300];
            snprintf(pastaCurso, sizeof(pastaCurso), "%s/%s", BASE_PASTA, curso);
            criarPastaSeNaoExistir(pastaCurso);

            char pastaMaterias[350];
            snprintf(pastaMaterias, sizeof(pastaMaterias), "%s/materias", pastaCurso);
            criarPastaSeNaoExistir(pastaMaterias);

            char pastaMateria[400];
            snprintf(pastaMateria, sizeof(pastaMateria), "%s/%s", pastaMaterias, materia);
            criarPastaSeNaoExistir(pastaMateria);

            criarPastaSeNaoExistir(pastaConteudo);

            char destino[500];
            snprintf(destino, sizeof(destino), "%s/%s", pastaConteudo, nomeFinal);

            if (copiarArquivo(caminho, destino)) {
                FILE *dados = fopen(DADOS_ARQUIVOS, "a");
                if (dados) {
                    fprintf(dados, "%s -> %s\n", nomeFinal, destino);
                    fclose(dados);
                }
                printf("Arquivo adicionado com sucesso.\n");
            } else {
                printf("Arquivo não encontrado ou não pode ser aberto. Verifique o caminho informado.\n");
            }
        }
        else if (opcao == '3') {
            char nomeArquivo[100], curso[100], materia[100];

            printf("Digite o curso do arquivo: ");
            fgets(curso, sizeof(curso), stdin);
            curso[strcspn(curso, "\n")] = '\0';

            printf("Digite a matéria do arquivo: ");
            fgets(materia, sizeof(materia), stdin);
            materia[strcspn(materia, "\n")] = '\0';

            printf("Digite o nome do arquivo a ser excluído: ");
            fgets(nomeArquivo, sizeof(nomeArquivo), stdin);
            nomeArquivo[strcspn(nomeArquivo, "\n")] = '\0';

            char caminho[500];
            snprintf(caminho, sizeof(caminho), "%s/%s/materias/%s/conteudo/%s", BASE_PASTA, curso, materia, nomeArquivo);

            if (remove(caminho) == 0) {
                FILE *orig = fopen(DADOS_ARQUIVOS, "r");
                FILE *tmp = fopen("tmp.txt", "w");

                if (orig && tmp) {
                    char linha[MAX_LINHA];
                    while (fgets(linha, sizeof(linha), orig)) {
                        if (!strstr(linha, nomeArquivo)) {
                            fputs(linha, tmp);
                        }
                    }
                    fclose(orig);
                    fclose(tmp);

                    remove(DADOS_ARQUIVOS);
                    rename("tmp.txt", DADOS_ARQUIVOS);

                    printf("Arquivo excluído com sucesso.\n");
                } else {
                    printf("Erro ao atualizar o arquivo de dados.\n");
                    if (orig) fclose(orig);
                    if (tmp) fclose(tmp);
                }
            } else {
                printf("Erro ao excluir o arquivo.\n");
            }
        }
        else if (opcao == '4') {
            char nomeAntigo[100], nomeNovo[100], curso[100], materia[100];

            printf("Digite o curso do arquivo: ");
            fgets(curso, sizeof(curso), stdin);
            curso[strcspn(curso, "\n")] = '\0';

            printf("Digite a matéria do arquivo: ");
            fgets(materia, sizeof(materia), stdin);
            materia[strcspn(materia, "\n")] = '\0';

            printf("Digite o nome atual do arquivo: ");
            fgets(nomeAntigo, sizeof(nomeAntigo), stdin);
            nomeAntigo[strcspn(nomeAntigo, "\n")] = '\0';

            printf("Digite o novo nome do arquivo: ");
            fgets(nomeNovo, sizeof(nomeNovo), stdin);
            nomeNovo[strcspn(nomeNovo, "\n")] = '\0';

            char antigo[500], novo[500];
            snprintf(antigo, sizeof(antigo), "%s/%s/materias/%s/conteudo/%s", BASE_PASTA, curso, materia, nomeAntigo);
            snprintf(novo, sizeof(novo), "%s/%s/materias/%s/conteudo/%s", BASE_PASTA, curso, materia, nomeNovo);

            if (rename(antigo, novo) == 0) {
                FILE *orig = fopen(DADOS_ARQUIVOS, "r");
                FILE *tmp = fopen("tmp.txt", "w");

                if (orig && tmp) {
                    char linha[MAX_LINHA];
                    while (fgets(linha, sizeof(linha), orig)) {
                        if (strstr(linha, nomeAntigo)) {
                            fprintf(tmp, "%s -> %s\n", nomeNovo, novo);
                        } else {
                            fputs(linha, tmp);
                        }
                    }
                    fclose(orig);
                    fclose(tmp);
                    remove(DADOS_ARQUIVOS);
                    rename("tmp.txt", DADOS_ARQUIVOS);

                    printf("Arquivo renomeado com sucesso.\n");
                } else {
                    printf("Erro ao atualizar o arquivo de dados.\n");
                    if (orig) fclose(orig);
                    if (tmp) fclose(tmp);
                }
            } else {
                printf("Erro ao renomear o arquivo.\n");
            }
        }
        else if (opcao == '5') {
            char curso[100], materia[100];
            printf("Digite o curso para listar os arquivos: ");
            fgets(curso, sizeof(curso), stdin);
            curso[strcspn(curso, "\n")] = '\0';

            printf("Digite a matéria para listar os arquivos: ");
            fgets(materia, sizeof(materia), stdin);
            materia[strcspn(materia, "\n")] = '\0';

            listarArquivosGuardadosPorMateria(curso, materia);
        }
        else if (opcao == '6') {
            printf("Saindo...\n");
        }
        else {
            printf("Opção inválida.\n");
        }

    } while (opcao != '6');

    return 0;
}
