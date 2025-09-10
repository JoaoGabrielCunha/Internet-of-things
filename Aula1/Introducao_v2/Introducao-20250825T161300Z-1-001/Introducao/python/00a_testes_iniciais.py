from flask import Flask,render_template
from requests import post


app = Flask(__name__)

@app.route("/")
def Mostrar_Pagina_Principal():
  return render_template("pagina1.html")

@app.route("/Telegram/<string:x>")
def Mandar_Mensagem_Telegram(x):
  chave = "8461303397:AAFmnVVnmS19XDH8wwKsGHsO-dtd7wCn5jU"
  id_da_conversa = "895090204"
  endereco_base = "https://api.telegram.org/bot" +chave
  endereco = endereco_base +"/sendMessage"
  dados = {"chat_id": id_da_conversa, "text": x}

  post(endereco,data = dados)
  return "Texto enviado"




app.run(port=5000, debug=True)