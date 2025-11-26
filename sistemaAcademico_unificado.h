#ifndef SISTEMAACADEMICO_H_INCLUDED
#define SISTEMAACADEMICO_H_INCLUDED
#define MASTER_PASSWORD "admin"
/*
 * sistemaacademico.h - Versao unificada e corrigida.
 *
 * Contem:
 * - Estrutura UsuarioCSV e funcoes utilitarias.
 * - Funcoes de manipulacao de arquivo (CSV plano).
 * - Funcoes CRUD (Create, Read, Update, Delete) de usuários.
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

/* --- ALTERACAO PRINCIPAL AQUI --- */
#define DIR_OUTPUT "output"
#define ARQ_SISTEMA "sistemaAcademico.csv" 
/* -------------------------------- */

#define DIR_BACKUPS "backups"
#define MAX_LINE 2048
#define MAX_TURMAS 256
#define DIR_ATIVIDADES "atividades"
#define MAX_TURMAS_MENU 50

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
void lancarNotasUI(const UsuarioCSV *autor);
void listarTurmasUnicas(void);
void movimentarAlunoUI(void);
void gerenciarTurmasUI(void);
int copiarArquivo(const char *origem, const char *destino);
void enviarAtividadeUI(const UsuarioCSV *u);
void listarAtividadesTurma(const UsuarioCSV *u);
const char* obterNomeArquivoDoPath(const char *path);
int obterTurmasDoCurso(const char *cursoAlvo, char listaTurmas[MAX_TURMAS_MENU][64]);

/* ----------------- DEFINICOES DE FUNCOES ----------------- */

/* ============================================================================== */
/* ==================== INICIALIZAÇÃO DO SISTEMA (PT-BR) ======================== */
/* ============================================================================== */

void initSistema(void)
{
    /* 1. Força o Console do Windows para UTF-8 */
    /* Isso faz ele entender os caracteres como 'ç' e 'ã' vindos do VSCode */
#ifdef _WIN32
    SetConsoleOutputCP(65001); // CP_UTF8
    SetConsoleCP(65001);       // CP_UTF8
#endif

    /* 2. Mantém o padrão numérico como "C" (Ponto) */
    /* Isso garante que cálculos e leitura de arquivos não quebrem */
    setlocale(LC_NUMERIC, "C");
    
    /* OBS: Removemos o setlocale(LC_ALL, "Portuguese") porque ele
       causa conflito com o UTF-8 no Windows Console */
}

// Função para remover espaços em branco no início e no final de uma string
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

void stringToUpper(char *s)
{
    if (!s)
        return;
    for (; *s; ++s)
        *s = (char)toupper((unsigned char)*s);
}
// Função para converter uma string para minúsculas
void stringToLower(char *s)
{
    if (!s)
        return;
    for (; *s; ++s)
        *s = (char)tolower((unsigned char)*s);
}
// Função para converter uma string para título (primeira letra de cada palavra em maiúscula)
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
// Função para validar um endereço de e-mail
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
// Função para ler uma senha oculta do usuário
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
    if (fgets(senha, (int)maxLen, stdin))
    {
        senha[strcspn(senha, "\n")] = '\0';
    }
    else
        senha[0] = '\0';
#endif
}
// Função para verificar se um arquivo existe
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
// Função para garantir que uma pasta exista, criando-a se necessário
void garantirPasta(const char *pasta)
{
    if (!pasta)
        return;
    if (!arquivoExiste(pasta))
        MKDIR(pasta);
}
// Função para obter uma string com a data e hora atual no formato "YYYYMMDD_HHMMSS"
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
    /* Nota: Se o ARQ_SISTEMA ja inclui "output/", o backup vai ler de la */
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
// Função para copiar um arquivo de origem para um destino
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


/* ============================================================================== */
/* ==================== MODULO DE ENVIO: LISTAGEM DE TURMAS E CASCATA =========== */
/* ============================================================================== */

/* 1. Auxiliar: Isola o nome do arquivo do caminho completo */
const char* obterNomeArquivoDoPath(const char *path)
{
    const char *nome = strrchr(path, '\\');
    if (!nome) nome = strrchr(path, '/');
    if (nome) return nome + 1;
    return path;
}

/* 2. Auxiliar: Busca turmas UNICAS no CSV baseadas no Curso do Professor */
#define MAX_TURMAS_MENU 50

int obterTurmasDoCurso(const char *cursoAlvo, char listaTurmas[MAX_TURMAS_MENU][64])
{
    /* Verifica se o arquivo existe no caminho correto (output/sistemaAcademico.csv) */
    if (!arquivoExiste(ARQ_SISTEMA)) return 0;
    
    FILE *f = fopen(ARQ_SISTEMA, "r");
    if (!f) return 0;

    char linha[MAX_LINE];
    int qtd = 0;
    
    /* Pula a linha de cabeçalho */
    fgets(linha, sizeof(linha), f); 

    while (fgets(linha, sizeof(linha), f) && qtd < MAX_TURMAS_MENU)
    {
        char tmp[MAX_LINE];
        strncpy(tmp, linha, sizeof(tmp)-1);
        tmp[sizeof(tmp)-1] = 0;
        trim(tmp);

        UsuarioCSV u;
        memset(&u, 0, sizeof(u));
        
        /* Se a linha estiver vazia ou inválida, pula */
        if (!parseLinhaUsuario(tmp, &u)) continue;

        /* LOGICA DE FILTRO:
           - Tem que ser ALUNO
           - Tem que ser do MESMO CURSO do professor
           - A turma não pode estar vazia
        */
        if (STRCASECMP(u.nivel, "Aluno") == 0 && STRCASECMP(u.curso, cursoAlvo) == 0)
        {
            if (u.turma[0] != '\0')
            {
                /* Verifica se já adicionamos essa turma na lista para não repetir */
                int jaExiste = 0;
                for (int i = 0; i < qtd; i++) {
                    if (STRCASECMP(listaTurmas[i], u.turma) == 0) {
                        jaExiste = 1; 
                        break;
                    }
                }
                
                /* Se é uma turma nova, adiciona na lista */
                if (!jaExiste) {
                    strncpy(listaTurmas[qtd], u.turma, 63);
                    listaTurmas[qtd][63] = '\0';
                    qtd++;
                }
            }
        }
    }
    fclose(f);
    return qtd;
}

/* ============================================================================== */
/* FUNCAO AUXILIAR: Listar pastas (disciplinas) dentro da turma */
/* ============================================================================== */
int obterDisciplinasDaPasta(const char *pathTurma, char listaDisciplinas[50][128])
{
    int count = 0;

#ifdef _WIN32
    char busca[1024];
    snprintf(busca, sizeof(busca), "%s%s*", pathTurma, PATH_SEP);
    
    WIN32_FIND_DATA fd;
    HANDLE h = FindFirstFile(busca, &fd);

    if (h == INVALID_HANDLE_VALUE) return 0;

    do {
        if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            strcmp(fd.cFileName, ".") != 0 &&
            strcmp(fd.cFileName, "..") != 0) 
        {
            strncpy(listaDisciplinas[count], fd.cFileName, 127);
            listaDisciplinas[count][127] = '\0';
            count++;
            if (count >= 50) break;
        }
    } while (FindNextFile(h, &fd));
    FindClose(h);

#else
    DIR *d = opendir(pathTurma);
    if (!d) return 0;

    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        if (dir->d_type == DT_DIR && 
            strcmp(dir->d_name, ".") != 0 && 
            strcmp(dir->d_name, "..") != 0) 
        {
            strncpy(listaDisciplinas[count], dir->d_name, 127);
            listaDisciplinas[count][127] = '\0';
            count++;
            if (count >= 50) break;
        }
    }
    closedir(d);
#endif

    return count;
}

/* 3. FUNCAO PRINCIPAL: Menu de Seleção + Envio em Cascata */

void enviarAtividadeUI(const UsuarioCSV *u)
{
    /* --- VALIDAÇÃO INICIAL --- */
    if (u->curso[0] == '\0') {
        printf("\n[ERRO] Seu cadastro nao possui um Curso definido.\n"); 
        return;
    }

    printf("\n========================================\n");
    printf("   ENVIAR MATERIAL DE ESTUDO\n");
    printf("========================================\n");
    printf("Professor: %s\n", u->nome);
    printf("Curso Base: %s\n", u->curso);
    printf("----------------------------------------\n");

    /* --- PASSO 1: LISTAR TURMAS DISPONIVEIS --- */
    char turmas[MAX_TURMAS_MENU][64];
    int qtdTurmas = obterTurmasDoCurso(u->curso, turmas);

    if (qtdTurmas == 0) {
        printf("\n[AVISO] Nenhuma turma com alunos encontrada para o curso '%s'.\n", u->curso);
        return;
    }

    printf("Selecione a turma de destino:\n");
    for (int i = 0; i < qtdTurmas; i++) {
        printf("  %d - %s\n", i + 1, turmas[i]);
    }
    printf("  0 - Cancelar\n");
    printf("----------------------------------------\n");
    printf("> ");
    
    char buffer[64];
    if (!fgets(buffer, sizeof(buffer), stdin)) return;
    int escTurma = atoi(buffer);

    if (escTurma <= 0 || escTurma > qtdTurmas) {
        printf("Operacao cancelada.\n");
        return;
    }
    
    char turmaSel[64];
    strcpy(turmaSel, turmas[escTurma - 1]);
    printf(">> Turma selecionada: %s\n", turmaSel);

    /* --- PASSO 2: SELECIONAR DISCIPLINA (ATUALIZADO) --- */
    
    /* Monta o caminho da turma para buscar as disciplinas existentes */
    char caminhoTurma[1024];
    snprintf(caminhoTurma, sizeof(caminhoTurma), "%s%s%s%s%s", 
             DIR_ATIVIDADES, PATH_SEP, u->curso, PATH_SEP, turmaSel);
    
    /* Garante que a pasta da turma exista para podermos listar */
    garantirPasta(DIR_ATIVIDADES);
    char caminhoCurso[1024];
    snprintf(caminhoCurso, sizeof(caminhoCurso), "%s%s%s", DIR_ATIVIDADES, PATH_SEP, u->curso);
    garantirPasta(caminhoCurso);
    garantirPasta(caminhoTurma);

    char listaDisciplinas[50][128];
    int qtdDisciplinas = obterDisciplinasDaPasta(caminhoTurma, listaDisciplinas);
    char disciplinaFinal[128];
    disciplinaFinal[0] = '\0';

    printf("\nSelecione a DISCIPLINA:\n");
    if (qtdDisciplinas > 0) {
        for (int i = 0; i < qtdDisciplinas; i++) {
            printf("  %d - %s\n", i + 1, listaDisciplinas[i]);
        }
    } else {
        printf("  (Nenhuma disciplina criada ainda)\n");
    }
    /* Opção para criar nova sempre disponivel */
    printf("  %d - [CRIAR NOVA DISCIPLINA]\n", qtdDisciplinas + 1);
    printf("  0 - Cancelar\n");
    printf("> ");

    if (!fgets(buffer, sizeof(buffer), stdin)) return;
    int escDisc = atoi(buffer);

    if (escDisc == 0) return;

    if (escDisc > 0 && escDisc <= qtdDisciplinas) {
        /* Selecionou uma existente */
        strcpy(disciplinaFinal, listaDisciplinas[escDisc - 1]);
    } 
    else if (escDisc == qtdDisciplinas + 1) {
        /* Selecionou criar nova */
        printf("\nDigite o nome da NOVA Disciplina: ");
        if (!fgets(disciplinaFinal, sizeof(disciplinaFinal), stdin)) return;
        trim(disciplinaFinal);
        stringToTitle(disciplinaFinal);
    } 
    else {
        printf("Opção inválida.\n");
        return;
    }

    if (disciplinaFinal[0] == '\0') {
        printf("Nome de disciplina invalido.\n"); return;
    }
    
    printf(">> Disciplina: %s\n", disciplinaFinal);

    /* --- PASSO 3: CAMINHO DO ARQUIVO --- */
    printf("\nDigite o caminho do arquivo (ou arraste o arquivo aqui): ");
    char origem[512];
    if (!fgets(origem, sizeof(origem), stdin)) return;
    trim(origem);
    
    if (origem[0] == '"') {
        memmove(origem, origem + 1, strlen(origem));
        if (origem[strlen(origem)-1] == '"') origem[strlen(origem)-1] = 0;
    }

    if (!arquivoExiste(origem)) {
        printf("\n[ERRO] Arquivo nao encontrado: %s\n", origem); 
        return;
    }

    /* --- PASSO 4: CRIAR ESTRUTURA E COPIAR --- */
    /* Estrutura: atividades/Curso/Turma/Disciplina/MATERIAL_PARA_ESTUDO */

    char caminhoAtual[1024];
    // Caminho ate a disciplina (Reutilizando a logica)
    snprintf(caminhoAtual, sizeof(caminhoAtual), "%s%s%s", caminhoTurma, PATH_SEP, disciplinaFinal);
    garantirPasta(caminhoAtual);

    /* Garante subpastas padrao */
    char caminhoEntregas[1024];
    snprintf(caminhoEntregas, sizeof(caminhoEntregas), "%s%sENTREGAS", caminhoAtual, PATH_SEP);
    garantirPasta(caminhoEntregas);

    char caminhoMaterial[1024];
    snprintf(caminhoMaterial, sizeof(caminhoMaterial), "%s%sMATERIAL_PARA_ESTUDO", caminhoAtual, PATH_SEP);
    garantirPasta(caminhoMaterial);

    /* Copiar */
    char destinoFinal[1024];
    snprintf(destinoFinal, sizeof(destinoFinal), "%s%s%s", caminhoMaterial, PATH_SEP, obterNomeArquivoDoPath(origem));

    printf("\n------------------------------------------------\n");
    printf("Copiando arquivo...\n");
    printf("Para: .../%s/%s/MATERIAL_PARA_ESTUDO\n", turmaSel, disciplinaFinal);
    printf("------------------------------------------------\n");

    if (copiarArquivo(origem, destinoFinal)) {
        printf("\n[SUCESSO] Material enviado!\n");
    } else {
        printf("\n[ERRO] Falha ao copiar o arquivo.\n");
    }
}

/* ----------------- ARQUIVO INICIAL (ALTERADO PARA CRIAR PASTA OUTPUT) ----------------- */

void criarArquivoSistemaSeNaoExiste(void)
{
    /* 1. GARANTE QUE A PASTA OUTPUT EXISTA ANTES DE CRIAR O ARQUIVO */
    garantirPasta(DIR_OUTPUT); 

    if (arquivoExiste(ARQ_SISTEMA))
        return;
        
    FILE *f = fopen(ARQ_SISTEMA, "w");
    if (!f)
    {
        printf("Erro ao criar arquivo do sistema em '%s'!\n", ARQ_SISTEMA);
        printf("Verifique permissoes de pasta.\n");
        return;
    }
    /* Cabecalho */
    fprintf(f, "[USUARIOS]\n");
    fprintf(f, "id;nome;email;senha;nivel;curso;turma;idade;np1;np2;pim;media;atividade\n");

    /* Usuario admin padrao (PADRONIZADO MAIUSCULO, exceto email/senha) */
    fprintf(f, "1;ADMINISTRADOR;admin@admin.com;admin;ADMINISTRADOR;SISTEMA;GERAL;30;0.00;0.00;0.00;0.00;ATIVO\n");

    fclose(f);
    printf("Arquivo do sistema criado em: %s\n", ARQ_SISTEMA);
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

    /* A senha e texto puro, mas nao pode ser vazia */
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
    
    // ALTERADO: Padrão "ATIVO"
    if (!u.atividade[0])
        strncpy(u.atividade, "ATIVO", sizeof(u.atividade) - 1);
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
 * LISTAGEM RESTRITA: Mostra TODOS os usuários com senhas EM TEXTO PURO.
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

    printf("\n====================================================== LISTAGEM DE USUÁRIOS (COM SENHA) ======================================================\n");
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
            return 1;
        }
    }
    fclose(f);
    return 0;
}

float calcularMedia(float np1, float np2, float pim)
{
    return (float)((np1 * 0.4f) + (np2 * 0.4f) + (pim * 0.2f));
}

/* ============================================================================== */
/* ==================== NOVA LISTAGEM DE ATIVIDADES (CASCATA) =================== */
/* ============================================================================== */

void listarAtividadesTurma(const UsuarioCSV *u)
{
    /* 1. Validações Básicas */
    if (u->turma[0] == '\0' || STRCASECMP(u->turma, "Geral") == 0) {
        printf("\n[AVISO] Voce nao esta matriculado em uma turma especifica.\n");
        return;
    }
    if (u->curso[0] == '\0') {
        printf("\n[ERRO] Seu cadastro nao possui Curso definido.\n");
        return;
    }

    /* 2. Monta o caminho raiz da Turma: atividades/CURSO/TURMA */
    char caminhoTurma[1024];
    snprintf(caminhoTurma, sizeof(caminhoTurma), "%s%s%s%s%s", 
             DIR_ATIVIDADES, PATH_SEP, u->curso, PATH_SEP, u->turma);

    printf("\n======================================================\n");
    printf("   MATERIAIS DE ESTUDO DISPONIVEIS: %s\n", u->turma);
    printf("======================================================\n");

    int encontrouAlgumaCoisa = 0;

#ifdef _WIN32
    /* --- LOGICA WINDOWS --- */
    char buscaDisciplinas[1024];
    snprintf(buscaDisciplinas, sizeof(buscaDisciplinas), "%s%s*", caminhoTurma, PATH_SEP);
    
    WIN32_FIND_DATA fdDisc;
    HANDLE hDisc = FindFirstFile(buscaDisciplinas, &fdDisc);

    // Se o FindFirstFile falhar, a pasta não existe ou não pode ser lida.
    if (hDisc == INVALID_HANDLE_VALUE) { 
        printf("\n[AVISO] Nenhuma pasta encontrada para a turma: %s (Verifique o Curso e Turma no CSV)\n", u->turma);
        return;
    }
    
    do {
        if ((fdDisc.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            strcmp(fdDisc.cFileName, ".") != 0 && 
            strcmp(fdDisc.cFileName, "..") != 0) 
        {
            /* Achamos uma Disciplina */
            char nomeDisciplina[256];
            strcpy(nomeDisciplina, fdDisc.cFileName);

            /* Passo B: Verificar se existe MATERIAL_PARA_ESTUDO nesta disciplina */
            char buscaMateriais[1024];
            snprintf(buscaMateriais, sizeof(buscaMateriais), "%s%s%s%sMATERIAL_PARA_ESTUDO%s*", 
                     caminhoTurma, PATH_SEP, nomeDisciplina, PATH_SEP, PATH_SEP);

            WIN32_FIND_DATA fdMat;
            HANDLE hMat = FindFirstFile(buscaMateriais, &fdMat);
            
            int imprimiuHeader = 0;

            if (hMat != INVALID_HANDLE_VALUE) {
                do {
                    if (!(fdMat.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                        if (!imprimiuHeader) {
                            printf("\n>> DISCIPLINA: %s\n", nomeDisciplina);
                            imprimiuHeader = 1;
                        }
                        printf("   [DOWNLOAD] %s\n", fdMat.cFileName);
                        encontrouAlgumaCoisa = 1;
                    }
                } while (FindNextFile(hMat, &fdMat) != 0);
                FindClose(hMat);
            }
        }
    } while (FindNextFile(hDisc, &fdDisc) != 0);
    FindClose(hDisc);

#else
    /* --- LOGICA LINUX/MAC --- */
    DIR *dDisc = opendir(caminhoTurma);
    if (dDisc == NULL) {
         printf("\n[AVISO] Nenhuma pasta encontrada para a turma: %s (Verifique o Curso e Turma no CSV)\n", u->turma);
         return;
    }

    struct dirent *dirDisc;
    while ((dirDisc = readdir(dDisc)) != NULL) {
        if (strcmp(dirDisc->d_name, ".") == 0 || strcmp(dirDisc->d_name, "..") == 0) continue;
        
        char nomeDisciplina[256];
        strcpy(nomeDisciplina, dirDisc->d_name);

        char caminhoMaterial[1024];
        snprintf(caminhoMaterial, sizeof(caminhoMaterial), "%s%s%s%sMATERIAL_PARA_ESTUDO", 
                 caminhoTurma, PATH_SEP, nomeDisciplina, PATH_SEP);
        
        DIR *dMat = opendir(caminhoMaterial);
        if (dMat) {
            struct dirent *dirMat;
            int imprimiuHeader = 0;
            while((dirMat = readdir(dMat)) != NULL) {
                if (strcmp(dirMat->d_name, ".") != 0 && strcmp(dirMat->d_name, "..") != 0) {
                     if (!imprimiuHeader) {
                        printf("\n>> DISCIPLINA: %s\n", nomeDisciplina);
                        imprimiuHeader = 1;
                    }
                    printf("   [ARQUIVO] %s\n", dirMat->d_name);
                    encontrouAlgumaCoisa = 1;
                }
            }
            closedir(dMat);
        }
    }
    closedir(dDisc);
#endif

    if (!encontrouAlgumaCoisa) {
        printf("\n(Nenhum material postado para sua turma ainda)\n");
    }
    printf("\n======================================================\n");
}

/* ============================================================================== */
/* ==================== LANÇAR NOTAS (SELEÇÃO POR ID) =========================== */
/* ============================================================================== */

void lancarNotasUI(const UsuarioCSV *autor)
{
    /* Lógica Diferenciada:
       - PROFESSOR: Vê lista de turmas -> Tabela de Alunos -> Digita ID -> Lança.
       - ADM/COORD: Pode digitar qualquer ID globalmente.
    */

    int isProfessor = (STRCASECMP(autor->nivel, "Professor") == 0);

    /* --- FLUXO PARA PROFESSOR --- */
    if (isProfessor)
    {
        if (autor->curso[0] == '\0') {
            printf("\n[ERRO] Voce nao possui um curso vinculado ao seu perfil.\n");
            return;
        }

        /* 1. Selecionar Turma */
        char turmas[MAX_TURMAS_MENU][64];
        int qtdTurmas = obterTurmasDoCurso(autor->curso, turmas);

        if (qtdTurmas == 0) {
            printf("\n[AVISO] Nenhuma turma encontrada para o curso %s.\n", autor->curso);
            return;
        }

        printf("\n--- SELECIONE A TURMA PARA LANCAR NOTAS ---\n");
        for (int i = 0; i < qtdTurmas; i++) {
            printf(" %d - %s\n", i + 1, turmas[i]);
        }
        printf(" 0 - Cancelar\n");
        printf("> ");

        char buf[64];
        if (!fgets(buf, sizeof(buf), stdin)) return;
        int escTurma = atoi(buf);

        if (escTurma <= 0 || escTurma > qtdTurmas) return;
        
        char turmaAlvo[64];
        strcpy(turmaAlvo, turmas[escTurma - 1]);

        /* 2. Carregar Alunos (com notas) para memória temporária */
        struct { 
            int id; 
            char nome[64]; 
            float np1, np2, pim, media;
        } alunosEncontrados[100];
        
        int qtdAlunos = 0;

        FILE *f = fopen(ARQ_SISTEMA, "r");
        if (!f) return;

        char linha[MAX_LINE];
        if (!fgets(linha, sizeof(linha), f)) { 
            fclose(f); return;
        }

        while (fgets(linha, sizeof(linha), f) && qtdAlunos < 100)
        {
            char cp[MAX_LINE];
            strncpy(cp, linha, sizeof(cp)-1); cp[sizeof(cp)-1]=0;
            trim(cp);
            UsuarioCSV uTemp;
            if (!parseLinhaUsuario(cp, &uTemp)) continue;

            // Filtro: Aluno + Mesmo Curso + Mesma Turma
            if (STRCASECMP(uTemp.nivel, "Aluno") == 0 &&
                STRCASECMP(uTemp.curso, autor->curso) == 0 &&
                STRCASECMP(uTemp.turma, turmaAlvo) == 0)
            {
                alunosEncontrados[qtdAlunos].id = uTemp.id;
                strncpy(alunosEncontrados[qtdAlunos].nome, uTemp.nome, 63);
                alunosEncontrados[qtdAlunos].np1 = uTemp.np1;
                alunosEncontrados[qtdAlunos].np2 = uTemp.np2;
                alunosEncontrados[qtdAlunos].pim = uTemp.pim;
                alunosEncontrados[qtdAlunos].media = uTemp.media;
                qtdAlunos++;
            }
        }
        fclose(f);

        if (qtdAlunos == 0) {
            printf("\nNenhum aluno encontrado na turma %s.\n", turmaAlvo);
            return;
        }

        /* 3. Loop de Lançamento (SELEÇÃO POR ID) */
        while (1)
        {
            printf("\n================================ ALUNOS DA TURMA: %s ================================\n", turmaAlvo);
            printf("| %-5s | %-25s | %-5s | %-5s | %-5s | %-5s |\n", 
                   "ID", "NOME", "NP1", "NP2", "PIM", "MEDIA");
            printf("|-------|---------------------------|-------|-------|-------|-------|\n");

            for (int i = 0; i < qtdAlunos; i++) {
                printf("| %-5d | %-25.25s | %5.2f | %5.2f | %5.2f | %5.2f |\n", 
                       alunosEncontrados[i].id, 
                       alunosEncontrados[i].nome,
                       alunosEncontrados[i].np1,
                       alunosEncontrados[i].np2,
                       alunosEncontrados[i].pim,
                       alunosEncontrados[i].media);
            }
            printf("=====================================================================================\n");
            
            printf("Digite o ID do aluno para editar (ou 0 para Sair): ");
            
            if (!fgets(buf, sizeof(buf), stdin)) break;
            int idDigitado = atoi(buf);

            if (idDigitado == 0) break;

            /* Lógica de Validação: Verificar se o ID digitado está na lista filtrada */
            int indexEncontrado = -1;
            for (int i = 0; i < qtdAlunos; i++) {
                if (alunosEncontrados[i].id == idDigitado) {
                    indexEncontrado = i;
                    break;
                }
            }

            if (indexEncontrado == -1) {
                printf("\n[ERRO] O ID %d nao pertence a esta turma.\n", idDigitado);
                continue;
            }
            
            /* Se chegou aqui, o ID é válido e pertence à turma */
            UsuarioCSV aluno;
            if (buscarUsuarioPorID(idDigitado, &aluno))
            {
                char tmp[64];
                printf("\n>>> Editando notas de: %s (ID: %d)\n", aluno.nome, aluno.id);
                
                printf("NP1 Atual: %.2f | Nova NP1: ", aluno.np1);
                fgets(tmp, sizeof(tmp), stdin); 
                if (tmp[0] != '\n') aluno.np1 = (float)atof(tmp);

                printf("NP2 Atual: %.2f | Nova NP2: ", aluno.np2);
                fgets(tmp, sizeof(tmp), stdin);
                if (tmp[0] != '\n') aluno.np2 = (float)atof(tmp);

                printf("PIM Atual: %.2f | Novo PIM: ", aluno.pim);
                fgets(tmp, sizeof(tmp), stdin);
                if (tmp[0] != '\n') aluno.pim = (float)atof(tmp);

                aluno.media = calcularMedia(aluno.np1, aluno.np2, aluno.pim);
                printf("... Nova media calculada: %.2f\n", aluno.media);

                if(alterarUsuarioPorID(idDigitado, &aluno)) {
                    /* Atualiza a tabela visualmente */
                    alunosEncontrados[indexEncontrado].np1 = aluno.np1;
                    alunosEncontrados[indexEncontrado].np2 = aluno.np2;
                    alunosEncontrados[indexEncontrado].pim = aluno.pim;
                    alunosEncontrados[indexEncontrado].media = aluno.media;
                    printf("Sucesso!\n");
                }
            }
        }
    }
    else 
    {
        /* --- FLUXO (ADM/COORD) --- */
        int idBusca;
        UsuarioCSV aluno;
        char tmp[64];
        while (1)
        {
            listarApenasAlunos(); 
            printf("\n(MODO ADM) Digite o ID do aluno (ou 0 para voltar): ");
            if (scanf("%d", &idBusca) != 1) {
                int c; while ((c = getchar()) != '\n' && c != EOF);
                idBusca = -1;
            } else { getchar(); }
            
            if (idBusca == 0) break;
            if (idBusca < 0) continue;
            
            if (buscarUsuarioPorID(idBusca, &aluno)) {
                if (STRCASECMP(aluno.nivel, "Aluno") != 0) {
                    printf("Este ID nao e de um aluno.\n"); continue;
                }
                printf("\n--- Notas de %s ---\n", aluno.nome);
                printf("NP1 Atual: %.2f | Nova NP1: ", aluno.np1);
                fgets(tmp, sizeof(tmp), stdin); if(tmp[0]!='\n') aluno.np1 = (float)atof(tmp);
                
                printf("NP2 Atual: %.2f | Nova NP2: ", aluno.np2);
                fgets(tmp, sizeof(tmp), stdin); if(tmp[0]!='\n') aluno.np2 = (float)atof(tmp);
                
                printf("PIM Atual: %.2f | Novo PIM: ", aluno.pim);
                fgets(tmp, sizeof(tmp), stdin); if(tmp[0]!='\n') aluno.pim = (float)atof(tmp);

                aluno.media = calcularMedia(aluno.np1, aluno.np2, aluno.pim);
                alterarUsuarioPorID(idBusca, &aluno);
            } else {
                printf("ID nao encontrado.\n");
            }
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
    
    // ALTERADO: Converte para MAIUSCULO
    stringToUpper(novaTurma); 

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
            printf("Opção inválida.\n");
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
            printf("Opção inválida.\n");
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
        printf("2 - Lançar notas para alunos\n");
        printf("3 - Enviar Material / Atividades\n");
        printf("0 - Sair\n> ");
        
        if (scanf("%d", &opc) != 1) {
            while (getchar() != '\n');
            opc = -1;
        }
        while (getchar() != '\n');

        switch (opc)
        {
        case 1:
            mostrarUsuario(u);
            break;
        case 2:
            lancarNotasUI(u);
            break;
        case 3:
            enviarAtividadeUI(u);
            break;
        case 0:
            break;
        default:
            printf("Opção inválida.\n");
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
        printf("2 - Lançar/Gerenciar notas de alunos\n");
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
            lancarNotasUI(u);
            break;
        case 3:
            gerenciarTurmasUI();
            break;
        case 0:
            break;
        default:
            printf("Opção inválida.\n");
        }
    } while (opc != 0);
}

void gerenciarUsuariosUI(void)
{
    int opc = -1;
    char buffer[128]; // Buffer de entrada padronizado

    while (1)
    {
        printf("\n--- GERENCIAR USUÁRIOS ---\n");
        printf("1 - Listar todos usuários\n");
        printf("2 - Adicionar\n");
        printf("3 - Alterar por ID\n");
        printf("4 - Excluir por ID\n");
        printf("5 - Visualizar Senhas (Requer Autorização)\n");
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
            /* --- Logica de Adicionar Usuario (Padronizada MAIUSCULA) --- */
            UsuarioCSV u;
            memset(&u, 0, sizeof(u));
            int opcNivel = 0;
            int opcAtividade = 0;

            printf("Nome: ");
            fgets(u.nome, sizeof(u.nome), stdin);
            u.nome[strcspn(u.nome, "\n")] = 0;
            trim(u.nome);
            stringToUpper(u.nome);

            printf("Email: ");
            fgets(u.email, sizeof(u.email), stdin);
            u.email[strcspn(u.email, "\n")] = 0;
            trim(u.email);
            stringToLower(u.email); // Mantém minusculo

            printf("Senha: ");
            lerSenhaOculta(u.senha, sizeof(u.senha));
            trim(u.senha); // Mantém original (case-sensitive)

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
                    printf("Opção inválida. Tente novamente.\n\n");
                }
            }

            //Strings fixas em MAIUSCULO
            switch (opcNivel)
            {
            case 1:
                strncpy(u.nivel, "ALUNO", sizeof(u.nivel) - 1);
                break;
            case 2:
                strncpy(u.nivel, "PROFESSOR", sizeof(u.nivel) - 1);
                break;
            case 3:
                strncpy(u.nivel, "COORDENADOR", sizeof(u.nivel) - 1);
                break;
            case 4:
                strncpy(u.nivel, "ADMINISTRADOR", sizeof(u.nivel) - 1);
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
                    printf("Opção inválida. Tente novamente.\n\n");
                }
            }

            // ALTERADO: Strings fixas em MAIUSCULO
            if (opcAtividade == 1)
            {
                strncpy(u.atividade, "ATIVO", sizeof(u.atividade) - 1);
            }
            else
            {
                strncpy(u.atividade, "INATIVO", sizeof(u.atividade) - 1);
            }

            if (!adicionarUsuario(&u))
                printf("Falha ao adicionar usuario.\n");
        }
        else if (opc == 3)
        {
            /* Alterar Usuario (Padronizada MAIUSCULA) */
            int id;
            printf("ID a alterar: ");

            if (!fgets(buffer, sizeof(buffer), stdin))
            {
                continue;
            }
            id = atoi(buffer);
            if (id <= 0)
            {
                printf("ID inválido.\n");
                continue;
            }

            UsuarioCSV u;
            memset(&u, 0, sizeof(u));
            if (!buscarUsuarioPorID(id, &u))
            {
                printf("Usuário com ID %d nao encontrado.\n", id);
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
                stringToUpper(buffer); // ALTERADO: Converte entrada para maiúsculos
                strncpy(u.nome, buffer, sizeof(u.nome) - 1);
            }

            printf("Email (%s): ", u.email);
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            trim(buffer);
            if (buffer[0] != '\0')
            {
                stringToLower(buffer); // Mantém minúsculos
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
            // else: mantem a senha antiga que ja esta em 'u.senha'

            /* Menu de Nivel para Alteracao */
            while (1)
            {
                printf("Nivel (%s):\n", u.nivel);
                printf(" 1 - Aluno\n 2 - Professor\n 3 - Coordenador\n 4 - Administrador\n");
                printf("Digite a opção (1-4) ou [ENTER] para manter: ");

                fgets(buffer, sizeof(buffer), stdin);
                trim(buffer);

                if (buffer[0] == '\0')
                {
                    break;
                }

                opcNivel = atoi(buffer);
                if (opcNivel >= 1 && opcNivel <= 4)
                {
                    //Strings fixas em MAIUSCULO
                    switch (opcNivel)
                    {
                    case 1:
                        strncpy(u.nivel, "ALUNO", sizeof(u.nivel) - 1);
                        break;
                    case 2:
                        strncpy(u.nivel, "PROFESSOR", sizeof(u.nivel) - 1);
                        break;
                    case 3:
                        strncpy(u.nivel, "COORDENADOR", sizeof(u.nivel) - 1);
                        break;
                    case 4:
                        strncpy(u.nivel, "ADMINISTRADOR", sizeof(u.nivel) - 1);
                        break;
                    }
                    break;
                }
                else
                {
                    printf("Opção inválida. Tente novamente.\n\n");
                }
            }

            printf("Curso (%s): ", u.curso);
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            trim(buffer);
            if (buffer[0] != '\0')
            {
                stringToUpper(buffer); // Mantém UPPER
                strncpy(u.curso, buffer, sizeof(u.curso) - 1);
            }

            printf("Turma (%s): ", u.turma);
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            trim(buffer);
            if (buffer[0] != '\0')
            {
                stringToUpper(buffer); // Mantém UPPER
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
            printf("... Mádia (re)calculada: %.2f\n", u.media);

            /* Menu de Atividade para Alteracao */
            while (1)
            {
                printf("Atividade (%s):\n", u.atividade);
                printf(" 1 - Ativo\n 2 - Inativo\n");
                printf("Digite a opção (1-2) ou [ENTER] para manter: ");

                fgets(buffer, sizeof(buffer), stdin);
                trim(buffer);

                if (buffer[0] == '\0')
                {
                    break;
                }

                opcAtividade = atoi(buffer);
                if (opcAtividade == 1 || opcAtividade == 2)
                {
                    // Strings fixas em MAIUSCULO
                    strncpy(u.atividade, (opcAtividade == 1 ? "ATIVO" : "INATIVO"), sizeof(u.atividade) - 1);
                    break;
                }
                else
                {
                    printf("Opção inválida. Tente novamente.\n\n");
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
                printf("Falha ao excluir usuário.\n");
        }
        else if (opc == 5)
        {
            char passBuffer[128];
            printf("\n!!! AÇÃO RESTRITA !!!\n");
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
            printf("Opção inválida.\n");
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
        printf("2 - Gerenciar usuários (CRUD)\n");
        printf("3 - Lançar notas de alunos\n");
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
            lancarNotasUI(u);
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
            printf("Opção inválida.\n");
        }
    } while (opc != 0);
}

/* ----------------- EXECUTAR SISTEMA (menu principal) ----------------- */

void executarSistema(void)
{
    /* initSistema() */

    initSistema();
    criarArquivoSistemaSeNaoExiste();

    UsuarioCSV logado;
    char email[256], senha[128];
    int tentativas = 0;

    printf("\n==== SISTEMA ACADÊMICO UNIFICADO (Console C) ====\n");

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

// PROJETO FINAL
