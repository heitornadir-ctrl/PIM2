import tkinter as tk 
from tkinter import ttk

# Um dicionário que contém as perguntas e respostas do chatbot.
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
    "Quais são os horários de atendimento do coordenador?": "O Prof. Cordeiro atende às quartas, das 14h às 16h."
}
# Aqui está a classe principal do Chatbot, onde é criada a interface gráfica e a lógica do chatbot.
class Chatbot:
    def __init__(self, root):
        self.root = root 
        self.root.title("Assistente Acadêmico")
        self.root.geometry("650x500")
        
        self.criar_interface()
        self.mostrar_mensagem_boas_vindas()
    
    def criar_interface(self):
        tk.Label(self.root, text="Assistente Acadêmico", font=('Arial', 14, 'bold')).pack(pady=10)
        # Cria um rótulo (label) que aparecerá na janela (root) com o texto "Assistente Acadêmico" e a fonte Arial, tamanho 14, negrito.
        # O método pack() é usado para adicionar o rótulo na interface gráfica.
        # O parâmetro pady=10 adiciona um espaçamento vertical de 10 pixels ao redor do rótulo.

        # Cria uma área de texto (Text) para exibir as mensagens do chatbot.
        # Com height=12 definindo a altura da área de texto em 12 linhas.
        # O parâmetro state=tk.DISABLED define que a área de texto está desabilitada, impossibilitando escrever nela.
        self.area_chat = tk.Text(self.root, height=12, state=tk.DISABLED)
        self.area_chat.pack(padx=10, pady=5, fill='both', expand=True)
        # O método pack() é usado para adicionar a área de texto na interface gráfica.
        # padx=10 adiciona um espaço horizontal de 10 pixels e pady=5 adiciona um espaçamento vertical de 5 pixels ao redor da área de texto.
        # O parâmetro fill='both' faz com que a área de texto possa se expandir para preencher o espaço disponível.
        # O parâmetro expand=True faz com que a área de texto possa se expandir caso redimensionada.

        # Cria um frame (quadro) para conter os botões.
        frame_botoes = ttk.Frame(self.root)
        frame_botoes.pack(padx=10, pady=10, fill='both')
        # Adiciona 10 pixels de largura e 10 pixels de altura ao redor do frame de botões.
        # fill='both' faz com que o frame de botões possam se expandir para preencher o espaço disponível.

        # Adiciona o texto como rótulo da área de botões, um em cada linha alinhados e usando 2 colunas, organizados à esquerda e espaçados verticalmente 5 pixels.
        tk.Label(frame_botoes, text="Perguntas Frequentes:", font=('Arial', 10, 'bold')).grid(row=0, column=0, columnspan=2, sticky="w", pady=5)
        
        # Criar botões automaticamente com base nas perguntas do dicionário respostas, e os organiza em 2 colunas, que permite expansão da esquerda para direita.
        for i, pergunta in enumerate(respostas.keys()):
            btn = ttk.Button(frame_botoes, text=pergunta, command=lambda p=pergunta: self.fazer_pergunta(p))
            btn.grid(row=(i//2)+1, column=i%2, padx=5, pady=2, sticky="ew")
        
        # Configurar expansão das colunas
        frame_botoes.columnconfigure(0, weight=1)
        frame_botoes.columnconfigure(1, weight=1)
    
    # Define a função que mostra a mensagem de boas-vindas ao abrir o chatbot.
    def mostrar_mensagem_boas_vindas(self):
        self.adicionar_mensagem("Assistente", "Olá! Sou seu assistente acadêmico. Clique em uma pergunta abaixo.")
    
    # Define a função que adiciona uma mensagem na área de chat.
    def adicionar_mensagem(self, remetente, mensagem):
        self.area_chat.config(state=tk.NORMAL) # Habilita a edição na área de texto, para que aas mensagens apareçam.
        self.area_chat.insert(tk.END, f"{remetente}: {mensagem}\n\n") # Define o formato da mensagem, com o remetente e a mensagem separados por dois pontos e um espaço.
        self.area_chat.config(state=tk.DISABLED) # Desabilita a edição na área de texto após a resposta, para que o usuário não possa escrever.
        self.area_chat.see(tk.END) # Rola a área de texto para o final, para que a última mensagem apareça visível.
    
    # Define a função que faz a pergunta ao chatbot e mostra a resposta na área de chat.
    def fazer_pergunta(self, pergunta):
        self.adicionar_mensagem("Você", pergunta)  # Adiciona a mensagem do botão na área de chat com "Você" sendo o remetente da função a cima.
        self.adicionar_mensagem("Assistente", respostas[pergunta])  # Adiciona a resposta para a pergunta do dicionário na área de chat.

if __name__ == "__main__":  # Define a função principal do programa, que será executada apenas quando o arquivo for rodado diretamente.
    root = tk.Tk()  # Cria a janela principal do programa.
    app = Chatbot(root)  # Cria o aplicativo que passa na janela.
    root.mainloop()  # Inicia o loop principal da interface gráfica, para que o programa continue rodando.