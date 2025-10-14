// Gerenciador de arquivos por curso e matéria para administradores e professores
// Codificação: UTF-8

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <locale.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#include <conio.h>    // _getch
#define MKDIR(path) _mkdir(path)
#else
#include <unistd.h>
#include <termios.h>  // controlar echo
#include <sys/types.h>
#include <sys/stat.h>
#define MKDIR(path) mkdir(path, 0700)
#endif

#define MAX_LINHA 512
#define ARQUIVO_USUARIOS "usuarios.csv"
#define DADOS_ARQUIVOS "dados_arquivos.txt"
#define BASE_PASTA "cursos"

void criarPastaSeNaoExistir(const char *pasta) {
    struct stat st = {0};
    if (stat(pasta, &st) == -1) {
        MKDIR(pasta);
    }
}

/* Função para ler senha sem eco */
void lerSenha(char *senha, size_t tamanho) {
#ifdef _WIN32
    size_t idx = 0;
    int ch;
    while ((ch = _getch()) != '\r' && ch != '\n' && idx < tamanho - 1) {
        if (ch == 8) { // backspace
            if (idx > 0) {
                idx--;
                printf("\b \b");
            }
        } else {
            senha[idx++] = (char)ch;
            printf("*");
        }
    }
    senha[idx] = '\0';
    printf("\n");
#else
    struct termios oldt, newt;
    if (tcgetattr(STDIN_FILENO, &oldt) != 0) {
        // fallback para fgets se não for possível manipular termios
        if (fgets(senha, tamanho, stdin) != NULL) {
            senha[strcspn(senha, "\n")] = '\0';
        } else {
            senha[0] = '\0';
        }
        return;
    }
    newt = oldt;
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    if (fgets(senha, tamanho, stdin) != NULL) {
        senha[strcspn(senha, "\n")] = '\0';
    } else {
        senha[0] = '\0';
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\n");
#endif
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

        token = strtok(NULL, ";"); // idade
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

int validarExtensao(const char *arquivo) {
    const char *ext = strrchr(arquivo, '.');
    if (!ext) return 0;

    return (
        strcasecmp(ext, ".pdf") == 0 ||
        strcasecmp(ext, ".txt") == 0 ||
        strcasecmp(ext, ".doc") == 0 ||
        strcasecmp(ext, ".docx") == 0
    );
}

void listarArquivosDiretorioAtual() {
    DIR *d = opendir(".");
    struct dirent *dir;

    if (d) {
        printf("\nTipos de arquivos suportados: .pdf, .txt, .doc, .docx\n");
        printf("Arquivos disponíveis na pasta atual:\n");

        while ((dir = readdir(d)) != NULL) {
            const char *ext = strrchr(dir->d_name, '.');
            if (ext && validarExtensao(dir->d_name)) {
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
    char pastaAtividades[400];
    snprintf(pastaAtividades, sizeof(pastaAtividades), "%s/%s/materias/%s/atividades", BASE_PASTA, curso, materia);

    DIR *d = opendir(pastaAtividades);
    struct dirent *dir;

    if (!d) {
        printf("A pasta de atividades para a matéria '%s' no curso '%s' não existe ou não pode ser aberta.\n", materia, curso);
        return;
    }

    printf("\nArquivos na pasta de atividades da matéria '%s' no curso '%s':\n", materia, curso);
    int achou = 0;
    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
            printf(" - %s\n", dir->d_name);
            achou = 1;
        }
    }

    if (!achou) {
        printf("Nenhum arquivo encontrado na pasta de atividades dessa matéria.\n");
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
    if (fgets(email, sizeof(email), stdin) == NULL) {
        printf("Erro na leitura do email.\n");
        return 1;
    }
    email[strcspn(email, "\n")] = '\0';

    printf("Senha: ");
    /* leitura sem eco */
    lerSenha(senha, sizeof(senha));

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

            if (!validarExtensao(caminho)) {
                printf("Tipo de arquivo inválido. Apenas .pdf, .txt, .doc e .docx são permitidos.\n");
                continue;
            }

            printf("Digite o nome final do arquivo (ex: atividade.docx): ");
            fgets(nomeFinal, sizeof(nomeFinal), stdin);
            nomeFinal[strcspn(nomeFinal, "\n")] = '\0';

            if (!validarExtensao(nomeFinal)) {
                printf("Extensão inválida. Apenas .pdf, .txt, .doc e .docx são permitidas.\n");
                continue;
            }

            printf("Digite o curso relacionado (ex: Engenharia): ");
            fgets(curso, sizeof(curso), stdin);
            curso[strcspn(curso, "\n")] = '\0';

            printf("Digite a matéria relacionada (ex: Matematica): ");
            fgets(materia, sizeof(materia), stdin);
            materia[strcspn(materia, "\n")] = '\0';

            char pastaAtividades[400];
            snprintf(pastaAtividades, sizeof(pastaAtividades), "%s/%s/materias/%s/atividades", BASE_PASTA, curso, materia);

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

            criarPastaSeNaoExistir(pastaAtividades);

            char destino[500];
            snprintf(destino, sizeof(destino), "%s/%s", pastaAtividades, nomeFinal);

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
            snprintf(caminho, sizeof(caminho), "%s/%s/materias/%s/atividades/%s", BASE_PASTA, curso, materia, nomeArquivo);

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
                printf("Erro ao excluir o arquivo. Verifique se o caminho e nome estão corretos.\n");
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

            if (!validarExtensao(nomeNovo)) {
                printf("Extensão inválida. Apenas .pdf, .txt, .doc e .docx são permitidas.\n");
                continue;
            }

            char antigo[500], novo[500];
            snprintf(antigo, sizeof(antigo), "%s/%s/materias/%s/atividades/%s", BASE_PASTA, curso, materia, nomeAntigo);
            snprintf(novo, sizeof(novo), "%s/%s/materias/%s/atividades/%s", BASE_PASTA, curso, materia, nomeNovo);

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
                printf("Erro ao renomear o arquivo. Verifique se o nome está correto.\n");
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

