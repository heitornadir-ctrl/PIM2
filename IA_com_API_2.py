import tkinter as tk
from tkinter import ttk
import google.genai as genai
from google.genai import types # Importação necessária para GenerateContentConfig e errors
from google.genai import errors 
import os 
import threading # Necessário para evitar o congelamento da interface

# Configuração da chave da API Gemini (mantida como solicitado)
os.environ["GEMINI_API_KEY"] = "AIzaSyAn3fTYzK-8vLcMV4PNz5anyc7uerX7yw8"

# Base de conhecimento organizada como dicionário de perguntas e respostas
respostas = {
    "Quais cursos posso me matrícular?": "As aulas disponibilizadas para esse semestre são Educação Ambiental, Redes de Computadores, Banco de Dados, Inteligência Artificial, Ciberssegurança, Programação Orientada a Objetos, Python, Java, C / C++ e Análise e Projeto de Sistemas.",
    "Como calcular a média final?": "A média é calculada com média ponderada, onde cada prova tem peso 4 e o trabalho final tem peso 2. A fórmula é: (NP1 * 4 + NP2 * 4 + PIM * 2) / 10.",
    "Quais os horários que posso fazer as aulas?": "A partir do momento em que você se matricula em uma disciplina, tem um período de 6 meses para completar o curso.",
    "Quem é o coordenador geral?": "O coordenador geral é o Prof. Cordeiro, escolhido dentro da sua instituição.",
    "Qual é o prazo para entrega dos trabalhos?": "A data de entrega dos trabalhos é até o final do semestre.",
    "Qual é o conteúdo da aula de segunda-feira?": "Na segunda-feira, estudamos Programação Orientada a Objetos e Java.",
    "Qual é o conteúdo da aula de terça-feira?": "Na terça-feira, estudamos Educação Ambiental e C / C++.",
    "Qual é o conteúdo da aula de quarta-feira?": "Na quarta-feira, estudamos Redes de Computadores e Análise e Projeto de Sistemas.",
    "Qual é o conteúdo da aula de quinta-feira?": "Na quinta-feira, estudamos Banco de Dados e Ciberssegurança.",
    "Qual é o conteúdo da aula de sexta-feira?": "Na sexta-feira, estudamos Inteligência Artificial e Python.",
    "Como funciona a avaliação do curso?": "A avaliação são duas provas de 12 questões, sendo 10 alternativas e 2 dissertativas e um trabalho final.",
    "Quais são os horários de atendimento do coordenador?": "O Prof. Cordeiro atende às quartas, das 14h às 16h.",
    "Qual é o e-mail do coordenador?" : "O e-mail do Prof. Cordeiro é profcordeiro@unip.br.",
    "Como posso me matrícular?": "Você pode se matricular pelo número 129988556672 ou indo a secretaria da instituição, aberta das 8h às 12h de segunda a sexta."
}

# 1. Montar o contexto (System Prompt) a partir do seu dicionário 'respostas'
SYSTEM_PROMPT = """
Você é um Assistente Acadêmico útil e amigável. Use SOMENTE as informações a seguir
para responder às perguntas do usuário. Se a pergunta não puder ser respondida com
base neste contexto, diga educadamente que você não tem essa informação.

--- CONTEXTO ACADÊMICO ---
"""
for pergunta, resposta in respostas.items():
    SYSTEM_PROMPT += f"- Pergunta: {pergunta}\n  Resposta: {resposta}\n"
SYSTEM_PROMPT += "--------------------------"

class Chatbot:
    def __init__(self, root):
        self.root = root
        self.root.title("Assistente Acadêmico")
        self.root.geometry("700x500")
        self.root.configure(bg="#f0f0f0")
        
        # Inicializar o cliente Gemini
        try:
            self.client = genai.Client()
        except Exception as e:
            print(f"Erro ao inicializar o Gemini Client. Verifique sua GEMINI_API_KEY: {e}")
            self.client = None # Impede que o chatbot funcione sem a API

        # Configuração de geração (config) que inclui a instrução do sistema (SYSTEM_PROMPT)
        self.config = types.GenerateContentConfig(
            system_instruction=SYSTEM_PROMPT
        )

        self.criar_interface()
        self.mostrar_mensagem_boas_vindas()

    # Função para chamar a API em uma thread separada
    def _chamar_api_em_thread(self, pergunta):
        if not self.client:
            self.root.after(0, lambda: self.adicionar_mensagem("Assistente", "Erro de configuração da API. A chave do Gemini não foi carregada corretamente."))
            return

        try:
            # 3. Chamar a API, usando o modelo e injetando o contexto no parâmetro 'config'
            response = self.client.models.generate_content(
                model="gemini-2.5-flash", 
                contents=[
                    {"role": "user", "parts": [{"text": pergunta}]} # Apenas a pergunta do usuário no contents
                ],
                config=self.config # Passando a instrução do sistema
            )
            
            # 4. Processar a resposta da API
            resposta_texto = response.text
            self.root.after(0, lambda: self.adicionar_mensagem("Assistente", resposta_texto)) # Retorna a resposta para a thread principal

        except errors.APIError as e:
            # Trata erros específicos da API (autenticação, limite de taxa, etc.)
            self.root.after(0, lambda: self.adicionar_mensagem("Assistente", f"Erro da API (Verifique a chave/cota): {e}"))
        except Exception as e:
            # Trata outros erros (conexão, etc.)
            self.root.after(0, lambda: self.adicionar_mensagem("Assistente", f"Erro ao comunicar com a API: {e}"))
        finally:
            # 5. Reabilita a interface na thread principal, garantindo o feedback
            self.root.after(0, self._habilitar_interface)


    def _desabilitar_interface(self):
        """Desabilita entrada e botão enquanto a API processa."""
        self.entrada_usuario.config(state=tk.DISABLED)
        self.enviar_btn.config(state=tk.DISABLED, text="Processando...")

    def _habilitar_interface(self):
        """Habilita entrada e botão após o processamento da API."""
        self.entrada_usuario.config(state=tk.NORMAL)
        self.enviar_btn.config(state=tk.NORMAL, text="Enviar")
        self.entrada_usuario.focus()
        
    # 5. Modificar processar_entrada para usar threading e feedback visual
    def processar_entrada(self, event=None):
        pergunta = self.entrada_usuario.get()
        if not pergunta.strip():
            return 
        
        self.adicionar_mensagem("Você", pergunta)  
        self.entrada_usuario.delete(0, tk.END) 
        
        # Desabilita a interface antes de chamar a API
        self._desabilitar_interface()
        
        # Cria e inicia uma nova thread para a chamada da API
        api_thread = threading.Thread(target=self._chamar_api_em_thread, args=(pergunta,))
        api_thread.start()
        
    def criar_interface(self):
        # Frame principal
        main_frame = ttk.Frame(self.root, padding=10)
        main_frame.pack(fill=tk.BOTH, expand=True)
        
        # Título
        title_label = tk.Label(main_frame, 
                              text="Assistente Acadêmico", 
                              font=('Arial', 14, 'bold'),
                              fg='#2c3e50',
                              bg='#f0f0f0')
        title_label.pack(pady=(0, 10))
        
        # Frame do chat
        chat_frame = ttk.Frame(main_frame)
        chat_frame.pack(fill=tk.BOTH, expand=True, pady=(0, 10))
        
        # Área de conversa
        self.chat_area = tk.Text(chat_frame, 
                                wrap=tk.WORD, 
                                font=('Arial', 10),
                                bg='white',
                                fg='#2c3e50',
                                padx=10,
                                pady=10,
                                state=tk.DISABLED,
                                relief=tk.FLAT,
                                borderwidth=1)
        
        # Scrollbar
        scrollbar = ttk.Scrollbar(chat_frame, command=self.chat_area.yview)
        self.chat_area.configure(yscrollcommand=scrollbar.set)
        
        self.chat_area.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        
        # Frame de entrada
        input_frame = ttk.Frame(main_frame)
        input_frame.pack(fill=tk.X, pady=(0, 5))
        
        # Campo de entrada
        self.entrada_usuario = ttk.Entry(input_frame, font=('Arial', 10))
        self.entrada_usuario.pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 5))
        self.entrada_usuario.bind("<Return>", self.processar_entrada)
        
        # Botão de envio (agora é um atributo da classe)
        self.enviar_btn = ttk.Button(input_frame, 
                               text="Enviar", 
                               command=self.processar_entrada)
        self.enviar_btn.pack(side=tk.RIGHT)
        
        # Configurar tags para formatação de texto
        self.chat_area.tag_configure("assistant_name", foreground="#4a6fa5", font=('Arial', 10, 'bold'))
        self.chat_area.tag_configure("user_name", foreground="#2c3e50", font=('Arial', 10, 'bold'))
        
        # Focar no campo de entrada
        self.entrada_usuario.focus()
    
    def mostrar_mensagem_boas_vindas(self):
        self.adicionar_mensagem("Assistente", "Olá! Sou seu assistente acadêmico. Como posso ajudá-lo hoje? \t \n Você pode me perguntar sobre cursos, horários, avaliações e outros assuntos acadêmicos.")
    
    def adicionar_mensagem(self, sender, message):
        self.chat_area.config(state=tk.NORMAL)
        
        # Adicionar nome do remetente
        if sender == "Assistente":
            self.chat_area.insert(tk.END, "Assistente: ", "assistant_name")
        else:
            self.chat_area.insert(tk.END, "Você: ", "user_name")
        
        # Adicionar mensagem
        self.chat_area.insert(tk.END, message + "\n\n")
        
        self.chat_area.config(state=tk.DISABLED)
        self.chat_area.see(tk.END)

if __name__ == "__main__":
    root = tk.Tk()
    app = Chatbot(root)
    root.mainloop()