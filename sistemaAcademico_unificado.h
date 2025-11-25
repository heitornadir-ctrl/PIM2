#ifndef SISTEMAACADEMICO_H_INCLUDED
#define SISTEMAACADEMICO_H_INCLUDED
#define MASTER_PASSWORD "admin"
/*
 * sistemaacademico.h - Versao unificada e corrigida.
 *
 * Contem:
 * - Estrutura UsuarioCSV e funcoes utilitarias.
 * - Funcoes de manipulacao de arquivo (CSV plano).
 * - Funcoes CRUD (Create, Read, Update, Delete) de usuarios.
 * - Menus de interacao para diferentes niveis de acesso.
 */

#define _CRT_SECURE_NO_WARNINGS

/* Removido include da libsodium */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <locale.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#include <conio.h>
#define MKDIR(p) _mkdir(p)
#define PATH_SEP "\\"
#define STRCASECMP _stricmp
#else
#include <unistd.h>
#include <sys/stat.h>
#include <strings.h> /* strcasecmp */
#include <dirent.h>
#define MKDIR(p) mkdir((p), 0700)
#define PATH_SEP "/"
#define STRCASECMP strcasecmp

#endif

#define ARQ_SISTEMA "sistemaAcademico.csv"
#define DIR_BACKUPS "backups"
#define MAX_LINE 2048
#define MAX_TURMAS 256
#define DIR_ATIVIDADES "atividades"

/* ----------------- ESTRUTURA USUARIO ----------------- */
typedef struct
{
    int id;
    char nome[256];
    char email[256];

    /* Campo de senha revertido para tamanho padrao (texto puro) */
    char senha[128];

    char nivel[64];
    char curso[128];
    char turma[64];
    int idade;
    float np1, np2, pim, media;
    char atividade[32];
} UsuarioCSV;

/* ----------------- DECLARACOES DE FUNCOES ----------------- */
/* ... (todas as suas declarações de função permanecem as mesmas) ... */
void initSistema(void);
void trim(char *s);

int validarEmail(const char *email);
void lerSenhaOculta(char *senha, size_t maxLen);
int arquivoExiste(const char *nome);
void garantirPasta(const char *pasta);
int backupSistema(void);
void criarArquivoSistemaSeNaoExiste(void);
int parseLinhaUsuario(const char *line, UsuarioCSV *u);
void formatarLinhaUsuario(const UsuarioCSV *u, char *out, size_t outsz);
int verificarLoginUnico(const char *email, const char *senha, UsuarioCSV *out);
int obterUltimoIDUsuarios(void);
int emailDuplicado(const char *email);
int adicionarUsuario(const UsuarioCSV *u_in);
int listarTodosUsuarios(void);
int alterarUsuarioPorID(int idBusca, const UsuarioCSV *novo);
int excluirUsuarioPorID(int idBusca);
void menuAlunoUnificado(const UsuarioCSV *u);
void menuProfessorUnificado(const UsuarioCSV *u);
void menuCoordenadorUnificado(const UsuarioCSV *u);
void menuAdministradorUnificado(const UsuarioCSV *u);
void executarSistema(void);
int listarApenasAlunos(void);
int buscarUsuarioPorID(int idBusca, UsuarioCSV *out);
float calcularMedia(float np1, float np2, float pim);
void lancarNotasUI(void);
void listarTurmasUnicas(void);
void movimentarAlunoUI(void);
void gerenciarTurmasUI(void);
int copiarArquivo(const char *origem, const char *destino);
void enviarAtividadeUI(const UsuarioCSV *u);
int copiarArquivo(const char *origem, const char *destino);
void enviarAtividadeUI(const UsuarioCSV *u);
void listarAtividadesTurma(const UsuarioCSV *u);

/* ----------------- DEFINICOES DE FUNCOES ----------------- */

void initSistema(void)
{
    setlocale(LC_ALL, "");
#ifdef _WIN32
    /* tenta forcar UTF-8 no console Windows */
    SetConsoleOutputCP(65001); // CP_UTF8 = 65001
    SetConsoleCP(65001);       // CP_UTF8 = 65001
#endif

    /* Inicializacao da libsodium removida */
}

void trim(char *s)
{
    if (!s)
        return;
    char *p = s;
    while (*p && isspace((unsigned char)*p))
        p++;
    if (p != s)
        memmove(s, p, strlen(p) + 1);
    size_t L = strlen(s);
    while (L > 0 && isspace((unsigned char)s[L - 1]))
        s[--L] = '\0';
}

/* Funcoes de padronizacao de texto */

/**
 * Converte toda a string para MAIUSCULAS.
 * Ex: "ads-1a" -> "ADS-1A"
 */
void stringToUpper(char *s)
{
    if (!s)
        return;
    for (; *s; ++s)
        *s = (char)toupper((unsigned char)*s);
}

/**
 * Converte toda a string para minusculas.
 * Ex: "Admin@EMAIL.com" -> "admin@email.com"
 */
void stringToLower(char *s)
{
    if (!s)
        return;
    for (; *s; ++s)
        *s = (char)tolower((unsigned char)*s);
}

/**
 * Converte a string para "Title Case" (Primeira Letra Maiuscula).
 * Ex: "krigor" -> "Krigor"
 * Ex: "krigor da silva" -> "Krigor Da Silva"
 */
void stringToTitle(char *s)
{
    if (!s)
        return;
    int cap = 1; /* Flag para capitalizar o proximo caractere */
    for (; *s; ++s)
    {
        if (isspace((unsigned char)*s))
        {
            cap = 1;
        }
        else if (cap)
        {
            *s = (char)toupper((unsigned char)*s);
            cap = 0;
        }
        else
        {
            *s = (char)tolower((unsigned char)*s);
        }
    }
}

int validarEmail(const char *email)
{
    if (!email)
        return 0;
    const char *at = strchr(email, '@');
    if (!at || at == email)
        return 0;
    const char *dot = strchr(at + 1, '.');
    if (!dot || dot == at + 1)
        return 0;
    if (*(dot + 1) == '\0')
        return 0;
    return 1;
}

void lerSenhaOculta(char *senha, size_t maxLen)
{
    if (!senha || maxLen == 0)
        return;
#ifdef _WIN32
    size_t idx = 0;
    int ch;
    while ((ch = _getch()) != '\r' && ch != '\n' && idx + 1 < maxLen)
    {
        if (ch == '\b')
        {
            if (idx > 0)
            {
                idx--;
                printf("\b \b");
            }
        }
        else
        {
            senha[idx++] = (char)ch;
            printf("*");
        }
    }
    senha[idx] = '\0';
    printf("\n");
#else
    /* No Linux/macOS, o terminal deve estar configurado para modo cbreak/raw,
       o que 'conio.h' (simulada) faria. Usaremos fgets para simplicidade */
    if (fgets(senha, (int)maxLen, stdin))
    {
        senha[strcspn(senha, "\n")] = '\0';
    }
    else
        senha[0] = '\0';
#endif
}

int arquivoExiste(const char *nome)
{
    if (!nome)
        return 0;
    FILE *f = fopen(nome, "r");
    if (f)
    {
        fclose(f);
        return 1;
    }
    return 0;
}

void garantirPasta(const char *pasta)
{
    if (!pasta)
        return;
    if (!arquivoExiste(pasta))
        MKDIR(pasta);
}

void now_str(char *dest, size_t n)
{
    time_t t = time(NULL);
    struct tm tm;
#ifdef _WIN32
    struct tm *tptr = localtime(&t);
    if (tptr)
        tm = *tptr;
    else
        memset(&tm, 0, sizeof(tm));
#else
    localtime_r(&t, &tm);
#endif
    strftime(dest, n, "%Y%m%d_%H%M%S", &tm);
}

/* ----------------- BACKUP ----------------- */

int backupSistema(void)
{
    if (!arquivoExiste(ARQ_SISTEMA))
        return 0;
    garantirPasta(DIR_BACKUPS);
    char stamp[64];
    now_str(stamp, sizeof(stamp));
    char dest[512];
    snprintf(dest, sizeof(dest), "%s%ssistemaAcademico_backup_%s.csv", DIR_BACKUPS, PATH_SEP, stamp);
    FILE *fs = fopen(ARQ_SISTEMA, "rb");
    if (!fs)
        return 0;
    FILE *fd = fopen(dest, "wb");
    if (!fd)
    {
        fclose(fs);
        return 0;
    }
    char buf[4096];
    size_t r;
    while ((r = fread(buf, 1, sizeof(buf), fs)) > 0)
        fwrite(buf, 1, r, fd);
    fclose(fs);
    fclose(fd);
    printf("Backup criado com sucesso: %s\n", dest);
    return 1;
}

int copiarArquivo(const char *origem, const char *destino)
{
    if (!arquivoExiste(origem))
    {
        printf("Erro de copia: Arquivo de origem nao existe: %s\n", origem);
        return 0;
    }
    FILE *fs = fopen(origem, "rb");
    if (!fs)
    {
        printf("Erro ao abrir origem: %s\n", origem);
        return 0;
    }
    FILE *fd = fopen(destino, "wb");
    if (!fd)
    {
        fclose(fs);
        printf("Erro ao criar destino: %s\n", destino);
        return 0;
    }
    char buf[4096];
    size_t r;
    while ((r = fread(buf, 1, sizeof(buf), fs)) > 0)
    {
        fwrite(buf, 1, r, fd);
    }
    fclose(fs);
    fclose(fd);
    return 1;
}

void enviarAtividadeUI(const UsuarioCSV *u)
{
    if (u->turma[0] == '\0' || STRCASECMP(u->turma, "Geral") == 0)
    {
        printf("\nErro: Voce precisa ter uma turma especifica (ex: 'ADS-1A')\n");
        printf("associada ao seu perfil de professor para poder enviar atividades.\n");
        printf("Peça a um Administrador para atualizar seu cadastro.\n");
        return;
    }
    printf("\n--- ENVIAR ATIVIDADE PARA TURMA: %s ---\n", u->turma);
    printf("O arquivo PDF deve estar na mesma pasta do executavel.\n");
    char nomeArquivo[256];
    printf("Digite o nome exato do arquivo (ex: Atividade1.pdf): ");
    if (!fgets(nomeArquivo, sizeof(nomeArquivo), stdin))
    {
        return;
    }
    trim(nomeArquivo);
    if (nomeArquivo[0] == '\0')
    {
        printf("Nome do arquivo nao pode ser vazio. Operacao cancelada.\n");
        return;
    }
    if (!arquivoExiste(nomeArquivo))
    {
        printf("\nErro: Arquivo '%s' nao encontrado.\n", nomeArquivo);
        printf("Por favor, coloque o arquivo PDF no mesmo diretorio do programa e tente novamente.\n");
        return;
    }
    char pastaTurma[512];
    snprintf(pastaTurma, sizeof(pastaTurma), "%s%s%s", DIR_ATIVIDADES, PATH_SEP, u->turma);
    garantirPasta(DIR_ATIVIDADES);
    garantirPasta(pastaTurma);
    char destArquivo[1024];
    snprintf(destArquivo, sizeof(destArquivo), "%s%s%s", pastaTurma, PATH_SEP, nomeArquivo);
    if (copiarArquivo(nomeArquivo, destArquivo))
    {
        printf("\nSucesso!\n");
        printf("Atividade '%s' enviada para a pasta da turma '%s'.\n", nomeArquivo, u->turma);
        printf("Localizacao: %s\n", destArquivo);
    }
    else
    {
        printf("\nErro: Nao foi possivel copiar o arquivo para o destino.\n");
    }
}
/* ----------------- ARQUIVO INICIAL ----------------- */

void criarArquivoSistemaSeNaoExiste(void)
{
    if (arquivoExiste(ARQ_SISTEMA))
        return;
    FILE *f = fopen(ARQ_SISTEMA, "w");
    if (!f)
    {
        printf("Erro ao criar arquivo do sistema!\n");
        return;
    }
    /* Cabecalho: id;nome;email;senha;nivel;curso;turma;idade;np1;np2;pim;media;atividade */
    fprintf(f, "[USUARIOS]\n");
    fprintf(f, "id;nome;email;senha;nivel;curso;turma;idade;np1;np2;pim;media;atividade\n");

    /* Usuario admin padrao com senha "admin" em texto puro */
    fprintf(f, "1;Administrador;admin@admin.com;admin;Administrador;Sistema;Geral;30;0.00;0.00;0.00;0.00;Ativo\n");

    fclose(f);
    printf("Arquivo do sistema criado com usuario padrao: admin@admin.com / senha: admin\n");
}

/* ----------------- PARSE / FORMATACAO ----------------- */

int parseLinhaUsuario(const char *line, UsuarioCSV *u)
{
    if (!line || !u)
        return 0;
    char *buf = strdup(line);
    if (!buf)
        return 0;
    trim(buf);
    if (buf[0] == '\0')
    {
        free(buf);
        return 0;
    }
    if (STRCASECMP(buf, "id;nome;email;senha;nivel;curso;turma;idade;np1;np2;pim;media;atividade\n") == 0)
    {
        free(buf);
        return 0;
    }

    char *tok = strtok(buf, ";");
    if (!tok)
    {
        free(buf);
        return 0;
    }
    u->id = atoi(tok);

    tok = strtok(NULL, ";");
    if (!tok)
    {
        free(buf);
        return 0;
    }
    strncpy(u->nome, tok, sizeof(u->nome) - 1);
    u->nome[sizeof(u->nome) - 1] = 0;
    tok = strtok(NULL, ";");
    if (!tok)
    {
        free(buf);
        return 0;
    }
    strncpy(u->email, tok, sizeof(u->email) - 1);
    u->email[sizeof(u->email) - 1] = 0;

    /* Le a senha (texto puro) do CSV */
    tok = strtok(NULL, ";");
    if (!tok)
    {
        free(buf);
        return 0;
    }
    strncpy(u->senha, tok, sizeof(u->senha) - 1);
    u->senha[sizeof(u->senha) - 1] = 0;

    tok = strtok(NULL, ";");
    if (!tok)
    {
        free(buf);
        return 0;
    }
    strncpy(u->nivel, tok, sizeof(u->nivel) - 1);
    u->nivel[sizeof(u->nivel) - 1] = 0;
    tok = strtok(NULL, ";");
    if (!tok)
    {
        free(buf);
        return 0;
    }
    strncpy(u->curso, tok, sizeof(u->curso) - 1);
    u->curso[sizeof(u->curso) - 1] = 0;
    tok = strtok(NULL, ";");
    if (!tok)
    {
        free(buf);
        return 0;
    }
    strncpy(u->turma, tok, sizeof(u->turma) - 1);
    u->turma[sizeof(u->turma) - 1] = 0;
    tok = strtok(NULL, ";");
    if (!tok)
        tok = "0";
    u->idade = atoi(tok);
    tok = strtok(NULL, ";");
    if (!tok)
        tok = "0";
    u->np1 = (float)atof(tok);
    tok = strtok(NULL, ";");
    if (!tok)
        tok = "0";
    u->np2 = (float)atof(tok);
    tok = strtok(NULL, ";");
    if (!tok)
        tok = "0";
    u->pim = (float)atof(tok);
    tok = strtok(NULL, ";");
    if (!tok)
        tok = "0";
    u->media = (float)atof(tok);
    tok = strtok(NULL, ";");
    if (!tok)
        tok = "Ativo";
    strncpy(u->atividade, tok, sizeof(u->atividade) - 1);
    u->atividade[sizeof(u->atividade) - 1] = 0;

    trim(u->nome);
    trim(u->email);
    trim(u->senha);
    trim(u->nivel);
    trim(u->curso);
    trim(u->turma);
    trim(u->atividade);
    free(buf);
    return 1;
}

void formatarLinhaUsuario(const UsuarioCSV *u, char *out, size_t outsz)
{
    if (!u || !out)
        return;
    /* O formato de float foi corrigido para %.2f */
    snprintf(out, outsz, "%d;%s;%s;%s;%s;%s;%s;%d;%.2f;%.2f;%.2f;%.2f;%s\n",
             u->id,
             u->nome,
             u->email,
             u->senha, /* Salva a senha em texto puro */
             u->nivel,
             u->curso,
             u->turma,
             u->idade,
             u->np1, u->np2, u->pim, u->media,
             u->atividade[0] ? u->atividade : "Ativo");
}

/* ----------------- OPERACOES SOBRE ARQUIVO (CSV PLANO) ----------------- */

/* Funcao revertida para comparar senha em texto puro (strcmp) */
int verificarLoginUnico(const char *email, const char *senha_digitada, UsuarioCSV *out)
{
    if (!email || !senha_digitada || !arquivoExiste(ARQ_SISTEMA))
        return 0;

    FILE *f = fopen(ARQ_SISTEMA, "r");
    if (!f)
        return 0;

    char linha[MAX_LINE];
    /* pula cabecalho */
    if (!fgets(linha, sizeof(linha), f))
    {
        fclose(f);
        return 0;
    }

    while (fgets(linha, sizeof(linha), f))
    {
        char tmp[MAX_LINE];
        strncpy(tmp, linha, sizeof(tmp) - 1);
        tmp[sizeof(tmp) - 1] = 0;
        trim(tmp);

        /* Preenche 'out' com os dados do usuario da linha atual */
        if (!parseLinhaUsuario(tmp, out))
            continue;

        /* 1. Verifica se o email bate (case-insensitive) */
        if (STRCASECMP(out->email, email) == 0)
        {

            /* 2. VERIFICACAO EM TEXTO PURO:
             * Compara a senha salva no CSV (out->senha) com a senha digitada.
             * Usamos strcmp (case-sensitive) para senhas.
             */
            if (strcmp(out->senha, senha_digitada) == 0)
            {
                /* SUCESSO! Email e Senha corretos. */
                fclose(f);
                return 1;
            }

            /* Email bateu, mas a senha nao. */
            fclose(f);
            return 0;
        }
    }
    fclose(f);
    return 0; /* Email nao encontrado */
}

int obterUltimoIDUsuarios(void)
{
    if (!arquivoExiste(ARQ_SISTEMA))
        return 0;
    FILE *f = fopen(ARQ_SISTEMA, "r");
    if (!f)
        return 0;
    char linha[MAX_LINE];
    int maxID = 0;
    if (!fgets(linha, sizeof(linha), f))
    {
        fclose(f);
        return 0;
    }
    while (fgets(linha, sizeof(linha), f))
    {
        char tmp[MAX_LINE];
        strncpy(tmp, linha, sizeof(tmp) - 1);
        tmp[sizeof(tmp) - 1] = 0;
        trim(tmp);
        UsuarioCSV u;
        memset(&u, 0, sizeof(u));
        char *p = tmp;
        while (*p && isspace((unsigned char)*p))
            p++;
        if (!isdigit((unsigned char)*p))
            continue;
        int id = atoi(p);
        if (id > maxID)
            maxID = id;
    }
    fclose(f);
    return maxID;
}

int emailDuplicado(const char *email)
{
    if (!email || !arquivoExiste(ARQ_SISTEMA))
        return 0;
    FILE *f = fopen(ARQ_SISTEMA, "r");
    if (!f)
        return 0;
    char linha[MAX_LINE];
    if (!fgets(linha, sizeof(linha), f))
    {
        fclose(f);
        return 0;
    }
    while (fgets(linha, sizeof(linha), f))
    {
        char tmp[MAX_LINE];
        strncpy(tmp, linha, sizeof(tmp) - 1);
        tmp[sizeof(tmp) - 1] = 0;
        trim(tmp);
        UsuarioCSV u;
        memset(&u, 0, sizeof(u));
        if (!parseLinhaUsuario(tmp, &u))
            continue;
        if (STRCASECMP(u.email, email) == 0)
        {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

/* ----------------- CRUD ----------------- */

int adicionarUsuario(const UsuarioCSV *u_in)
{
    if (!u_in)
        return 0;

    /* A senha eh texto puro, mas nao pode ser vazia */
    if (strlen(u_in->nome) == 0 || strlen(u_in->email) == 0 || strlen(u_in->senha) == 0 || strlen(u_in->nivel) == 0)
    {
        printf("Campos obrigatorios vazios.\n");
        return 0;
    }
    if (!validarEmail(u_in->email))
    {
        printf("Email invalido.\n");
        return 0;
    }
    if (emailDuplicado(u_in->email))
    {
        printf("Email ja cadastrado.\n");
        return 0;
    }

    int novoID = obterUltimoIDUsuarios() + 1;
    UsuarioCSV u = *u_in;
    u.id = novoID;
    if (!u.atividade[0])
        strncpy(u.atividade, "Ativo", sizeof(u.atividade) - 1);
    u.atividade[sizeof(u.atividade) - 1] = 0;

    char linha[MAX_LINE];
    formatarLinhaUsuario(&u, linha, sizeof(linha));
    backupSistema();

    FILE *f = fopen(ARQ_SISTEMA, "a");
    if (!f)
    {
        printf("Erro ao abrir arquivo para adicionar.\n");
        return 0;
    }
    fputs(linha, f);
    fclose(f);
    printf("Usuario adicionado com ID %d\n", novoID);
    return 1;
}

int listarTodosUsuarios(void)
{
    if (!arquivoExiste(ARQ_SISTEMA))
    {
        printf("Nenhum usuario cadastrado.\n");
        return 0;
    }
    FILE *f = fopen(ARQ_SISTEMA, "r");
    if (!f)
    {
        printf("Erro ao abrir arquivo.\n");
        return 0;
    }
    char linha[MAX_LINE];
    if (!fgets(linha, sizeof(linha), f))
    {
        fclose(f);
        printf("Arquivo vazio.\n");
        return 0;
    }

    printf("\n========================================================= LISTAGEM DE USUARIOS =============================================================\n");
    printf("%-4s | %-25.25s | %-25.25s | %-20.20s | %-13.13s | %-10.10s | %-10.10s | %-3s | %-5s | %-5s | %-5s | %-5s | %-5s\n",
           "ID", "Nome", "Email", "Senha", "Nivel", "Curso", "Turma", "Id", "NP1", "NP2", "PIM", "Media", "Atv.");
    printf("----------------------------------------------------------------------------------------------------------------------------------------------\n");

    while (fgets(linha, sizeof(linha), f))
    {
        char tmp[MAX_LINE];
        strncpy(tmp, linha, sizeof(tmp) - 1);
        tmp[sizeof(tmp) - 1] = 0;
        trim(tmp);
        UsuarioCSV u;
        memset(&u, 0, sizeof(u));
        if (!parseLinhaUsuario(tmp, &u))
            continue;

        printf("%-4d | %-25.25s | %-25.25s | %-20.20s | %-13.13s | %-10.10s | %-10.10s | %-3d | %-5.2f | %-5.2f | %-5.2f | %-5.2f | %-5.5s\n",
               u.id, u.nome, u.email,
               "********************", // Senha oculta
               u.nivel, u.curso, u.turma, u.idade,
               u.np1, u.np2, u.pim, u.media, u.atividade);
    }
    printf("==============================================================================================================================================\n");
    fclose(f);
    return 1;
}

int alterarUsuarioPorID(int idBusca, const UsuarioCSV *novo)
{
    if (!arquivoExiste(ARQ_SISTEMA) || !novo)
        return 0;
    FILE *f = fopen(ARQ_SISTEMA, "r");
    if (!f)
        return 0;
    char **linhas = NULL;
    size_t count = 0;
    char linha[MAX_LINE];
    while (fgets(linha, sizeof(linha), f))
    {
        char *c = strdup(linha);
        if (!c)
        {
            fclose(f);
            for (size_t i = 0; i < count; i++)
                free(linhas[i]);
            free(linhas);
            return 0;
        }
        char **tmp = realloc(linhas, sizeof(char *) * (count + 1));
        if (!tmp)
        {
            free(c);
            fclose(f);
            for (size_t i = 0; i < count; i++)
                free(linhas[i]);
            free(linhas);
            return 0;
        }
        linhas = tmp;
        linhas[count++] = c;
    }
    fclose(f);

    int found = 0;
    for (size_t i = 0; i < count; i++)
    {
        char copy[MAX_LINE];
        strncpy(copy, linhas[i], sizeof(copy) - 1);
        copy[sizeof(copy) - 1] = 0;
        trim(copy);
        if (i == 0 && (STRCASECMP(copy, "id;nome;email;senha;nivel;curso;turma;idade;np1;np2;pim;media;atividade") == 0))
            continue;

        UsuarioCSV u;
        memset(&u, 0, sizeof(u));
        if (!parseLinhaUsuario(copy, &u))
            continue;

        if (u.id == idBusca)
        {
            char nova[MAX_LINE];
            UsuarioCSV temp = *novo;
            temp.id = idBusca;

            /* Se a senha passada em 'novo' estiver vazia, significa "nao alterar".
             * Entao, copiamos a senha antiga (texto puro) de volta.
             */
            if (novo->senha[0] == '\0')
            {
                strncpy(temp.senha, u.senha, sizeof(temp.senha) - 1);
            }

            if (!temp.atividade[0])
                strncpy(temp.atividade, "Ativo", sizeof(temp.atividade) - 1);
            temp.atividade[sizeof(temp.atividade) - 1] = 0;

            formatarLinhaUsuario(&temp, nova, sizeof(nova));
            free(linhas[i]);
            linhas[i] = strdup(nova);
            found = 1;
            break;
        }
    }

    if (!found)
    {
        for (size_t i = 0; i < count; i++)
            free(linhas[i]);
        free(linhas);
        printf("Usuario ID %d nao encontrado.\n", idBusca);
        return 0;
    }

    backupSistema();
    FILE *fw = fopen(ARQ_SISTEMA, "w");
    if (!fw)
    {
        for (size_t i = 0; i < count; i++)
            free(linhas[i]);
        free(linhas);
        return 0;
    }
    for (size_t i = 0; i < count; i++)
    {
        fputs(linhas[i], fw);
        free(linhas[i]);
    }
    free(linhas);
    fclose(fw);
    printf("Usuario ID %d alterado com sucesso.\n", idBusca);
    return 1;
}

int excluirUsuarioPorID(int idBusca)
{
    if (!arquivoExiste(ARQ_SISTEMA))
        return 0;
    FILE *f = fopen(ARQ_SISTEMA, "r");
    if (!f)
        return 0;
    char **linhas = NULL;
    size_t count = 0;
    char linha[MAX_LINE];
    while (fgets(linha, sizeof(linha), f))
    {
        char *c = strdup(linha);
        if (!c)
        {
            fclose(f);
            for (size_t i = 0; i < count; i++)
                free(linhas[i]);
            free(linhas);
            return 0;
        }
        char **tmp = realloc(linhas, sizeof(char *) * (count + 1));
        if (!tmp)
        {
            free(c);
            fclose(f);
            for (size_t i = 0; i < count; i++)
                free(linhas[i]);
            free(linhas);
            return 0;
        }
        linhas = tmp;
        linhas[count++] = c;
    }
    fclose(f);

    int removed = 0;
    FILE *fw = fopen("tmp_sistema.csv", "w");
    if (!fw)
    {
        for (size_t i = 0; i < count; i++)
            free(linhas[i]);
        free(linhas);
        return 0;
    }

    for (size_t i = 0; i < count; i++)
    {
        char copy[MAX_LINE];
        strncpy(copy, linhas[i], sizeof(copy) - 1);
        copy[sizeof(copy) - 1] = 0;
        trim(copy);
        if (i == 0 && (STRCASECMP(copy, "id;nome;email;senha;nivel;curso;turma;idade;np1;np2;pim;media;atividade") == 0))
        {
            fputs(linhas[i], fw);
            continue;
        }
        UsuarioCSV u;
        memset(&u, 0, sizeof(u));
        if (!parseLinhaUsuario(copy, &u))
        {
            fputs(linhas[i], fw);
            continue;
        }
        if (u.id == idBusca)
        {
            removed = 1;
        }
        else
            fputs(linhas[i], fw);
    }
    for (size_t i = 0; i < count; i++)
        free(linhas[i]);
    free(linhas);
    fclose(fw);

    if (!removed)
    {
        remove("tmp_sistema.csv");
        printf("Usuario ID %d nao encontrado.\n", idBusca);
        return 0;
    }

    backupSistema();
#ifdef _WIN32
    remove(ARQ_SISTEMA);
    rename("tmp_sistema.csv", ARQ_SISTEMA);
#else
    if (rename("tmp_sistema.csv", ARQ_SISTEMA) != 0)
    {
        printf("Erro ao substituir arquivo.\n");
        remove("tmp_sistema.csv");
        return 0;
    }
#endif
    printf("Usuario ID %d excluido com sucesso.\n", idBusca);
    return 1;
}
/**
 * LISTAGEM RESTRITA: Mostra TODOS os usuarios com senhas EM TEXTO PURO.
 * Requer autenticacao pela interface.
 */
int listarTodosUsuariosComSenha(void)
{
    if (!arquivoExiste(ARQ_SISTEMA))
    {
        printf("Nenhum usuario cadastrado.\n");
        return 0;
    }
    FILE *f = fopen(ARQ_SISTEMA, "r");
    if (!f)
    {
        printf("Erro ao abrir arquivo.\n");
        return 0;
    }
    char linha[MAX_LINE];
    if (!fgets(linha, sizeof(linha), f))
    {
        fclose(f);
        printf("Arquivo vazio.\n");
        return 0;
    }

    printf("\n====================================================== LISTAGEM DE USUARIOS (COM SENHA) ======================================================\n");
    printf("%-4s | %-25.25s | %-25.25s | %-20.20s | %-13.13s | %-10.10s | %-10.10s | %-3s | %-5s | %-5s | %-5s | %-5s | %-5s\n",
           "ID", "Nome", "Email", "Senha (!!!)", "Nivel", "Curso", "Turma", "Id", "NP1", "NP2", "PIM", "Media", "Atv.");
    printf("----------------------------------------------------------------------------------------------------------------------------------------------\n");

    while (fgets(linha, sizeof(linha), f))
    {
        char tmp[MAX_LINE];
        strncpy(tmp, linha, sizeof(tmp) - 1);
        tmp[sizeof(tmp) - 1] = 0;
        trim(tmp);
        UsuarioCSV u;
        memset(&u, 0, sizeof(u));
        if (!parseLinhaUsuario(tmp, &u))
            continue;

        printf("%-4d | %-25.25s | %-25.25s | %-20.20s | %-13.13s | %-10.10s | %-10.10s | %-3d | %-5.2f | %-5.2f | %-5.2f | %-5.2f | %-5.5s\n",
               u.id, u.nome, u.email,
               u.senha, // A SENHA REAL E MOSTRADA AQUI
               u.nivel, u.curso, u.turma, u.idade,
               u.np1, u.np2, u.pim, u.media, u.atividade);
    }
    printf("==============================================================================================================================================\n");
    fclose(f);
    return 1;
}

int listarApenasAlunos(void)
{
    if (!arquivoExiste(ARQ_SISTEMA))
    {
        printf("Nenhum usuario cadastrado.\n");
        return 0;
    }
    FILE *f = fopen(ARQ_SISTEMA, "r");
    if (!f)
    {
        printf("Erro ao abrir arquivo.\n");
        return 0;
    }
    char linha[MAX_LINE];
    int countAlunos = 0;
    if (!fgets(linha, sizeof(linha), f))
    {
        fclose(f);
        printf("Arquivo vazio.\n");
        return 0;
    }
    printf("\n============================= LISTAGEM DE ALUNOS =============================\n");
    printf("%-5s | %-30s | %-30s | %-10s | %-10s\n", "ID", "Nome", "Email", "Turma", "Curso");
    printf("--------------------------------------------------------------------------------\n");
    while (fgets(linha, sizeof(linha), f))
    {
        char tmp[MAX_LINE];
        strncpy(tmp, linha, sizeof(tmp) - 1);
        tmp[sizeof(tmp) - 1] = 0;
        trim(tmp);
        UsuarioCSV u;
        memset(&u, 0, sizeof(u));
        if (!parseLinhaUsuario(tmp, &u))
            continue;
        if (STRCASECMP(u.nivel, "Aluno") == 0)
        {
            printf("%-5d | %-30.30s | %-30.30s | %-10.10s | %-10.10s\n",
                   u.id, u.nome, u.email, u.turma, u.curso);
            countAlunos++;
        }
    }
    fclose(f);
    if (countAlunos == 0)
    {
        printf("Nenhum aluno encontrado.\n");
    }
    printf("================================================================================\n");
    return countAlunos > 0;
}

int buscarUsuarioPorID(int idBusca, UsuarioCSV *out)
{
    if (!out || !arquivoExiste(ARQ_SISTEMA))
        return 0;
    FILE *f = fopen(ARQ_SISTEMA, "r");
    if (!f)
        return 0;
    char linha[MAX_LINE];
    if (!fgets(linha, sizeof(linha), f))
    {
        fclose(f);
        return 0;
    }
    while (fgets(linha, sizeof(linha), f))
    {
        char tmp[MAX_LINE];
        strncpy(tmp, linha, sizeof(tmp) - 1);
        tmp[sizeof(tmp) - 1] = 0;
        trim(tmp);
        memset(out, 0, sizeof(UsuarioCSV));
        if (!parseLinhaUsuario(tmp, out))
            continue;
        if (out->id == idBusca)
        {
            fclose(f);
            return 1; /* Encontrado */
        }
    }
    fclose(f);
    return 0; /* Nao encontrado */
}

float calcularMedia(float np1, float np2, float pim)
{
    return (np1 * 0.4) + (np2 * 0.4) + (pim * 0.2);
}

void listarAtividadesTurma(const UsuarioCSV *u)
{
    if (u->turma[0] == '\0' || STRCASECMP(u->turma, "Geral") == 0)
    {
        printf("\nVoce nao esta associado a uma turma especifica.\n");
        printf("Nao e possivel listar atividades.\n");
        return;
    }
    char pastaTurma[512];
    snprintf(pastaTurma, sizeof(pastaTurma), "%s%s%s", DIR_ATIVIDADES, PATH_SEP, u->turma);
    printf("\n--- ATIVIDADES PARA A TURMA: %s ---\n", u->turma);
    int count = 0;

#ifdef _WIN32
    char searchPath[1024];
    snprintf(searchPath, sizeof(searchPath), "%s%s*", pastaTurma, PATH_SEP);
    WIN32_FIND_DATA fd;
    HANDLE hFind = FindFirstFile(searchPath, &fd);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        printf("  (Nenhuma atividade postada ou pasta da turma nao existe.)\n");
        return;
    }
    do
    {
        if (strcmp(fd.cFileName, ".") != 0 && strcmp(fd.cFileName, "..") != 0)
        {
            printf("  - %s\n", fd.cFileName);
            count++;
        }
    } while (FindNextFile(hFind, &fd) != 0);
    FindClose(hFind);
#else
    DIR *d = opendir(pastaTurma);
    if (d == NULL)
    {
        printf("  (Nenhuma atividade postada ou pasta da turma nao existe.)\n");
        return;
    }
    struct dirent *dir;
    while ((dir = readdir(d)) != NULL)
    {
        if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0)
        {
            printf("  - %s\n", dir->d_name);
            count++;
        }
    }
    closedir(d);
#endif
    if (count == 0)
    {
        printf("  (Nenhum arquivo de atividade postado ainda.)\n");
    }
    printf("----------------------------------------\n");
}

void lancarNotasUI(void)
{
    int idBusca;
    UsuarioCSV aluno;
    char tmp[64];
    while (1)
    {
        if (!listarApenasAlunos())
        {
            return;
        }
        printf("\nDigite o ID do aluno para lancar notas (ou 0 para voltar): ");
        if (scanf("%d", &idBusca) != 1)
        {
            while (getchar() != '\n')
                ;
            idBusca = -1;
        }
        while (getchar() != '\n')
            ;
        if (idBusca == 0)
        {
            break;
        }
        if (idBusca < 0)
        {
            printf("ID invalido.\n");
            continue;
        }
        if (buscarUsuarioPorID(idBusca, &aluno))
        {
            if (STRCASECMP(aluno.nivel, "Aluno") != 0)
            {
                printf("Erro: O ID %d pertence a um %s, nao a um Aluno.\n", idBusca, aluno.nivel);
                continue;
            }
            printf("\n--- Lancando notas para: %s (ID: %d) ---\n", aluno.nome, aluno.id);
            printf("Curso: %s | Turma: %s\n", aluno.curso, aluno.turma);

            printf("NP1 Atual: %.2f | Nova NP1: ", aluno.np1);
            fgets(tmp, sizeof(tmp), stdin);
            aluno.np1 = (float)atof(tmp);

            printf("NP2 Atual: %.2f | Nova NP2: ", aluno.np2);
            fgets(tmp, sizeof(tmp), stdin);
            aluno.np2 = (float)atof(tmp);

            printf("PIM Atual: %.2f | Novo PIM: ", aluno.pim);
            fgets(tmp, sizeof(tmp), stdin);
            aluno.pim = (float)atof(tmp);

            aluno.media = calcularMedia(aluno.np1, aluno.np2, aluno.pim);
            printf("... Nova media calculada: %.2f\n", aluno.media);

            if (alterarUsuarioPorID(idBusca, &aluno))
            {
                printf("Notas atualizadas com sucesso para %s.\n", aluno.nome);
            }
            else
            {
                printf("Erro ao salvar as notas.\n");
            }
        }
        else
        {
            printf("Erro: Usuario com ID %d nao encontrado.\n", idBusca);
        }
    }
}

typedef struct
{
    char nome[64];
    int count;
} TurmaComContagem;

void listarTurmasUnicas(void)
{
    if (!arquivoExiste(ARQ_SISTEMA))
    {
        printf("Arquivo do sistema nao encontrado.\n");
        return;
    }
    TurmaComContagem turmas[MAX_TURMAS];
    int totalTurmasUnicas = 0;
    memset(turmas, 0, sizeof(turmas));
    FILE *f = fopen(ARQ_SISTEMA, "r");
    if (!f)
    {
        printf("Erro ao abrir arquivo.\n");
        return;
    }
    char linha[MAX_LINE];
    fgets(linha, sizeof(linha), f);
    while (fgets(linha, sizeof(linha), f))
    {
        char tmp[MAX_LINE];
        strncpy(tmp, linha, sizeof(tmp) - 1);
        tmp[sizeof(tmp) - 1] = 0;
        trim(tmp);
        UsuarioCSV u;
        memset(&u, 0, sizeof(u));
        if (!parseLinhaUsuario(tmp, &u))
            continue;
        if (STRCASECMP(u.nivel, "Aluno") != 0 || u.turma[0] == '\0')
        {
            continue;
        }
        int encontrada = 0;
        for (int i = 0; i < totalTurmasUnicas; i++)
        {
            if (STRCASECMP(turmas[i].nome, u.turma) == 0)
            {
                turmas[i].count++;
                encontrada = 1;
                break;
            }
        }
        if (!encontrada && totalTurmasUnicas < MAX_TURMAS)
        {
            strncpy(turmas[totalTurmasUnicas].nome, u.turma, sizeof(turmas[totalTurmasUnicas].nome) - 1);
            turmas[totalTurmasUnicas].count = 1;
            totalTurmasUnicas++;
        }
    }
    fclose(f);

    if (totalTurmasUnicas == 0)
    {
        printf("\nNenhum aluno encontrado em turmas.\n");
    }
    else
    {
        printf("\n--- TOTAL DE ALUNOS POR TURMA (%d turmas) ---\n", totalTurmasUnicas);
        printf("%-30s | %s\n", "Turma", "Total de Alunos");
        printf("----------------------------------------------\n");
        int totalAlunosGeral = 0;
        for (int i = 0; i < totalTurmasUnicas; i++)
        {
            printf("%-30s | %d\n", turmas[i].nome, turmas[i].count);
            totalAlunosGeral += turmas[i].count;
        }
        printf("----------------------------------------------\n");
        printf("%-30s | %d\n", "Total Geral de Alunos", totalAlunosGeral);
    }
}

void movimentarAlunoUI(void)
{
    int idBusca;
    UsuarioCSV aluno;
    char novaTurma[128];
    if (!listarApenasAlunos())
    {
        printf("Nao ha alunos para movimentar.\n");
        return;
    }
    printf("\nDigite o ID do aluno para movimentar (ou 0 para cancelar): ");
    if (scanf("%d", &idBusca) != 1)
    {
        while (getchar() != '\n')
            ;
        idBusca = -1;
    }
    while (getchar() != '\n')
        ;
    if (idBusca <= 0)
    {
        printf("Movimentacao cancelada.\n");
        return;
    }
    if (!buscarUsuarioPorID(idBusca, &aluno))
    {
        printf("Erro: Aluno com ID %d nao encontrado.\n", idBusca);
        return;
    }
    if (STRCASECMP(aluno.nivel, "Aluno") != 0)
    {
        printf("Erro: O ID %d pertence a um %s, nao a um Aluno.\n", idBusca, aluno.nivel);
        return;
    }
    printf("\nAluno selecionado: %s\n", aluno.nome);
    printf("Turma ATUAL: %s\n", aluno.turma[0] ? aluno.turma : "(Nenhuma)");
    printf("Digite a NOVA turma: ");
    fgets(novaTurma, sizeof(novaTurma), stdin);
    trim(novaTurma);
    if (novaTurma[0] == '\0')
    {
        printf("Nome da turma nao pode ser vazio. Operacao cancelada.\n");
        return;
    }
    strncpy(aluno.turma, novaTurma, sizeof(aluno.turma) - 1);
    aluno.turma[sizeof(aluno.turma) - 1] = '\0';
    if (alterarUsuarioPorID(idBusca, &aluno))
    {
        printf("Sucesso! Aluno %s (ID %d) movido para a turma '%s'.\n", aluno.nome, idBusca, aluno.turma);
    }
    else
    {
        printf("Falha ao salvar a alteracao da turma.\n");
    }
}

void gerenciarTurmasUI(void)
{
    int opc;
    do
    {
        printf("\n--- GERENCIAR TURMAS ---\n");
        printf("1 - Listar todas as turmas unicas\n");
        printf("2 - Movimentar aluno de turma\n");
        printf("0 - Voltar ao menu anterior\n");
        printf("> ");
        if (scanf("%d", &opc) != 1)
        {
            while (getchar() != '\n')
                ;
            opc = -1;
        }
        while (getchar() != '\n')
            ;
        switch (opc)
        {
        case 1:
            listarTurmasUnicas();
            break;
        case 2:
            movimentarAlunoUI();
            break;
        case 0:
            break;
        default:
            printf("Opcao invalida.\n");
        }
    } while (opc != 0);
}
/* ----------------- MENUS ----------------- */

void mostrarUsuario(const UsuarioCSV *u)
{
    if (!u)
        return;
    printf("\nID: %d\nNome: %s\nEmail: %s\nIdade: %d\nNivel: %s\nCurso: %s\nTurma: %s\nAtividade: %s\nNotas: NP1=%.2f NP2=%.2f PIM=%.2f Media=%.2f\n",
           u->id, u->nome, u->email, u->idade, u->nivel, u->curso, u->turma, u->atividade,
           u->np1, u->np2, u->pim, u->media);
}

void menuAlunoUnificado(const UsuarioCSV *u)
{
    int opc;
    do
    {
        printf("\n=== MENU ALUNO: %s ===\n", u->nome);
        printf("1 - Meus dados\n");
        printf("2 - Ver turma\n");
        printf("3 - Ver notas\n");
        printf("4 - Ver atividades da turma\n");
        printf("0 - Sair\n> ");
        if (scanf("%d", &opc) != 1)
        {
            while (getchar() != '\n')
                ;
            opc = -1;
        }
        while (getchar() != '\n')
            ;
        switch (opc)
        {
        case 1:
            mostrarUsuario(u);
            break;
        case 2:
            printf("Turma: %s\n", u->turma[0] ? u->turma : "(Nao definida)");
            break;
        case 3:
            printf("Notas: NP1=%.2f NP2=%.2f PIM=%.2f Media=%.2f\n", u->np1, u->np2, u->pim, u->media);
            break;
        case 4:
            listarAtividadesTurma(u);
            break;
        case 0:
            break;
        default:
            printf("Opcao invalida.\n");
        }
    } while (opc != 0);
}

void menuProfessorUnificado(const UsuarioCSV *u)
{
    int opc;
    do
    {
        printf("\n=== MENU PROFESSOR: %s ===\n", u->nome);
        printf("1 - Meus dados\n");
        printf("2 - Lancar notas para alunos\n");
        printf("3 - Enviar atividade (PDF)\n");
        printf("0 - Sair\n> ");
        if (scanf("%d", &opc) != 1)
        {
            while (getchar() != '\n')
                ;
            opc = -1;
        }
        while (getchar() != '\n')
            ;
        switch (opc)
        {
        case 1:
            mostrarUsuario(u);
            break;
        case 2:
            lancarNotasUI();
            break;
        case 3:
            enviarAtividadeUI(u);
            break;
        case 0:
            break;
        default:
            printf("Opcao invalida.\n");
        }
    } while (opc != 0);
}

void menuCoordenadorUnificado(const UsuarioCSV *u)
{
    int opc;
    do
    {
        printf("\n=== MENU COORDENADOR: %s ===\n", u->nome);
        printf("1 - Meus dados\n");
        printf("2 - Lancar/Gerenciar notas de alunos\n");
        printf("3 - Gerenciar turmas\n");
        printf("0 - Sair\n> ");
        if (scanf("%d", &opc) != 1)
        {
            while (getchar() != '\n')
                ;
            opc = -1;
        }
        while (getchar() != '\n')
            ;
        switch (opc)
        {
        case 1:
            mostrarUsuario(u);
            break;
        case 2:
            lancarNotasUI();
            break;
        case 3:
            gerenciarTurmasUI();
            break;
        case 0:
            break;
        default:
            printf("Opcao invalida.\n");
        }
    } while (opc != 0);
}

/* Funcoes administrativas: menu com CRUD integrado */
/* Funcoes administrativas: menu com CRUD integrado */
/* Funcoes administrativas: menu com CRUD integrado */
/* Funcoes administrativas: menu com CRUD integrado */
void gerenciarUsuariosUI(void)
{
    int opc = -1;
    char buffer[128]; // Buffer de entrada padronizado

    while (1)
    {
        printf("\n--- GERENCIAR USUARIOS ---\n");
        printf("1 - Listar todos (Padrao)\n");
        printf("2 - Adicionar\n");
        printf("3 - Alterar por ID\n");
        printf("4 - Excluir por ID\n");
        printf("5 - Visualizar Senhas (Requer Autorizacao)\n");
        printf("0 - Voltar\n> ");

        if (!fgets(buffer, sizeof(buffer), stdin))
        {
            opc = 0;
        }
        opc = atoi(buffer);

        if (opc == 1)
        {
            listarTodosUsuarios();
        }
        else if (opc == 2)
        {
            /* --- Logica de Adicionar Usuario (Padronizada) --- */
            UsuarioCSV u;
            memset(&u, 0, sizeof(u));
            int opcNivel = 0;
            int opcAtividade = 0;

            printf("Nome: ");
            fgets(u.nome, sizeof(u.nome), stdin);
            u.nome[strcspn(u.nome, "\n")] = 0;
            trim(u.nome);
            stringToTitle(u.nome);

            printf("Email: ");
            fgets(u.email, sizeof(u.email), stdin);
            u.email[strcspn(u.email, "\n")] = 0;
            trim(u.email);
            stringToLower(u.email);

            printf("Senha: ");
            lerSenhaOculta(u.senha, sizeof(u.senha));
            trim(u.senha);

            if (u.senha[0] == '\0')
            {
                printf("Senha nao pode ser vazia. Operacao cancelada.\n");
                continue;
            }

            /* Menu de seleção de Nível */
            while (opcNivel < 1 || opcNivel > 4)
            {
                printf("Nivel:\n");
                printf(" 1 - Aluno\n");
                printf(" 2 - Professor\n");
                printf(" 3 - Coordenador\n");
                printf(" 4 - Administrador\n");
                printf("Digite a opcao (1-4): ");

                fgets(buffer, sizeof(buffer), stdin);
                opcNivel = atoi(buffer);

                if (opcNivel < 1 || opcNivel > 4)
                {
                    printf("Opcao invalida. Tente novamente.\n\n");
                }
            }

            switch (opcNivel)
            {
            case 1:
                strncpy(u.nivel, "Aluno", sizeof(u.nivel) - 1);
                break;
            case 2:
                strncpy(u.nivel, "Professor", sizeof(u.nivel) - 1);
                break;
            case 3:
                strncpy(u.nivel, "Coordenador", sizeof(u.nivel) - 1);
                break;
            case 4:
                strncpy(u.nivel, "Administrador", sizeof(u.nivel) - 1);
                break;
            }

            printf("Curso: ");
            fgets(u.curso, sizeof(u.curso), stdin);
            u.curso[strcspn(u.curso, "\n")] = 0;
            trim(u.curso);
            stringToUpper(u.curso);

            printf("Turma: ");
            fgets(u.turma, sizeof(u.turma), stdin);
            u.turma[strcspn(u.turma, "\n")] = 0;
            trim(u.turma);
            stringToUpper(u.turma);

            printf("Idade: ");
            fgets(buffer, sizeof(buffer), stdin);
            u.idade = atoi(buffer);
            printf("NP1: ");
            fgets(buffer, sizeof(buffer), stdin);
            u.np1 = (float)atof(buffer);
            printf("NP2: ");
            fgets(buffer, sizeof(buffer), stdin);
            u.np2 = (float)atof(buffer);
            printf("PIM: ");
            fgets(buffer, sizeof(buffer), stdin);
            u.pim = (float)atof(buffer);
            printf("Media: ");
            fgets(buffer, sizeof(buffer), stdin);
            u.media = (float)atof(buffer);

            /* Menu de seleção de Atividade */
            while (opcAtividade < 1 || opcAtividade > 2)
            {
                printf("Status da Atividade:\n");
                printf(" 1 - Ativo\n");
                printf(" 2 - Inativo\n");
                printf("Digite a opcao (1-2): ");

                fgets(buffer, sizeof(buffer), stdin);
                opcAtividade = atoi(buffer);

                if (opcAtividade < 1 || opcAtividade > 2)
                {
                    printf("Opcao invalida. Tente novamente.\n\n");
                }
            }

            if (opcAtividade == 1)
            {
                strncpy(u.atividade, "Ativo", sizeof(u.atividade) - 1);
            }
            else
            {
                strncpy(u.atividade, "Inativo", sizeof(u.atividade) - 1);
            }

            if (!adicionarUsuario(&u))
                printf("Falha ao adicionar usuario.\n");
        }
        else if (opc == 3)
        {
            /* --- Logica de Alterar Usuario (Padronizada) --- */
            int id;
            printf("ID a alterar: ");

            if (!fgets(buffer, sizeof(buffer), stdin))
            {
                continue;
            }
            id = atoi(buffer);
            if (id <= 0)
            {
                printf("ID invalido.\n");
                continue;
            }

            UsuarioCSV u;
            memset(&u, 0, sizeof(u));
            if (!buscarUsuarioPorID(id, &u))
            {
                printf("Usuario com ID %d nao encontrado.\n", id);
                continue;
            }

            printf("Alterando dados para: %s (ID %d)\n", u.nome, u.id);
            printf("AVISO: Pressione ENTER para manter o valor atual.\n");

            char senha_digitada[128];
            int opcNivel = 0;
            int opcAtividade = 0;

            printf("Nome (%s): ", u.nome);
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            trim(buffer);
            if (buffer[0] != '\0')
            {
                stringToTitle(buffer);
                strncpy(u.nome, buffer, sizeof(u.nome) - 1);
            }

            printf("Email (%s): ", u.email);
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            trim(buffer);
            if (buffer[0] != '\0')
            {
                stringToLower(buffer);
                strncpy(u.email, buffer, sizeof(u.email) - 1);
            }

            printf("Senha (DEIXE EM BRANCO para nao alterar): ");
            lerSenhaOculta(senha_digitada, sizeof(senha_digitada));
            trim(senha_digitada);

            if (senha_digitada[0] != '\0')
            {
                strncpy(u.senha, senha_digitada, sizeof(u.senha) - 1);
                u.senha[sizeof(u.senha) - 1] = '\0';
                printf("Senha alterada com sucesso.\n");
            }
            else
            {
                memset(u.senha, 0, sizeof(u.senha));
            }

            /* Menu de Nivel para Alteracao */
            while (1)
            {
                printf("Nivel (%s):\n", u.nivel);
                printf(" 1 - Aluno\n 2 - Professor\n 3 - Coordenador\n 4 - Administrador\n");
                printf("Digite a opcao (1-4) ou [ENTER] para manter: ");

                fgets(buffer, sizeof(buffer), stdin);
                trim(buffer);

                if (buffer[0] == '\0')
                {
                    break;
                }

                opcNivel = atoi(buffer);
                if (opcNivel >= 1 && opcNivel <= 4)
                {
                    switch (opcNivel)
                    {
                    case 1:
                        strncpy(u.nivel, "Aluno", sizeof(u.nivel) - 1);
                        break;
                    case 2:
                        strncpy(u.nivel, "Professor", sizeof(u.nivel) - 1);
                        break;
                    case 3:
                        strncpy(u.nivel, "Coordenador", sizeof(u.nivel) - 1);
                        break;
                    case 4:
                        strncpy(u.nivel, "Administrador", sizeof(u.nivel) - 1);
                        break;
                    }
                    break;
                }
                else
                {
                    printf("Opcao invalida. Tente novamente.\n\n");
                }
            }

            printf("Curso (%s): ", u.curso);
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            trim(buffer);
            if (buffer[0] != '\0')
            {
                stringToUpper(buffer);
                strncpy(u.curso, buffer, sizeof(u.curso) - 1);
            }

            printf("Turma (%s): ", u.turma);
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            trim(buffer);
            if (buffer[0] != '\0')
            {
                stringToUpper(buffer);
                strncpy(u.turma, buffer, sizeof(u.turma) - 1);
            }

            printf("Idade (%d): ", u.idade);
            fgets(buffer, sizeof(buffer), stdin);
            trim(buffer);
            if (buffer[0] != '\0')
                u.idade = atoi(buffer);

            printf("NP1 (%.2f): ", u.np1);
            fgets(buffer, sizeof(buffer), stdin);
            trim(buffer);
            if (buffer[0] != '\0')
                u.np1 = (float)atof(buffer);

            printf("NP2 (%.2f): ", u.np2);
            fgets(buffer, sizeof(buffer), stdin);
            trim(buffer);
            if (buffer[0] != '\0')
                u.np2 = (float)atof(buffer);

            printf("PIM (%.2f): ", u.pim);
            fgets(buffer, sizeof(buffer), stdin);
            trim(buffer);
            if (buffer[0] != '\0')
                u.pim = (float)atof(buffer);

            u.media = calcularMedia(u.np1, u.np2, u.pim);
            printf("... Media (re)calculada: %.2f\n", u.media);

            /* Menu de Atividade para Alteracao */
            while (1)
            {
                printf("Atividade (%s):\n", u.atividade);
                printf(" 1 - Ativo\n 2 - Inativo\n");
                printf("Digite a opcao (1-2) ou [ENTER] para manter: ");

                fgets(buffer, sizeof(buffer), stdin);
                trim(buffer);

                if (buffer[0] == '\0')
                {
                    break;
                }

                opcAtividade = atoi(buffer);
                if (opcAtividade == 1 || opcAtividade == 2)
                {
                    strncpy(u.atividade, (opcAtividade == 1 ? "Ativo" : "Inativo"), sizeof(u.atividade) - 1);
                    break;
                }
                else
                {
                    printf("Opcao invalida. Tente novamente.\n\n");
                }
            }

            if (!alterarUsuarioPorID(id, &u))
                printf("Falha ao alterar usuario.\n");
        }
        else if (opc == 4)
        {
            int id;
            printf("ID a excluir: ");

            if (!fgets(buffer, sizeof(buffer), stdin))
            {
                continue;
            }
            id = atoi(buffer);
            if (id <= 0)
            {
                printf("ID invalido.\n");
                continue;
            }

            if (!excluirUsuarioPorID(id))
                printf("Falha ao excluir usuario.\n");
        }
        else if (opc == 5)
        {
            /* --- LOGICA DE AUTENTICACAO DE SENHA MESTRE --- */
            char passBuffer[128];
            printf("\n!!! ACAO RESTRITA !!!\n");
            printf("Digite a senha mestre para visualizar os dados: ");
            lerSenhaOculta(passBuffer, sizeof(passBuffer));

            if (strcmp(passBuffer, MASTER_PASSWORD) == 0)
            {
                printf("\nAutenticacao bem-sucedida. Exibindo dados completos...\n");
                listarTodosUsuariosComSenha();
            }
            else
            {
                printf("\n>>> SENHA MESTRE INCORRETA! ACESSO NEGADO! <<<\n");
            }
        }
        else if (opc == 0)
        {
            break;
        }
        else
        {
            printf("Opcao invalida.\n");
        }
    }
}

void menuAdministradorUnificado(const UsuarioCSV *u)
{
    int opc;
    do
    {
        printf("\n=== MENU ADMINISTRADOR: %s ===\n", u->nome);
        printf("1 - Meus dados\n");
        printf("2 - Gerenciar usuarios (CRUD)\n");
        printf("3 - Lancar notas de alunos\n");
        printf("4 - Gerenciar turmas\n");
        printf("5 - Criar backup manual\n");
        printf("0 - Sair\n> ");
        if (scanf("%d", &opc) != 1)
        {
            while (getchar() != '\n')
                ;
            opc = -1;
        }
        while (getchar() != '\n')
            ;
        switch (opc)
        {
        case 1:
            mostrarUsuario(u);
            break;
        case 2:
            gerenciarUsuariosUI();
            break;
        case 3:
            lancarNotasUI();
            break;
        case 4:
            gerenciarTurmasUI();
            break;
        case 5:
            backupSistema();
            break;
        case 0:
            break;
        default:
            printf("Opcao invalida.\n");
        }
    } while (opc != 0);
}

/* ----------------- EXECUTAR SISTEMA (menu principal) ----------------- */

void executarSistema(void)
{
    /* initSistema() nao contem mais a inicializacao da libsodium */

    initSistema();
    criarArquivoSistemaSeNaoExiste();

    UsuarioCSV logado;
    char email[256], senha[128];
    int tentativas = 0;

    printf("\n==== SISTEMA ACADEMICO UNIFICADO (Console C) ====\n");

    while (tentativas < 3)
    {
        printf("Email: ");
        if (!fgets(email, sizeof(email), stdin))
            return;
        email[strcspn(email, "\n")] = 0;
        trim(email);

        printf("Senha: ");
        lerSenhaOculta(senha, sizeof(senha));

        /* Agora chama a funcao de verificacao de texto puro */
        if (verificarLoginUnico(email, senha, &logado))
            break;

        printf("Email ou senha incorretos (%d/3)\n", ++tentativas);
    }
    if (tentativas >= 3)
    {
        printf("Numero maximo de tentativas atingido.\n");
        return;
    }

    printf("\nLogin bem-sucedido! Bem-vindo, %s.\n", logado.nome);

    if (STRCASECMP(logado.nivel, "Administrador") == 0)
        menuAdministradorUnificado(&logado);
    else if (STRCASECMP(logado.nivel, "Coordenador") == 0)
        menuCoordenadorUnificado(&logado);
    else if (STRCASECMP(logado.nivel, "Professor") == 0)
        menuProfessorUnificado(&logado);
    else
        menuAlunoUnificado(&logado);
}

#endif // SISTEMAACADEMICO_H_INCLUDED

// atualizado em 14/11 as 00:12