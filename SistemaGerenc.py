import customtkinter as ctk
import tkinter as tk
from tkinter import ttk, messagebox, filedialog
import os
import io
import pandas as pd
from pathlib import Path
from PIL import Image

# --- 1. CONSTANTES E FUNÇÕES LGPD ---
CAMINHO_ARQUIVO = "Output/SistemaAcademico.csv"
SECAO_ALVO = "[USUARIOS]"
CONSENT_FILE = "lgpd_consent.txt"


def check_lgpd_consent():
    """Verifica se o usuário já aceitou os termos da LGPD."""
    return os.path.exists(CONSENT_FILE)


def set_lgpd_consent():
    """Registra o consentimento do usuário em um arquivo."""
    try:
        with open(CONSENT_FILE, "w") as f:
            f.write("accepted")
    except Exception as e:
        print(f"Erro ao salvar consentimento: {e}")


# === CLASSE POP-UP LGPD
class LGPDPopup(ctk.CTkToplevel):
    def __init__(self, master, on_accept_callback):
        super().__init__(master)
        self.title("Aviso de Privacidade (LGPD)")
        self.on_accept_callback = on_accept_callback
        popup_width, popup_height = 500, 350
        screen_width, screen_height = (
            self.winfo_screenwidth(),
            self.winfo_screenheight(),
        )
        pos_x, pos_y = (screen_width // 2) - (popup_width // 2), (
            screen_height // 2
        ) - (popup_height // 2)
        self.geometry(f"{popup_width}x{popup_height}+{pos_x}+{pos_y}")
        self.resizable(False, False)
        self.transient(master)
        self.grab_set()
        self.protocol("WM_DELETE_WINDOW", master.quit)
        msg = "De acordo com a Lei Geral de Proteção de Dados (LGPD), precisamos do seu consentimento para processar seus dados. Ao clicar em 'Aceitar', você concorda com o uso de seus dados (como login, senha e notas) para o funcionamento deste aplicativo."
        ctk.CTkLabel(self, text=msg, wraplength=380, font=("Arial", 14)).pack(
            pady=30, padx=20, fill="x"
        )
        ctk.CTkButton(
            self,
            text="Aceitar e Continuar",
            command=self.accept,
            fg_color="green",
            font=("Arial", 12, "bold"),
        ).pack(pady=20, padx=20, ipady=5)

    def accept(self):
        set_lgpd_consent()
        self.protocol("WM_DELETE_WINDOW", lambda: None)
        self.grab_release()
        self.destroy()
        self.on_accept_callback()


# === FUNÇÃO DE CARREGAMENTO
def carregar_tabela(caminho_arquivo):
    if not os.path.exists(caminho_arquivo):
        messagebox.showerror(
            "Erro de Leitura",
            f"Arquivo CSV não encontrado no caminho: {caminho_arquivo}",
        )
        return pd.DataFrame()
    try:
        with open(caminho_arquivo, "r", encoding="utf-8", errors="ignore") as f:
            lines = f.readlines()

        start_index = -1
        end_index = -1
        for i, line in enumerate(lines):
            if line.strip().upper() == SECAO_ALVO:
                start_index = i + 1
                break
        if start_index == -1:
            messagebox.showerror(
                "Erro de Formato",
                f"Não foi possível encontrar a seção {SECAO_ALVO} no arquivo {caminho_arquivo}.",
            )
            return pd.DataFrame()

        for i in range(start_index, len(lines)):
            if lines[i].strip().startswith("["):
                end_index = i
                break
        if end_index == -1:
            end_index = len(lines)

        usuarios_lines = lines[start_index:end_index]
        usuarios_lines_clean = [line for line in usuarios_lines if line.strip()]

        header_default = (
            "id;nome;email;senha;nivel;curso;turma;idade;np1;np2;pim;media;atividade"
        )
        if len(usuarios_lines_clean) <= 1:
            header_line = (
                usuarios_lines_clean[0] if usuarios_lines_clean else header_default
            )
            colunas = [col.strip() for col in header_line.split(";")]
            return pd.DataFrame(columns=colunas)

        header_line = usuarios_lines_clean[0]
        colunas = [col.strip() for col in header_line.split(";")]
        data_lines = "".join(usuarios_lines_clean[1:])
        csv_file_like = io.StringIO(data_lines)
        df = pd.read_csv(csv_file_like, sep=";", header=None, names=colunas)

        df.columns = df.columns.str.strip().str.upper()

        column_map = {"TURMA": "ID_TURMAS", "ATIVIDADE": "STATUS DO ALUNO"}
        df.rename(columns=column_map, inplace=True)

        cols_numericas = ["NP1", "NP2", "PIM", "MEDIA"]
        for col in cols_numericas:
            if col in df.columns:
                df[col] = df[col].astype(str).str.replace(",", ".", regex=False)
                df[col] = pd.to_numeric(df[col], errors="coerce").fillna(0)

        cols_inteiras = ["ID", "IDADE"]
        for col in cols_inteiras:
            if col in df.columns:
                df[col] = pd.to_numeric(df[col], errors="coerce").fillna(0).astype(int)

        if "NOME" in df.columns:
            df["NOME"] = df["NOME"].astype(str).str.upper()
        if "EMAIL" in df.columns:
            df["EMAIL"] = df["EMAIL"].astype(str).str.lower()
        if "SENHA" in df.columns:
            df["SENHA"] = df["SENHA"].astype(str)
        if "NIVEL" in df.columns:
            df["NIVEL"] = df["NIVEL"].astype(str).str.upper()

        if all(col in df.columns for col in ["NP1", "NP2", "PIM"]):
            df["MEDIA"] = (df["NP1"] * 4 + df["NP2"] * 4 + df["PIM"] * 2) / 10
            df["MEDIA"] = df["MEDIA"].round(2)

        return df
    except Exception as e:
        messagebox.showerror(
            "Erro de Leitura", f"Não foi possível ler o arquivo CSV.\nErro: {e}"
        )
        return pd.DataFrame()


respostas = {
    "Quais cursos posso me matrícular?": "As aulas disponibilizadas para esse semestre são Educação Ambiental, Redes de Computadores, Banco de Dados, Inteligência Artificial, Ciberssegurança, Programação Orientada a Objetos, Python, Java, C / C++ e Análise e Projeto de Sistemas.",
    "Como calcular a média final?": "A média é calculada com média ponderada, onde cada prova tem peso 4 e o trabalho final tem peso 2. A fórmula é: (NP1 * 4 + NP2 * 4 + PIM * 2) / 10.",
    "Quais os horários que posso fazer as aulas?": "A partir do momento em que você se matricula em uma disciplina, tem um período de 6 meses para completar o curso.",
    "Quem é o coordenador geral?": "O coordenador geral é o Prof. Cordeiro, escolhido dentro da sua instituição.",
    "Qual é o prazo para entrega dos trabalhos?": "A data de entrega dos trabalhos é até o final do semestre.",
    "Qual é o conteúdo da aula de segunda-feira?": "Na segunda-feira, estudamos Programação Orientada a Objetos e Java.",
    "Qual é o conteúdo da aula de terça-feira?": "Na terça-feira, estudamos Educação Ambiental e C / C++.",
    "Qual é o conteúdo da aula de quarta-feira?": "Na quarta-feira, estudamos Redes de Computadores e Análise e Projeto de Sistemas.",
    "Qual é o conteúdo da aula de quinta-feira?": "Na quarta-feira, estudamos Banco de Dados e Ciberssegurança.",
    "Qual é o conteúdo da aula de sexta-feira?": "Na sexta-feira, estudamos Inteligência Artificial e Python.",
    "Como funciona a avaliação do curso?": "A avaliação são duas provas de 12 questões, sendo 10 alternativas e 2 dissertativas e um trabalho final.",
    "Quais são os horários de atendimento do coordenador?": "O Prof. Cordeiro atende às quartas, das 14h às 16h.",
}


# === CLASSE 3: CHATBOT
class Chatbot(ctk.CTkFrame):
    def __init__(self, master):
        super().__init__(master)
        self.pack(fill="both", expand=True, padx=10, pady=10)
        ctk.CTkLabel(
            self, text="Assistente Acadêmico", font=("Arial", 18, "bold")
        ).pack(pady=10)
        self.area_chat = tk.Text(
            self, height=12, state=tk.DISABLED, wrap=tk.WORD, font=("Arial", 10)
        )
        self.area_chat.pack(padx=10, pady=5, fill="both", expand=True)
        frame_botoes = ctk.CTkFrame(self)
        frame_botoes.pack(padx=10, pady=10, fill="x")
        ctk.CTkLabel(
            frame_botoes, text="Perguntas Frequentes:", font=("Arial", 12, "bold")
        ).grid(row=0, column=0, columnspan=2, sticky="w", padx=5, pady=5)
        canvas = tk.Canvas(
            frame_botoes,
            height=200,
            bg=self._apply_appearance_mode(
                ctk.ThemeManager.theme["CTkFrame"]["fg_color"]
            ),
            highlightthickness=0,
        )
        v_scrollbar = ctk.CTkScrollbar(
            frame_botoes, orientation="vertical", command=canvas.yview
        )
        scrollable_frame = ctk.CTkFrame(canvas)
        scrollable_window_id = canvas.create_window(
            (0, 0), window=scrollable_frame, anchor="nw"
        )
        canvas.configure(yscrollcommand=v_scrollbar.set)

        def on_canvas_configure(event):
            canvas.configure(scrollregion=canvas.bbox("all"))
            canvas.itemconfig(scrollable_window_id, width=event.width)

        canvas.bind("<Configure>", on_canvas_configure)
        scrollable_frame.bind(
            "<Configure>", lambda e: canvas.configure(scrollregion=canvas.bbox("all"))
        )

        v_scrollbar.grid(row=1, column=2, sticky="ns")
        canvas.grid(row=1, column=0, columnspan=2, sticky="nsew")
        frame_botoes.grid_columnconfigure(0, weight=1)
        frame_botoes.grid_rowconfigure(1, weight=1)

        perguntas = list(respostas.keys())
        num_perguntas = len(perguntas)
        num_colunas = 2

        scrollable_frame.grid_columnconfigure(0, weight=1)
        scrollable_frame.grid_columnconfigure(1, weight=1)

        for i in range(num_perguntas):
            pergunta = perguntas[i]
            row = i // num_colunas
            col = i % num_colunas
            btn = ctk.CTkButton(
                scrollable_frame,
                text=pergunta,
                command=lambda p=pergunta: self.fazer_pergunta(p),
                corner_radius=8,
                height=40,
            )
            btn.grid(row=row, column=col, padx=10, pady=5, sticky="ew")

        self.mostrar_mensagem_boas_vindas()

    def mostrar_mensagem_boas_vindas(self):
        self.adicionar_mensagem(
            "Assistente",
            "Olá! Sou seu assistente acadêmico. Clique em uma pergunta para começar.",
        )

    def adicionar_mensagem(self, remetente, mensagem):
        self.area_chat.config(state=tk.NORMAL)
        tag_color = "blue" if remetente == "Assistente" else "green"
        self.area_chat.tag_config(
            remetente, foreground=tag_color, font=("Arial", 10, "bold")
        )
        self.area_chat.insert(tk.END, f"{remetente}: ", remetente)
        self.area_chat.insert(tk.END, f"{mensagem}\n\n")
        self.area_chat.config(state=tk.DISABLED)
        self.area_chat.see(tk.END)

    def fazer_pergunta(self, pergunta):
        self.adicionar_mensagem("Você", pergunta)
        self.adicionar_mensagem("Assistente", respostas[pergunta])


# === CLASSE 1: TELA DE LOGIN
class LoginFrame(ctk.CTkFrame):
    def __init__(self, master, login_callback, forgot_password_callback):
        super().__init__(master)
        self.login_callback = login_callback
        self.forgot_password_callback = forgot_password_callback
        self.grid_rowconfigure(0, weight=1)
        self.grid_rowconfigure(8, weight=1)
        self.grid_columnconfigure(0, weight=1)
        self.grid_columnconfigure(2, weight=1)

        logo_path = Path(__file__).parent / "logo.png"
        try:
            pil_image = Image.open(logo_path)
            logo_image = ctk.CTkImage(pil_image, size=(80, 80))
            self.logo_label = ctk.CTkLabel(self, image=logo_image, text="")
        except FileNotFoundError:
            self.logo_label = ctk.CTkLabel(self, text="[Logo]", font=("Arial", 16))
        self.logo_label.grid(row=1, column=1, padx=20, pady=(10, 10))

        ctk.CTkLabel(
            self, text="Acesso ao Sistema Acadêmico", font=("Arial", 18, "bold")
        ).grid(row=2, column=1, padx=20, pady=(10, 10))
        self.username_entry = ctk.CTkEntry(
            self, placeholder_text="Usuário (Nome/Email)", width=250
        )
        self.username_entry.grid(row=3, column=1, padx=20, pady=10, sticky="ew")
        self.password_entry = ctk.CTkEntry(
            self, placeholder_text="Senha", show="*", width=250
        )
        self.password_entry.grid(row=4, column=1, padx=20, pady=10, sticky="ew")

        self.show_password_var = ctk.StringVar(value="off")
        self.show_password_check = ctk.CTkCheckBox(
            self,
            text="Mostrar Senha",
            variable=self.show_password_var,
            onvalue="on",
            offvalue="off",
            command=self.toggle_password_visibility,
            font=("Arial", 12),
        )
        self.show_password_check.grid(
            row=5, column=1, padx=20, pady=(0, 10), sticky="w"
        )

        self.login_button = ctk.CTkButton(
            self,
            text="Entrar",
            command=self.attempt_login,
            width=250,
            fg_color="#3C66E0",
        )
        self.login_button.grid(row=6, column=1, padx=20, pady=(10, 10), sticky="ew")

        self.forgot_password_button = ctk.CTkButton(
            self,
            text="Esqueci minha senha",
            command=self.forgot_password_callback,
            fg_color="transparent",
            text_color="#3C66E0",
            hover_color="#444444",
            font=("Arial", 12),
        )
        self.forgot_password_button.grid(row=7, column=1, padx=20, pady=(0, 20))

        self.password_entry.bind("<Return>", lambda event: self.attempt_login())
        self.username_entry.bind("<Return>", lambda event: self.attempt_login())

    def toggle_password_visibility(self):
        if self.show_password_var.get() == "on":
            self.password_entry.configure(show="")
        else:
            self.password_entry.configure(show="*")

    def attempt_login(self):
        username = self.username_entry.get()
        password = self.password_entry.get()
        if not username or not password:
            messagebox.showwarning("Aviso", "Por favor, insira o usuário e a senha.")
            return

        self.login_button.configure(state="disabled", text="Verificando...")
        self.update_idletasks()

        login_successful = self.login_callback(username, password)
        if not login_successful:
            self.login_button.configure(state="normal", text="Entrar")


# === CLASSE AUXILIAR: Gerenciar Turmas
class GerenciarTurmasFrame(ctk.CTkFrame):
    def __init__(self, master, main_app_instance):
        super().__init__(master)
        self.pack(fill="both", expand=True, padx=10, pady=10)
        self.main_app = main_app_instance

        self.df_turmas = pd.DataFrame()
        self.tabela_turmas_widget = None

        # Estrutura de Layout
        frame_controles = ctk.CTkFrame(self)
        frame_controles.pack(fill="x", pady=(0, 10))

        ctk.CTkLabel(
            frame_controles,
            text="🎓 Gerenciamento de Turmas",
            font=("Arial", 16, "bold"),
        ).pack(pady=5)

        # Frame para Filtros e Ações
        self.frame_filtros_acoes = ctk.CTkFrame(frame_controles)
        self.frame_filtros_acoes.pack(fill="x", padx=10, pady=(0, 10))

        self.criar_widgets_filtros()

        # Frame para a Tabela
        self.frame_tabela_turmas = ctk.CTkFrame(self)
        self.frame_tabela_turmas.pack(fill="both", expand=True, padx=0, pady=0)

        # Carrega e exibe os dados iniciais
        self.atualizar_tabela_turmas()

    def criar_widgets_filtros(self):
        # Filtro por Curso (Combo Box)
        ctk.CTkLabel(self.frame_filtros_acoes, text="Filtrar Curso:").grid(
            row=0, column=0, padx=(10, 5), pady=10, sticky="w"
        )

        cursos = ["Todos os Cursos"] + self._get_unique_values("CURSO")
        self.combo_filtro_curso = ctk.CTkComboBox(
            self.frame_filtros_acoes,
            values=cursos,
            width=200,
            command=self.filtrar_turmas,
        )
        self.combo_filtro_curso.set(cursos[0])
        self.combo_filtro_curso.grid(row=0, column=1, padx=5, pady=10, sticky="w")

        # Botão Limpar Filtros
        botao_limpar = ctk.CTkButton(
            self.frame_filtros_acoes,
            text="Limpar Filtro",
            command=self.limpar_filtros,
            width=120,
        )
        botao_limpar.grid(row=0, column=2, padx=(20, 5), pady=10, sticky="w")

        # Botão Atualizar
        botao_atualizar = ctk.CTkButton(
            self.frame_filtros_acoes,
            text="🔄 Recarregar Dados",
            command=self.atualizar_tabela_turmas,
            width=150,
        )
        botao_atualizar.grid(row=0, column=3, padx=(5, 10), pady=10, sticky="w")

        self.frame_filtros_acoes.grid_columnconfigure(4, weight=1)  # Espaçador

    def _get_unique_values(self, column_name):
        """Obtém valores únicos de uma coluna do dataframe principal (ignora NaN)."""
        df_full = self.main_app.data_frame_full
        if df_full is not None and not df_full.empty and column_name in df_full.columns:
            # Garante que o tipo é string antes de chamar .unique()
            return df_full[column_name].astype(str).str.strip().unique().tolist()
        return []

    def gerar_dados_turmas(self):
        """Gera um DataFrame de resumo com estatísticas por Turma."""
        df_full = self.main_app.data_frame_full
        if df_full is None or df_full.empty:
            return pd.DataFrame(
                columns=["TURMA", "CURSO", "TOTAL ALUNOS", "MÉDIA GERAL"]
            )

        df_alunos = df_full[df_full["NIVEL"] == "ALUNO"].copy()

        if df_alunos.empty or "ID_TURMAS" not in df_alunos.columns:
            return pd.DataFrame(
                columns=["TURMA", "CURSO", "TOTAL ALUNOS", "MÉDIA GERAL"]
            )

        # Agrupar e calcular estatísticas
        df_resumo = (
            df_alunos.groupby("ID_TURMAS")
            .agg({"ID": "count", "MEDIA": "mean", "CURSO": "first"})
            .reset_index()
        )

        df_resumo.rename(
            columns={
                "ID_TURMAS": "TURMA",
                "ID": "TOTAL ALUNOS",
                "MEDIA": "MÉDIA GERAL",
            },
            inplace=True,
        )

        # Formatação
        df_resumo["MÉDIA GERAL"] = df_resumo["MÉDIA GERAL"].round(2)
        df_resumo["TURMA"] = df_resumo["TURMA"].astype(str)
        df_resumo.sort_values(by="TURMA", inplace=True)
        df_resumo.reset_index(drop=True, inplace=True)
        return df_resumo

    def filtrar_turmas(self, *args):
        """Aplica o filtro de curso e atualiza a tabela."""
        curso_selecionado = self.combo_filtro_curso.get()
        df_filtrado = self.gerar_dados_turmas()

        if curso_selecionado != "Todos os Cursos":
            df_filtrado = df_filtrado[df_filtrado["CURSO"] == curso_selecionado.upper()]

        self.mostrar_tabela_turmas(df_filtrado)

    def limpar_filtros(self):
        """Limpa o filtro e recarrega a tabela completa de turmas."""
        self.combo_filtro_curso.set("Todos os Cursos")
        self.atualizar_tabela_turmas(force_reload_csv=True)

    def atualizar_tabela_turmas(self, force_reload_csv=False):
        """Recarrega os dados do CSV (se necessário), gera o resumo e exibe a tabela."""
        if force_reload_csv:
            self.main_app.data_frame_full = carregar_tabela(CAMINHO_ARQUIVO)
            # Atualiza a lista de cursos no combobox após recarregar
            cursos = ["Todos os Cursos"] + self._get_unique_values("CURSO")
            self.combo_filtro_curso.configure(values=cursos)
            self.combo_filtro_curso.set("Todos os Cursos")

        self.df_turmas = self.gerar_dados_turmas()
        self.mostrar_tabela_turmas(self.df_turmas)

    def mostrar_tabela_turmas(self, df):
        """Exibe o DataFrame de Turmas na Treeview."""
        for widget in self.frame_tabela_turmas.winfo_children():
            widget.destroy()

        if df.empty:
            ctk.CTkLabel(
                self.frame_tabela_turmas,
                text="Nenhuma turma encontrada ou dados insuficientes.",
                text_color="red",
            ).pack(pady=20)
            self.tabela_turmas_widget = None
            return

        colunas = list(df.columns)
        self.tabela_turmas_widget = ttk.Treeview(
            self.frame_tabela_turmas, columns=colunas, show="headings"
        )

        vsb = ttk.Scrollbar(
            self.frame_tabela_turmas,
            orient="vertical",
            command=self.tabela_turmas_widget.yview,
        )
        hsb = ttk.Scrollbar(
            self.frame_tabela_turmas,
            orient="horizontal",
            command=self.tabela_turmas_widget.xview,
        )
        self.tabela_turmas_widget.configure(
            yscrollcommand=vsb.set, xscrollcommand=hsb.set
        )

        vsb.pack(side="right", fill="y")
        hsb.pack(side="bottom", fill="x")
        self.tabela_turmas_widget.pack(fill="both", expand=True)

        for col in colunas:
            self.tabela_turmas_widget.heading(col, text=col.replace("_", " "))

            col_width = 150
            if col == "TURMA":
                col_width = 120
            elif col == "MÉDIA GERAL":
                col_width = 150
            elif col == "TOTAL ALUNOS":
                col_width = 120

            self.tabela_turmas_widget.column(
                col, anchor="center", width=col_width, minwidth=50
            )

        for i, row in df.iterrows():
            row_list = list(row)

            # Formatação de campos numéricos (MÉDIA GERAL)
            if "MÉDIA GERAL" in colunas:
                media_index = colunas.index("MÉDIA GERAL")
                try:
                    media_valor = row_list[media_index]
                    row_list[media_index] = f"{media_valor:.2f}"
                except (ValueError, TypeError):
                    pass

            self.tabela_turmas_widget.insert("", "end", iid=i, values=row_list)


# === CLASSE 2: APLICAÇÃO PRINCIPAL (CORRIGIDA)
class MainApp(ctk.CTk):
    def __init__(self):
        super().__init__()
        self.title("Sistema Acadêmico Integrado")
        self.geometry("1100x700")
        if os.name == "nt":
            pass
        else:
            pass

        self.data_frame_full = None
        self.data_frame = pd.DataFrame()
        self.tabela_widget = None
        self.frame_tabela_dados = None
        self.current_user = None

        self.container = ctk.CTkFrame(self)
        self.container.pack(fill="both", expand=True)

    def start_app_with_lgpd_check(self):
        """Verifica o consentimento LGPD antes de iniciar o app."""
        self.center_window(400, 480)
        if not check_lgpd_consent():
            self.withdraw()
            LGPDPopup(self, self.on_lgpd_accepted)
        else:
            self.on_lgpd_accepted()

    def on_lgpd_accepted(self):
        """Chamado após o usuário aceitar a LGPD. Inicia a tela de login."""
        self.deiconify()
        self.after(50, self.show_login)

    def center_window(self, width=1100, height=700):
        """Função auxiliar para centralizar a janela principal."""
        screen_width = self.winfo_screenwidth()
        screen_height = self.winfo_screenheight()
        x = (screen_width // 2) - (width // 2)
        y = (screen_height // 2) - (height // 2)
        self.geometry(f"{width}x{height}+{x}+{y}")

    def show_login(self):
        """Mostra a tela de Login."""
        for widget in self.container.winfo_children():
            widget.destroy()
        login_frame = LoginFrame(
            self.container, self.authenticate_user, self.handle_forgot_password
        )
        login_frame.pack(fill="both", expand=True)
        self.center_window(400, 480)
        self.title("Login - Sistema Acadêmico")
        self.container.grid_columnconfigure(0, weight=1)
        self.container.grid_rowconfigure(0, weight=1)

    def show_main_content(self):
        """Mostra a tela principal após o login."""
        for widget in self.container.winfo_children():
            widget.destroy()
        self.center_window(1100, 700)
        self.title(
            f"Sistema Acadêmico Integrado - Logado como {self.current_user.get('NOME', 'N/A')} ({self.current_user.get('NIVEL', 'N/A')})"
        )
        self.create_user_info_banner(self.container)
        self.create_main_tabs(self.container)
        self.atualizar_tabela(reload_csv=False)

    def create_user_info_banner(self, master):
        user_name = self.current_user.get("NOME", "N/A")
        user_id = self.current_user.get("ID", "N/A")
        user_level = self.current_user.get("NIVEL", "N/A")
        banner_frame = ctk.CTkFrame(master, height=40)
        banner_frame.pack(fill="x", padx=10, pady=(10, 5))
        banner_frame.grid_columnconfigure(0, weight=1)
        banner_frame.grid_columnconfigure(1, weight=0)
        info_text = f"👤 Logado como: {user_name} (ID: {user_id}) | Nível: {user_level}"
        info_label = ctk.CTkLabel(
            banner_frame,
            text=info_text,
            font=("Arial", 14, "bold"),
            text_color="#B0B6BB",
        )
        info_label.grid(row=0, column=0, padx=20, pady=5, sticky="w")
        logout_button = ctk.CTkButton(
            banner_frame, text="Sair", command=self.show_login, width=80, fg_color="red"
        )
        logout_button.grid(row=0, column=1, padx=(0, 10), pady=5, sticky="e")

    def handle_forgot_password(self):
        messagebox.showinfo(
            "Recuperação de Senha",
            "A funcionalidade de recuperação de senha (ex: Toplevel) seria iniciada aqui.",
            parent=self.container,
        )

    def authenticate_user(self, username_or_email, password):
        if self.data_frame_full is None:
            self.data_frame_full = carregar_tabela(CAMINHO_ARQUIVO)
        df_auth = self.data_frame_full

        if df_auth is None or df_auth.empty:
            messagebox.showerror(
                "Erro de Login",
                "Não foi possível carregar os dados de usuários. Verifique o arquivo CSV.",
            )
            return False
        if not all(
            col in df_auth.columns for col in ["NOME", "EMAIL", "SENHA", "NIVEL"]
        ):
            messagebox.showerror(
                "Erro de Configuração",
                "O arquivo CSV deve conter as colunas 'nome', 'email', 'senha' e 'nivel'.",
            )
            return False

        input_lower = username_or_email.strip().lower()
        user_row = df_auth[
            (df_auth["EMAIL"] == input_lower)
            | (df_auth["NOME"] == username_or_email.strip().upper())
        ]

        if user_row.empty:
            messagebox.showerror("Login Falhou", "Usuário não encontrado.")
            return False

        user = user_row.iloc[0]

        if str(user["SENHA"]) == str(password):
            self.current_user = user
            messagebox.showinfo("Sucesso", f"Bem-vindo(a), {user['NOME']}!")
            self.show_main_content()
            return True
        else:
            messagebox.showerror("Login Falhou", "Senha incorreta.")
            return False

    def _gerar_novo_id(self):
        if self.data_frame_full is None or self.data_frame_full.empty:
            return 1
        if "ID" in self.data_frame_full.columns:
            max_id = self.data_frame_full["ID"].max()
            return int(max_id) + 1
        return 1

    def create_main_tabs(self, master):
        style = ttk.Style(self)
        style.theme_use("default")
        style.configure("Treeview", font=("Arial", 10), rowheight=25)
        style.configure("Treeview.Heading", font=("Arial", 10, "bold"))
        style.map("Treeview", background=[("selected", "blue")])

        self.abas = ttk.Notebook(master)

        user_level = self.current_user.get("NIVEL", "ALUNO")

        if user_level in ["ADMINISTRADOR", "COORDENADOR", "PROFESSOR"]:
            frame_turmas = ctk.CTkFrame(self.abas)
            self.abas.add(frame_turmas, text="Gerenciar Turmas")
            GerenciarTurmasFrame(frame_turmas, self)

        if user_level == "ALUNO":
            frame_chatbot = ctk.CTkFrame(self.abas)
            self.abas.add(frame_chatbot, text="Assistente Acadêmico")
            Chatbot(frame_chatbot)

        frame_tabela = ctk.CTkFrame(self.abas)
        self.abas.add(frame_tabela, text="Tabela de Usuários")
        self.abas.pack(fill="both", expand=True, padx=10, pady=(0, 10))

        self.create_tabela_frame(frame_tabela)

    def create_tabela_frame(self, frame_tabela):
        frame_controles = ctk.CTkFrame(frame_tabela)
        frame_controles.pack(fill="x", padx=10, pady=10)

        user_level = (
            self.current_user["NIVEL"]
            if self.current_user is not None
            else "NÃO AUTORIZADO"
        )
        user_is_admin = user_level == "ADMINISTRADOR"
        user_is_student = user_level == "ALUNO"
        user_is_coordinator = user_level == "COORDENADOR"
        user_is_professor = user_level == "PROFESSOR"

        titulo = ctk.CTkLabel(
            frame_controles,
            text=f"Gerenciamento de Usuários (Nível: {user_level})",
            font=("Arial", 16, "bold"),
        )
        titulo.pack(pady=5)

        # --- Frame de Botões Padrão para TODOS os níveis ---
        standard_buttons_frame = ctk.CTkFrame(frame_controles)
        standard_buttons_frame.pack(pady=(10, 15), fill="x", padx=20)

        botao_ver_dados = ctk.CTkButton(
            standard_buttons_frame,
            text="👤 Ver Meus Dados",
            command=self.abrir_janela_meus_dados,
            fg_color="#007BFF",
            font=("Arial", 12, "bold"),
        )

        if user_is_student:
            standard_buttons_frame.grid_columnconfigure(0, weight=1)
            standard_buttons_frame.grid_columnconfigure(1, weight=1)
            standard_buttons_frame.grid_columnconfigure(2, weight=1)

            botao_enviar_atividade = ctk.CTkButton(
                standard_buttons_frame,
                text="🚀 Enviar Atividades",
                command=self.open_activity_submission_window,
                fg_color="#4CAF50",
                font=("Arial", 14, "bold"),
            )
            botao_enviar_atividade.grid(row=0, column=0, padx=5, pady=5, sticky="ew")

            botao_ver_dados.grid(row=0, column=1, padx=5, pady=5, sticky="ew")

            botao_atualizar_aluno = ctk.CTkButton(
                standard_buttons_frame,
                text="🔄 Recarregar Tabela",
                command=lambda: self.atualizar_tabela(reload_csv=True),
                font=("Arial", 12),
            )
            botao_atualizar_aluno.grid(row=0, column=2, padx=5, pady=5, sticky="ew")

        elif user_is_admin or user_is_coordinator or user_is_professor:

            standard_buttons_frame.grid_columnconfigure(0, weight=1)
            standard_buttons_frame.grid_columnconfigure(1, weight=5)
            botao_ver_dados.grid(row=0, column=0, padx=5, pady=5, sticky="w")

            frame_controles_especificos = ctk.CTkFrame(frame_controles)
            frame_controles_especificos.pack(fill="x", padx=10, pady=10)

            if user_is_admin:
                frame_botoes = frame_controles_especificos
                for i in range(10):
                    frame_botoes.grid_columnconfigure(i, weight=1)

                ctk.CTkButton(
                    frame_botoes,
                    text="🟢 Ativar Aluno",
                    command=self.ativar_aluno,
                    fg_color="#3A8A3A",
                ).grid(row=0, column=0, padx=5, pady=5, sticky="ew")
                ctk.CTkButton(
                    frame_botoes,
                    text="🔴 Desativar Aluno",
                    command=self.desativar_aluno,
                    fg_color="#E03C31",
                ).grid(row=0, column=1, padx=5, pady=5, sticky="ew")
                ctk.CTkButton(
                    frame_botoes,
                    text="✏️ Editar Usuário",
                    command=self.abrir_janela_edicao,
                ).grid(row=0, column=2, padx=5, pady=5, sticky="ew")
                ctk.CTkButton(
                    frame_botoes,
                    text="❌ Excluir Usuário",
                    command=self.excluir_usuario,
                    fg_color="#CC0000",
                ).grid(row=0, column=3, padx=5, pady=5, sticky="ew")
                ctk.CTkButton(
                    frame_botoes,
                    text="➕ Adicionar Usuário",
                    command=self.abrir_janela_novo_usuario,
                    fg_color="#007BFF",
                ).grid(row=0, column=4, padx=5, pady=5, sticky="ew")
                ctk.CTkButton(
                    frame_botoes,
                    text="💾 Salvar Alterações",
                    command=self.salvar_dados,
                    fg_color="#3C66E0",
                ).grid(row=0, column=5, padx=5, pady=5, sticky="ew")
                ctk.CTkLabel(
                    frame_botoes, text="| Filtros Rápidos:", font=("Arial", 12, "bold")
                ).grid(row=0, column=6, padx=10)
                ctk.CTkButton(
                    frame_botoes,
                    text="Mostrar Ativos",
                    command=lambda: self.filtrar_por_status("ATIVO"),
                    fg_color="#4CAF50",
                ).grid(row=0, column=7, padx=5, pady=5, sticky="ew")
                ctk.CTkButton(
                    frame_botoes,
                    text="Mostrar Inativos",
                    command=lambda: self.filtrar_por_status("INATIVO"),
                    fg_color="#FF9800",
                ).grid(row=0, column=8, padx=5, pady=5, sticky="ew")
                ctk.CTkButton(
                    frame_botoes,
                    text="🔄 Atualizar Arquivo",
                    command=lambda: self.atualizar_tabela(reload_csv=True),
                ).grid(row=0, column=9, padx=5, pady=5, sticky="ew")

                filtros_container = ctk.CTkFrame(frame_controles)
                filtros_container.pack(fill="x", padx=10, pady=5)
                self.criar_widgets_filtro(filtros_container)

            elif user_is_coordinator:
                frame_botoes_coord = frame_controles_especificos
                for i in range(5):
                    frame_botoes_coord.grid_columnconfigure(i, weight=1)

                ctk.CTkButton(
                    frame_botoes_coord,
                    text="🟢 Ativar Aluno",
                    command=self.ativar_aluno,
                    fg_color="#3A8A3A",
                ).grid(row=0, column=0, padx=5, pady=5, sticky="ew")
                ctk.CTkButton(
                    frame_botoes_coord,
                    text="🔴 Desativar Aluno",
                    command=self.desativar_aluno,
                    fg_color="#E03C31",
                ).grid(row=0, column=1, padx=5, pady=5, sticky="ew")
                ctk.CTkButton(
                    frame_botoes_coord,
                    text="➕ Adicionar Usuário",
                    command=self.abrir_janela_novo_usuario,
                    fg_color="#007BFF",
                ).grid(row=0, column=2, padx=5, pady=5, sticky="ew")
                ctk.CTkButton(
                    frame_botoes_coord,
                    text="💾 Salvar Alterações",
                    command=self.salvar_dados,
                    fg_color="#3C66E0",
                ).grid(row=0, column=3, padx=5, pady=5, sticky="ew")
                ctk.CTkButton(
                    frame_botoes_coord,
                    text="🔄 Atualzar Arquivo",
                    command=lambda: self.atualizar_tabela(reload_csv=True),
                ).grid(row=0, column=4, padx=5, pady=5, sticky="ew")

                filtros_container_coord = ctk.CTkFrame(frame_controles)
                filtros_container_coord.pack(fill="x", padx=10, pady=5)
                self.criar_widgets_filtro(filtros_container_coord)

            elif user_is_professor:
                frame_botoes_prof = frame_controles_especificos
                frame_botoes_prof.grid_columnconfigure(0, weight=1)
                frame_botoes_prof.grid_columnconfigure(1, weight=1)

                ctk.CTkButton(
                    frame_botoes_prof,
                    text="✏️ Lançar/Editar Notas de Aluno",
                    command=self.abrir_janela_edicao_notas,
                    fg_color="#007BFF",
                ).grid(row=0, column=0, padx=5, pady=5, sticky="ew")
                ctk.CTkButton(
                    frame_botoes_prof,
                    text="🔄 Recarregar Dados",
                    command=lambda: self.atualizar_tabela(reload_csv=True),
                ).grid(row=0, column=1, padx=5, pady=5, sticky="ew")

                filtros_container_prof = ctk.CTkFrame(frame_controles)
                filtros_container_prof.pack(fill="x", padx=10, pady=5)
                self.criar_widgets_filtro(filtros_container_prof)

        self.frame_tabela_dados = ctk.CTkFrame(frame_tabela)
        self.frame_tabela_dados.pack(fill="both", expand=True, padx=10, pady=10)

    # --- MÉTODO CORRIGIDO (abrir_janela_meus_dados) ---
    def abrir_janela_meus_dados(self):
        """Abre uma janela mostrando os dados do usuário logado, independentemente do nível."""

        user_data = self.current_user

        if user_data is None:
            messagebox.showwarning("Aviso", "Nenhum usuário logado.")
            return

        dados_window = ctk.CTkToplevel(self)
        dados_window.title("Meus Dados Cadastrais")
        # Tamanho ajustado para acomodar as notas
        dados_window.geometry("450x450")
        dados_window.transient(self)
        dados_window.grab_set()

        form_frame = ctk.CTkFrame(dados_window)
        form_frame.pack(padx=20, pady=20, fill="both", expand=True)

        is_notes_user = user_data.get("NIVEL") != "COORDENADOR"

        dados_para_exibir = [
            ("ID", "ID"),
            ("NOME", "Nome Completo"),
            ("EMAIL", "Email"),
            ("IDADE", "Idade"),
            ("NIVEL", "Nível de Acesso"),
            ("CURSO", "Curso"),
            ("ID_TURMAS", "Turma"),
            ("STATUS DO ALUNO", "Status da Matrícula"),
        ]

        if is_notes_user:
            dados_para_exibir.extend(
                [
                    ("NP1", "NP1"),
                    ("NP2", "NP2"),
                    ("PIM", "PIM"),
                    ("MEDIA", "Média Final"),
                ]
            )

        for i, (key, label_text) in enumerate(dados_para_exibir):

            label = ctk.CTkLabel(
                form_frame, text=f"{label_text}:", font=("Arial", 12, "bold")
            )
            label.grid(row=i, column=0, padx=10, pady=8, sticky="e")

            valor = user_data.get(key)
            valor_display = "N/A"

            if key == "STATUS DO ALUNO":
                valor_display = str(valor).upper() if valor else "N/A"
            elif key in ["NP1", "NP2", "PIM", "MEDIA"]:
                try:
                    valor_display = f"{float(valor):.2f}"
                except (ValueError, TypeError):
                    valor_display = str(valor)
            else:
                valor_display = str(valor)

            value_label = ctk.CTkLabel(
                form_frame, text=valor_display, text_color="gray"
            )
            value_label.grid(row=i, column=1, padx=10, pady=8, sticky="w")

        form_frame.grid_columnconfigure(0, weight=1)
        form_frame.grid_columnconfigure(1, weight=2)

    # --- FIM DO MÉTODO CORRIGIDO ---

    def criar_widgets_filtro(self, master_frame):
        ctk.CTkLabel(master_frame, text="Filtrar por:").grid(
            row=0, column=0, padx=(10, 5), pady=10, sticky="w"
        )
        colunas_filtro = ["Filtrar por Coluna..."]
        if self.data_frame_full is not None and not self.data_frame_full.empty:
            lista_completa_colunas = self.data_frame_full.columns.tolist()
            colunas_filtradas = [
                coluna for coluna in lista_completa_colunas if coluna != "SENHA"
            ]
            colunas_filtro.extend(colunas_filtradas)
        self.combo_filtro_coluna = ctk.CTkComboBox(
            master_frame, values=colunas_filtro, width=180
        )
        self.combo_filtro_coluna.grid(row=0, column=1, padx=5, pady=10, sticky="w")
        self.entrada_filtro_geral = ctk.CTkEntry(
            master_frame, placeholder_text="Digite o valor...", width=180
        )
        self.entrada_filtro_geral.grid(row=0, column=2, padx=5, pady=10, sticky="w")
        ctk.CTkButton(
            master_frame, text="Buscar", command=self.filtrar_geral, width=80
        ).grid(row=0, column=3, padx=5, pady=10, sticky="w")
        ctk.CTkButton(
            master_frame, text="Limpar Filtros", command=self.limpar_filtros
        ).grid(row=0, column=4, padx=(20, 10), pady=10, sticky="w")

    def excluir_usuario(self):
        if self.current_user["NIVEL"] != "ADMINISTRADOR":
            messagebox.showwarning(
                "Permissão Negada", "Somente usuários ADMIN podem excluir usuários."
            )
            return
        if self.tabela_widget is None:
            return
        selecionado = self.tabela_widget.focus()
        if not selecionado:
            messagebox.showwarning("Seleção", "Selecione uma linha para excluir.")
            return
        try:
            idx_visible = int(selecionado)
            user_id = self.data_frame.loc[idx_visible, "ID"]
            user_nome = self.data_frame.loc[idx_visible, "NOME"]
            if not messagebox.askyesno(
                "Confirmação de Exclusão",
                f"Tem certeza que deseja EXCLUIR o usuário ID {user_id} ({user_nome})? Esta ação é permanente.",
            ):
                return
            idx_full_list = self.data_frame_full[
                self.data_frame_full["ID"] == user_id
            ].index
            if not idx_full_list.empty:
                self.data_frame_full.drop(idx_full_list, inplace=True)
                self.salvar_dados()
                messagebox.showinfo(
                    "Sucesso", f"Usuário ID {user_id} excluído e arquivo CSV salvo."
                )
            else:
                messagebox.showwarning(
                    "Erro",
                    f"Usuário ID {user_id} não encontrado no banco de dados completo.",
                )
        except Exception as e:
            messagebox.showerror(
                "Erro de Exclusão", f"Ocorreu um erro ao excluir o usuário: {e}"
            )

    def filtrar_por_status(self, status):
        if self.current_user["NIVEL"] in ["ADMINISTRADOR", "COORDENADOR", "PROFESSOR"]:
            self.atualizar_tabela(reload_csv=False, filter_status=status)
        else:
            messagebox.showwarning(
                "Permissão Negada",
                "Filtros rápidos são exclusivos para Admin, Coordenador ou Professor.",
            )

    def filtrar_geral(self):
        if self.current_user["NIVEL"] not in [
            "ADMINISTRADOR",
            "COORDENADOR",
            "PROFESSOR",
        ]:
            messagebox.showwarning(
                "Permissão Negada",
                "Filtros são exclusivos para Admin, Coordenador ou Professor.",
            )
            return
        coluna = self.combo_filtro_coluna.get()
        texto = self.entrada_filtro_geral.get().strip()
        if coluna == "Filtrar por Coluna..." or not texto:
            messagebox.showwarning(
                "Filtro", "Selecione uma coluna e digite um valor de busca."
            )
            return
        self.atualizar_tabela(
            reload_csv=False, general_filter_text=texto, filter_column=coluna
        )

    def limpar_filtros(self):
        if self.current_user["NIVEL"] in ["ADMINISTRADOR", "COORDENADOR", "PROFESSOR"]:
            self.combo_filtro_coluna.set("Filtrar por Coluna...")
            self.entrada_filtro_geral.delete(0, tk.END)
            self.atualizar_tabela(reload_csv=False)
        else:
            messagebox.showwarning(
                "Permissão Negada",
                "Limpar filtros é exclusivo para Admin, Coordenador ou Professor.",
            )

    def atualizar_tabela(
        self,
        reload_csv=True,
        filter_status=None,
        general_filter_text=None,
        filter_column=None,
    ):
        if reload_csv:
            self.data_frame_full = carregar_tabela(CAMINHO_ARQUIVO)
        if self.data_frame_full is None or self.data_frame_full.empty:
            self.data_frame = pd.DataFrame()
            self.mostrar_tabela(self.data_frame)
            return

        df_display = self.data_frame_full.copy()
        user_level = self.current_user["NIVEL"]
        columns_to_drop = []
        rows_filter = None

        if user_level == "ALUNO":
            user_id = self.current_user["ID"]
            rows_filter = df_display["ID"] == user_id
            columns_to_drop.extend(
                ["STATUS DO ALUNO", "SENHA", "ID_TURMAS", "EMAIL", "IDADE", "NIVEL"]
            )
        elif user_level == "PROFESSOR":
            rows_filter = df_display["NIVEL"] == "ALUNO"
            columns_to_drop.append("SENHA")
            columns_to_drop.append("ID_TURMAS")
        elif user_level == "COORDENADOR":
            rows_filter = df_display["NIVEL"].isin(
                ["ALUNO", "PROFESSOR", "COORDENADOR"]
            )
            columns_to_drop.append("SENHA")
            columns_to_drop.extend(["NP1", "NP2", "PIM"])
            columns_to_drop.append("ID_TURMAS")
        elif user_level == "ADMINISTRADOR":
            columns_to_drop.append("SENHA")

        if rows_filter is not None:
            df_display = df_display[rows_filter].copy()

        cols_to_drop_final = [
            col for col in columns_to_drop if col in df_display.columns
        ]
        if cols_to_drop_final:
            df_display = df_display.drop(columns=cols_to_drop_final)

        if user_level in ["ADMINISTRADOR", "COORDENADOR", "PROFESSOR"]:
            if filter_status:
                df_display = df_display[
                    df_display["STATUS DO ALUNO"] == filter_status.upper()
                ]
            if (
                general_filter_text
                and filter_column
                and filter_column in df_display.columns
            ):
                search_term = general_filter_text.lower()
                df_display = df_display[
                    df_display[filter_column]
                    .astype(str)
                    .str.lower()
                    .str.contains(search_term, na=False)
                ]

        self.data_frame = df_display.reset_index(drop=True)
        self.mostrar_tabela(self.data_frame)

        if reload_csv and user_level in ["ADMINISTRADOR", "COORDENADOR", "PROFESSOR"]:
            messagebox.showinfo(
                "Atualização", "Tabela recarregada a partir do arquivo CSV."
            )

    def mostrar_tabela(self, df):
        for widget in self.frame_tabela_dados.winfo_children():
            widget.destroy()

        if df.empty:
            ctk.CTkLabel(
                self.frame_tabela_dados,
                text="Nenhum dado encontrado para o filtro aplicado ou seu nível de acesso.",
                text_color="red",
            ).pack(pady=20)
            self.tabela_widget = None
            return

        colunas = list(df.columns)
        self.tabela_widget = ttk.Treeview(
            self.frame_tabela_dados, columns=colunas, show="headings"
        )
        vsb = ttk.Scrollbar(
            self.frame_tabela_dados, orient="vertical", command=self.tabela_widget.yview
        )
        hsb = ttk.Scrollbar(
            self.frame_tabela_dados,
            orient="horizontal",
            command=self.tabela_widget.xview,
        )
        self.tabela_widget.configure(yscrollcommand=vsb.set, xscrollcommand=hsb.set)
        vsb.pack(side="right", fill="y")
        hsb.pack(side="bottom", fill="x")
        self.tabela_widget.column("#0", width=0, stretch=tk.NO)
        user_level = self.current_user.get("NIVEL", "N/A")
        self.tabela_widget.tag_configure(
            "aprovado", foreground="#00B050", font=("Arial", 10, "bold")
        )
        self.tabela_widget.tag_configure(
            "reprovado", foreground="#E03C31", font=("Arial", 10, "bold")
        )

        for col in colunas:
            col_display = "TURMA" if col == "ID_TURMAS" else col.replace("_", " ")
            self.tabela_widget.heading(col, text=col_display)
            col_width = 100
            if col in ["NOME", "EMAIL", "CURSO"]:
                col_width = 180
            elif col in ["ID_TURMAS"]:
                col_width = 150
            elif col in ["NP1", "NP2", "PIM", "MEDIA", "ID", "IDADE"]:
                col_width = 80
            self.tabela_widget.column(
                col, anchor="center", width=col_width, minwidth=50
            )

        for i, row in df.iterrows():
            row_list = list(row)
            tag_para_aplicar = ()
            if "MEDIA" in colunas:
                media_index = colunas.index("MEDIA")
                try:
                    media_valor = float(row_list[media_index])
                    row_list[media_index] = f"{media_valor:.2f}"
                    if user_level == "ALUNO":
                        tag_para_aplicar = (
                            ("aprovado",) if media_valor >= 7.0 else ("reprovado",)
                        )
                except (ValueError, TypeError):
                    pass
            self.tabela_widget.insert(
                "", "end", iid=i, values=row_list, tags=tag_para_aplicar
            )

        self.tabela_widget.pack(fill="both", expand=True)

    def ativar_aluno(self):
        if self.current_user["NIVEL"] not in ["ADMINISTRADOR", "COORDENADOR"]:
            messagebox.showwarning("Permissão Negada", "Acesso negado.")
            return
        if self.tabela_widget is None:
            return
        selecionado = self.tabela_widget.focus()
        if selecionado:
            try:
                idx_visible = int(selecionado)
                user_id = self.data_frame.loc[idx_visible, "ID"]
                idx_full_list = self.data_frame_full[
                    self.data_frame_full["ID"] == user_id
                ].index
                if not idx_full_list.empty:
                    idx_full = idx_full_list[0]
                    self.data_frame_full.loc[idx_full, "STATUS DO ALUNO"] = "ATIVO"
                    if "STATUS DO ALUNO" in self.data_frame.columns:
                        self.data_frame.loc[idx_visible, "STATUS DO ALUNO"] = "ATIVO"
                    current_values = list(self.data_frame.loc[idx_visible].values)
                    if "MEDIA" in self.data_frame.columns:
                        media_index = self.data_frame.columns.tolist().index("MEDIA")
                        current_values[media_index] = (
                            f"{float(current_values[media_index]):.2f}"
                        )
                    self.tabela_widget.item(selecionado, values=current_values)
                    messagebox.showinfo(
                        "Status",
                        f"Aluno ID {user_id} ativado. Lembre-se de SALVAR as alterações.",
                    )
                else:
                    messagebox.showwarning(
                        "Erro",
                        f"Não foi possível encontrar o usuário ID {user_id} no banco de dados completo.",
                    )
            except Exception as e:
                messagebox.showerror("Erro", f"Ocorreu um erro ao ativar: {e}")
        else:
            messagebox.showwarning("Seleção", "Selecione uma linha para ativar.")

    def desativar_aluno(self):
        if self.current_user["NIVEL"] not in ["ADMINISTRADOR", "COORDENADOR"]:
            messagebox.showwarning("Permissão Negada", "Acesso negado.")
            return
        if self.tabela_widget is None:
            return
        selecionado = self.tabela_widget.focus()
        if selecionado:
            try:
                idx_visible = int(selecionado)
                user_id = self.data_frame.loc[idx_visible, "ID"]
                idx_full_list = self.data_frame_full[
                    self.data_frame_full["ID"] == user_id
                ].index
                if not idx_full_list.empty:
                    idx_full = idx_full_list[0]
                    self.data_frame_full.loc[idx_full, "STATUS DO ALUNO"] = "INATIVO"
                    if "STATUS DO ALUNO" in self.data_frame.columns:
                        self.data_frame.loc[idx_visible, "STATUS DO ALUNO"] = "INATIVO"
                    current_values = list(self.data_frame.loc[idx_visible].values)
                    if "MEDIA" in self.data_frame.columns:
                        media_index = self.data_frame.columns.tolist().index("MEDIA")
                        current_values[media_index] = (
                            f"{float(current_values[media_index]):.2f}"
                        )
                    self.tabela_widget.item(selecionado, values=current_values)
                    messagebox.showinfo(
                        "Status",
                        f"Aluno ID {user_id} desativado. Lembre-se de SALVAR as alterações.",
                    )
                else:
                    messagebox.showwarning(
                        "Erro",
                        f"Não foi possível encontrar o usuário ID {user_id} no banco de dados completo.",
                    )
            except Exception as e:
                messagebox.showerror("Erro", f"Ocorreu um erro ao desativar: {e}")
        else:
            messagebox.showwarning("Seleção", "Selecione uma linha para desativar.")

    def abrir_janela_novo_usuario(self):
        if self.current_user["NIVEL"] not in ["ADMINISTRADOR", "COORDENADOR"]:
            messagebox.showwarning("Permissão Negada", "Acesso negado.")
            return
        add_window = ctk.CTkToplevel(self)
        add_window.title("Adicionar Novo Usuário")
        add_window.geometry("400x600")
        add_window.transient(self)
        add_window.grab_set()
        form_frame = ctk.CTkFrame(add_window)
        form_frame.pack(padx=20, pady=20, fill="both", expand=True)
        entries = {}
        editaveis = [
            "NOME",
            "EMAIL",
            "SENHA",
            "IDADE",
            "CURSO",
            "ID_TURMAS",
            "NIVEL",
            "NP1",
            "NP2",
            "PIM",
        ]
        for i, col in enumerate(editaveis):
            label_text = "TURMA" if col == "ID_TURMAS" else col.replace("_", " ")
            ctk.CTkLabel(form_frame, text=f"{label_text}:").grid(
                row=i, column=0, padx=10, pady=5, sticky="w"
            )
            entry = ctk.CTkEntry(form_frame, width=250)
            if col in ["NP1", "NP2", "PIM", "IDADE"]:
                entry.insert(0, "0")
            elif col == "NIVEL":
                entry.insert(0, "ALUNO")
            entry.grid(row=i, column=1, padx=10, pady=5, sticky="ew")
            entries[col] = entry
        ctk.CTkButton(
            form_frame,
            text="Adicionar e Salvar Usuário",
            command=lambda: self.salvar_novo_usuario(entries, add_window),
            fg_color="green",
        ).grid(
            row=len(editaveis), column=0, columnspan=2, padx=10, pady=20, sticky="ew"
        )

    def salvar_novo_usuario(self, entries, window):
        try:
            new_user_data = {}
            for col, entry_widget in entries.items():
                new_value = entry_widget.get().strip()
                if not new_value and col not in ["NP1", "NP2", "PIM", "IDADE"]:
                    messagebox.showerror(
                        "Erro de Validação", f"O campo '{col}' não pode estar vazio."
                    )
                    return
                if col in ["IDADE", "NP1", "NP2", "PIM"]:
                    try:
                        new_value_converted = 0 if not new_value else float(new_value)
                        if col in ["NP1", "NP2", "PIM"] and not (
                            0 <= new_value_converted <= 10
                        ):
                            messagebox.showerror(
                                "Erro", f"Notas devem estar entre 0 e 10."
                            )
                            return
                        new_user_data[col] = new_value_converted
                    except ValueError:
                        messagebox.showerror(
                            "Erro", f"O campo '{col}' deve ser um número."
                        )
                        return
                elif col in ["NOME", "NIVEL", "CURSO"]:
                    new_user_data[col] = new_value.upper()
                elif col == "EMAIL":
                    new_user_data[col] = new_value.lower()
                elif col == "ID_TURMAS":
                    new_user_data[col] = new_value
                elif col == "SENHA":
                    new_user_data[col] = new_value
            new_user_data["ID"] = self._gerar_novo_id()
            new_user_data["STATUS DO ALUNO"] = "ATIVO"
            np1 = new_user_data.get("NP1", 0)
            np2 = new_user_data.get("NP2", 0)
            pim = new_user_data.get("PIM", 0)
            media = (np1 * 4 + np2 * 4 + pim * 2) / 10
            new_user_data["MEDIA"] = round(media, 2)
            colunas_completas = self.data_frame_full.columns
            for col in colunas_completas:
                if col not in new_user_data:
                    new_user_data[col] = (
                        0 if col in ["IDADE", "NP1", "NP2", "PIM", "MEDIA"] else ""
                    )
            new_user_df = pd.DataFrame([new_user_data])
            new_user_df = new_user_df[colunas_completas]
            self.data_frame_full = pd.concat(
                [self.data_frame_full, new_user_df], ignore_index=True
            )
            self.salvar_dados()
            messagebox.showinfo(
                "Sucesso",
                f"Novo usuário '{new_user_data['NOME']}' (ID: {new_user_data['ID']}) adicionado com sucesso!",
            )
            window.destroy()
        except Exception as e:
            messagebox.showerror("Erro ao Salvar Novo Usuário", f"Ocorreu um erro: {e}")

    def abrir_janela_edicao(self):
        if self.current_user["NIVEL"] != "ADMINISTRADOR":
            messagebox.showwarning("Permissão Negada", "Acesso negado.")
            return
        if not isinstance(self.tabela_widget, ttk.Treeview):
            messagebox.showwarning("Aviso", "A tabela não está carregada.")
            return
        selecionado = self.tabela_widget.focus()
        if not selecionado:
            messagebox.showwarning("Seleção", "Selecione um usuário para editar.")
            return
        idx = int(selecionado)
        user_data_series = self.data_frame.loc[idx]
        user_id = user_data_series["ID"]
        full_user_data_row = self.data_frame_full[self.data_frame_full["ID"] == user_id]
        if full_user_data_row.empty:
            messagebox.showerror("Erro", "Dados completos do usuário não encontrados.")
            return
        full_user_data = full_user_data_row.iloc[0]
        edit_window = ctk.CTkToplevel(self)
        edit_window.title(f"Editar Usuário ID: {user_id}")
        edit_window.geometry("400x600")
        edit_window.transient(self)
        edit_window.grab_set()
        form_frame = ctk.CTkFrame(edit_window)
        form_frame.pack(padx=20, pady=20, fill="both", expand=True)
        entries = {}
        editaveis = [
            "NOME",
            "EMAIL",
            "IDADE",
            "CURSO",
            "ID_TURMAS",
            "NIVEL",
            "SENHA",
            "NP1",
            "NP2",
            "PIM",
        ]
        for i, col in enumerate(editaveis):
            if col in full_user_data.index:
                label_text = "TURMA" if col == "ID_TURMAS" else col.replace("_", " ")
                ctk.CTkLabel(form_frame, text=f"{label_text}:").grid(
                    row=i, column=0, padx=10, pady=5, sticky="w"
                )
                entry = ctk.CTkEntry(form_frame, width=250)
                entry.insert(0, str(full_user_data[col]))
                entry.grid(row=i, column=1, padx=10, pady=5, sticky="ew")
                entries[col] = entry
        ctk.CTkButton(
            form_frame,
            text="Salvar Alterações",
            command=lambda: self.salvar_edicao_usuario(user_id, entries, edit_window),
        ).grid(
            row=len(editaveis), column=0, columnspan=2, padx=10, pady=20, sticky="ew"
        )

    def salvar_edicao_usuario(self, user_id, entries, window):
        try:
            user_index = self.data_frame_full[
                self.data_frame_full["ID"] == user_id
            ].index[0]
            for col, entry_widget in entries.items():
                new_value = entry_widget.get().strip()
                if col in ["IDADE", "NP1", "NP2", "PIM"]:
                    try:
                        new_value_converted = 0 if not new_value else float(new_value)
                        if col in ["NP1", "NP2", "PIM"] and not (
                            0 <= new_value_converted <= 10
                        ):
                            messagebox.showerror(
                                "Erro", f"Notas devem estar entre 0 e 10."
                            )
                            return
                        self.data_frame_full.loc[user_index, col] = new_value_converted
                    except ValueError:
                        messagebox.showerror(
                            "Erro", f"O campo '{col}' deve ser um número."
                        )
                        return
                elif col in ["NOME", "NIVEL", "CURSO"]:
                    self.data_frame_full.loc[user_index, col] = new_value.upper()
                elif col == "EMAIL":
                    self.data_frame_full.loc[user_index, col] = new_value.lower()
                elif col == "ID_TURMAS":
                    self.data_frame_full.loc[user_index, col] = new_value
                elif col == "SENHA":
                    self.data_frame_full.loc[user_index, col] = new_value
            if all(
                col in self.data_frame_full.columns for col in ["NP1", "NP2", "PIM"]
            ):
                row = self.data_frame_full.loc[user_index]
                media = (row["NP1"] * 4 + row["NP2"] * 4 + row["PIM"] * 2) / 10
                self.data_frame_full.loc[user_index, "MEDIA"] = media
            self.salvar_dados()
            messagebox.showinfo("Sucesso", "Usuário atualizado com sucesso!")
            window.destroy()
        except Exception as e:
            messagebox.showerror(
                "Erro ao Salvar", f"Ocorreu um erro ao salvar as alterações: {e}"
            )

    def abrir_janela_edicao_notas(self):
        if self.current_user["NIVEL"] != "PROFESSOR":
            messagebox.showwarning("Permissão Negada", "Acesso negado.")
            return
        if not isinstance(self.tabela_widget, ttk.Treeview):
            messagebox.showwarning("Aviso", "A tabela não está carregada.")
            return
        selecionado = self.tabela_widget.focus()
        if not selecionado:
            messagebox.showwarning(
                "Seleção", "Selecione um ALUNO para editar as notas."
            )
            return
        idx = int(selecionado)
        user_data_series = self.data_frame.loc[idx]
        if user_data_series["NIVEL"] != "ALUNO":
            messagebox.showwarning(
                "Ação Inválida",
                "Você só pode editar as notas de usuários do nível ALUNO.",
            )
            return
        user_id = user_data_series["ID"]
        full_user_data_row = self.data_frame_full[self.data_frame_full["ID"] == user_id]
        if full_user_data_row.empty:
            messagebox.showerror("Erro", "Dados completos do usuário não encontrados.")
            return
        full_user_data = full_user_data_row.iloc[0]
        edit_window = ctk.CTkToplevel(self)
        edit_window.title(f"Lançar Notas - Aluno ID: {user_id}")
        edit_window.geometry("400x300")
        edit_window.transient(self)
        edit_window.grab_set()
        form_frame = ctk.CTkFrame(edit_window)
        form_frame.pack(padx=20, pady=20, fill="both", expand=True)
        ctk.CTkLabel(form_frame, text="Aluno:").grid(
            row=0, column=0, padx=10, pady=5, sticky="w"
        )
        ctk.CTkLabel(
            form_frame, text=f"{full_user_data['NOME']}", font=("Arial", 12, "bold")
        ).grid(row=0, column=1, padx=10, pady=5, sticky="w")
        entries = {}
        editaveis = ["NP1", "NP2", "PIM"]
        for i, col in enumerate(editaveis):
            ctk.CTkLabel(form_frame, text=f"{col}:").grid(
                row=i + 1, column=0, padx=10, pady=5, sticky="w"
            )
            entry = ctk.CTkEntry(form_frame, width=250)
            entry.insert(0, str(full_user_data[col]))
            entry.grid(row=i + 1, column=1, padx=10, pady=5, sticky="ew")
            entries[col] = entry
        ctk.CTkButton(
            form_frame,
            text="Salvar Notas",
            command=lambda: self.salvar_edicao_notas(user_id, entries, edit_window),
            fg_color="green",
        ).grid(
            row=len(editaveis) + 1,
            column=0,
            columnspan=2,
            padx=10,
            pady=20,
            sticky="ew",
        )

    def salvar_edicao_notas(self, user_id, entries, window):
        try:
            user_index = self.data_frame_full[
                self.data_frame_full["ID"] == user_id
            ].index[0]
            for col, entry_widget in entries.items():
                new_value = entry_widget.get().strip()
                try:
                    new_value_converted = 0 if not new_value else float(new_value)
                    if not (0 <= new_value_converted <= 10):
                        messagebox.showerror(
                            "Erro de Validação",
                            f"A nota {col} deve estar entre 0 e 10.",
                        )
                        return
                    self.data_frame_full.loc[user_index, col] = new_value_converted
                except ValueError:
                    messagebox.showerror(
                        "Erro de Validação",
                        f"O campo '{col}' deve ser um número (use . para decimais).",
                    )
                    return
            row = self.data_frame_full.loc[user_index]
            media = (row["NP1"] * 4 + row["NP2"] * 4 + row["PIM"] * 2) / 10
            self.data_frame_full.loc[user_index, "MEDIA"] = round(media, 2)
            self.salvar_dados()
            messagebox.showinfo("Sucesso", "Notas do aluno atualizadas com sucesso!")
            window.destroy()
        except Exception as e:
            messagebox.showerror(
                "Erro ao Salvar", f"Ocorreu um erro ao salvar as notas: {e}"
            )

    def open_activity_submission_window(self):
        user_name = self.current_user.get("NOME", "N/A")
        user_id = self.current_user.get("ID", "N/A")
        submission_window = ctk.CTkToplevel(self)
        submission_window.title("Envio de Atividades do Aluno")
        submission_window.geometry("500x350")
        submission_window.transient(self)
        submission_window.grab_set()
        frame = ctk.CTkFrame(submission_window)
        frame.pack(padx=20, pady=20, fill="both", expand=True)
        ctk.CTkLabel(
            frame, text="Portal de Envio de Atividades", font=("Roboto", 18, "bold")
        ).pack(pady=10)
        ctk.CTkLabel(
            frame, text=f"Aluno: {user_name} (ID: {user_id})", text_color="#4CAF50"
        ).pack(pady=(5, 10))
        ctk.CTkLabel(frame, text="Disciplina:").pack(pady=(10, 0))

        self.discipline_entry = ctk.CTkEntry(frame, width=300)
        self.discipline_entry.pack(pady=(0, 10))
        self.current_file_path = None
        self.filepath_label = ctk.CTkLabel(
            frame, text="Nenhum arquivo selecionado.", text_color="gray"
        )
        self.filepath_label.pack(pady=(5, 0))

        ctk.CTkButton(
            frame, text="Anexar Arquivo...", command=self._anexar_arquivo_dialog
        ).pack(pady=10)
        ctk.CTkButton(
            frame,
            text="Enviar Atividade",
            command=lambda: self._enviar_atividade_action(submission_window),
            fg_color="green",
        ).pack(pady=20)

    def _anexar_arquivo_dialog(self):
        filepath = filedialog.askopenfilename(
            title="Selecione o arquivo da atividade",
            filetypes=(
                ("Todos os Arquivos", "*.*"),
                ("Documentos", "*.pdf;*.docx;*.txt"),
            ),
        )
        if filepath:
            self.filepath_label.configure(
                text=f"Arquivo: {os.path.basename(filepath)}", text_color="yellow"
            )
            self.current_file_path = filepath
        else:
            self.filepath_label.configure(
                text="Nenhum arquivo selecionado.", text_color="gray"
            )
            self.current_file_path = None

    def _enviar_atividade_action(self, window):
        disciplina = self.discipline_entry.get().strip()
        if not disciplina:
            messagebox.showwarning("Aviso", "Por favor, digite o nome da disciplina.")
            return
        if not hasattr(self, "current_file_path") or not self.current_file_path:
            messagebox.showwarning("Aviso", "Por favor, anexe o arquivo da atividade.")
            return
        messagebox.showinfo(
            "Envio Concluído",
            f"Atividade de '{disciplina}' enviada com sucesso (Simulação) pelo aluno ID: {self.current_user['ID']}!",
        )
        window.destroy()

    def salvar_dados(self):
        if self.current_user["NIVEL"] not in [
            "ADMINISTRADOR",
            "COORDENADOR",
            "PROFESSOR",
        ]:
            messagebox.showwarning(
                "Permissão Negada",
                "Somente Admin, Coordenador ou Professor (para salvar notas) podem salvar alterações.",
            )
            return
        if self.data_frame_full is None or self.data_frame_full.empty:
            messagebox.showwarning(
                "Salvar",
                "Não foi possível salvar, o banco de dados está vazio ou não foi carregado.",
            )
            return
        try:
            df_to_save = self.data_frame_full.copy()
            column_map_save = {
                "ID": "id",
                "NOME": "nome",
                "EMAIL": "email",
                "SENHA": "senha",
                "IDADE": "idade",
                "NIVEL": "nivel",
                "CURSO": "curso",
                "ID_TURMAS": "turma",
                "NP1": "np1",
                "NP2": "np2",
                "PIM": "pim",
                "MEDIA": "media",
                "STATUS DO ALUNO": "atividade",
            }
            df_to_save.rename(columns=column_map_save, inplace=True)
            final_csv_cols = [
                "id",
                "nome",
                "email",
                "senha",
                "nivel",
                "curso",
                "turma",
                "idade",
                "np1",
                "np2",
                "pim",
                "media",
                "atividade",
            ]
            df_to_save = df_to_save.reindex(columns=final_csv_cols)
            for col in ["np1", "np2", "pim", "media"]:
                if col in df_to_save.columns:
                    df_to_save[col] = df_to_save[col].apply(
                        lambda x: (
                            f"{x:.2f}".replace(".", ",")
                            if pd.notna(x) and pd.notnull(x)
                            else ""
                        )
                    )
            Path(CAMINHO_ARQUIVO).parent.mkdir(parents=True, exist_ok=True)
            with open(CAMINHO_ARQUIVO, "w", encoding="utf-8", newline="") as f:
                f.write("[USUARIOS]\n")
                df_to_save.to_csv(
                    f, sep=";", index=False, header=True, lineterminator="\r\n"
                )
            messagebox.showinfo(
                "Salvo",
                "Todas as alterações (Status e Edições) foram salvas com sucesso no arquivo CSV.",
            )
            self.atualizar_tabela(reload_csv=True)
            for widget in self.abas.winfo_children():
                if isinstance(
                    widget, ctk.CTkFrame
                ) and "Gerenciar Turmas" in self.abas.tab(widget, "text"):
                    for child in widget.winfo_children():
                        if isinstance(child, GerenciarTurmasFrame):
                            child.atualizar_tabela_turmas()
                            break
        except Exception as e:
            messagebox.showerror(
                "Erro ao Salvar", f"Ocorreu um erro ao salvar os dados: {e}"
            )


if __name__ == "__main__":
    Path("Output").mkdir(exist_ok=True)
    ctk.set_appearance_mode("Dark")
    ctk.set_default_color_theme("blue")

    app = MainApp()
    app.start_app_with_lgpd_check()
    app.mainloop()


# atualizado em 05/11 ás 23:12
