#include <stdio.h>
#include <string.h>

// Função para limpar o buffer de entrada (stdin) após leituras como scanf("%d", ...)
void limpar_buffer_entrada() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        // Apenas consome os caracteres restantes
    }
}

// Função para cadastrar usuários
void cadastrarUsuarios() {
    FILE *arquivo;
    char nome[50], email[50], senha[20], nivel[50], curso[50];
    int id, idade, np1, np2, pim;
    int quantidade;

    // Verifica se o arquivo já existe e tem conteúdo
    int arquivoTemConteudo = 0;
    arquivo = fopen("tabela_usuario.csv", "r");
    if (arquivo != NULL) {
        fseek(arquivo, 0, SEEK_END);
        long tamanho = ftell(arquivo);
        arquivoTemConteudo = (tamanho > 0);
        fclose(arquivo);
    }

    // Abre o arquivo em modo append
    arquivo = fopen("tabela_usuario.csv", "a");
    if (arquivo == NULL) {
        printf("Erro ao abrir arquivo.\n");
        return;
    }

    // Escreve cabeçalho se o arquivo estiver vazio
    if (!arquivoTemConteudo) {
        fprintf(arquivo, "ID,NOME,EMAIL,SENHA,IDADE,NIVEL,CURSO,NP1,NP2,PIM\n");
    }

    printf("Quantos usuários deseja cadastrar? ");
    scanf("%d", &quantidade);
    limpar_buffer_entrada(); // Limpa o buffer após scanf

    for (int i = 0; i < quantidade; i++) {
        printf("\nCadastro do usuário %d\n", i + 1);

        printf("ID: ");
        scanf("%d", &id);
        limpar_buffer_entrada(); // Limpa o buffer após scanf

        printf("Nome: ");
        fgets(nome, sizeof(nome), stdin);
        nome[strcspn(nome, "\n")] = '\0';

        printf("Email: ");
        fgets(email, sizeof(email), stdin);
        email[strcspn(email, "\n")] = '\0';

        printf("Senha: ");
        fgets(senha, sizeof(senha), stdin);
        senha[strcspn(senha, "\n")] = '\0';

        printf("Idade: ");
        scanf("%d", &idade);
        limpar_buffer_entrada(); // Limpa o buffer após scanf

        // Menu de níveis de acesso
        int opcaoNivel;
        printf("Selecione o nível de acesso:\n");
        printf("1 - Admin\n");
        printf("2 - Coordenador\n");
        printf("3 - Professor\n");
        printf("4 - Aluno\n");
        printf("Opção: ");
        scanf("%d", &opcaoNivel);
        limpar_buffer_entrada(); // Limpa o buffer após scanf

        switch (opcaoNivel) {
            case 1: strcpy(nivel, "admin"); break;
            case 2: strcpy(nivel, "coordenador"); break;
            case 3: strcpy(nivel, "professor"); break;
            case 4: strcpy(nivel, "aluno"); break;
            default:
                printf("Opção inválida. Nível definido como 'aluno'.\n");
                strcpy(nivel, "aluno");
        }

        printf("Curso: ");
        fgets(curso, sizeof(curso), stdin);
        curso[strcspn(curso, "\n")] = '\0';

        printf("NP1: ");
        scanf("%d", &np1);
        limpar_buffer_entrada(); // Limpa o buffer após scanf

        printf("NP2: ");
        scanf("%d", &np2);
        limpar_buffer_entrada(); // Limpa o buffer após scanf

        printf("PIM: ");
        scanf("%d", &pim);
        limpar_buffer_entrada(); // Limpa o buffer após scanf

        // Grava os dados no arquivo
        fprintf(arquivo, "%d,%s,%s,%s,%d,%s,%s,%d,%d,%d\n",
                id, nome, email, senha, idade, nivel, curso, np1, np2, pim);
    }

    fclose(arquivo);
    printf("\n✅ Dados gravados com sucesso!\n");
}

// Função para login e verificação de acesso
void loginUsuario() {
    FILE *arquivo;
    char email_login[50], senha_login[20];
    char linha[300];

    char email[50], senha[20], nivel[50];
    int id, idade, np1, np2, pim;
    char nome[50], curso[50];

    printf("\n🔐 Login do usuário\n");
    printf("Email: ");
    fgets(email_login, sizeof(email_login), stdin);
    email_login[strcspn(email_login, "\n")] = '\0';

    printf("Senha: ");
    fgets(senha_login, sizeof(senha_login), stdin);
    senha_login[strcspn(senha_login, "\n")] = '\0';

    arquivo = fopen("tabela_usuario.csv", "r");
    if (arquivo == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        return;
    }

    int encontrado = 0;
    fgets(linha, sizeof(linha), arquivo); // pula cabeçalho

    while (fgets(linha, sizeof(linha), arquivo)) {
        // Nota: sscanf com %[^\n] pode ser perigoso se a linha for mais longa que o buffer
        // Mas para este caso, e visto que a origem é seu próprio fprintf, deve funcionar.
        // O sscanf está correto para ignorar as vírgulas e limitar o tamanho da string.
        sscanf(linha, "%d,%49[^,],%49[^,],%19[^,],%d,%49[^,],%49[^,],%d,%d,%d",
                &id, nome, email, senha, &idade, nivel, curso, &np1, &np2, &pim);

        if (strcmp(email_login, email) == 0 && strcmp(senha_login, senha) == 0) {
            encontrado = 1;
            break;
        }
    }

    fclose(arquivo);

    if (encontrado) {
        printf("\n✅ Login bem-sucedido!\n");
        printf("Bem-vindo, %s (%s)\n", nome, nivel);

        // Ações por nível
        if (strcmp(nivel, "admin") == 0) {
            printf("🔧 Acesso total ao sistema.\n");
        } else if (strcmp(nivel, "coordenador") == 0) {
            printf("📊 Pode visualizar relatórios e gerenciar professores.\n");
        } else if (strcmp(nivel, "professor") == 0) {
            printf("📝 Pode lançar notas e acompanhar alunos.\n");
        } else if (strcmp(nivel, "aluno") == 0) {
            printf("📚 Pode consultar suas notas e dados pessoais.\n");
        } else {
            printf("⚠️ Nível de acesso desconhecido.\n");
        }
    } else {
        printf("\n❌ Email ou senha incorretos.\n");
    }
}

// Função principal com menu
int main() {
    int opcao;

    do {
        printf("\n📋 Menu Principal\n");
        printf("1 - Cadastrar usuários\n");
        printf("2 - Fazer login\n");
        printf("0 - Sair\n");
        printf("Escolha uma opção: ");
        scanf("%d", &opcao);
        limpar_buffer_entrada(); // Limpa o buffer após scanf do menu

        switch (opcao) {
            case 1:
                cadastrarUsuarios();
                break;
            case 2:
                loginUsuario();
                break;
            case 0:
                printf("Encerrando o programa...\n");
                break;
            default:
                printf("Opção inválida.\n");
        }
    } while (opcao != 0);

    return 0;
}