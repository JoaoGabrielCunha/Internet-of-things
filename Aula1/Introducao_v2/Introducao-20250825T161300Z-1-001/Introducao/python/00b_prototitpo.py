import json
from flask import Flask,render_template
from requests import post


with open("paises.json", "r", encoding="utf-8") as arquivo:
  lista_de_paises = json.load(arquivo)




app = Flask(__name__)

@app.route("/pais/<int:x>") 
def mostrar_pagina_principal(x): 
  pais = lista_de_paises[x]
  return render_template( 
     "pagina2.html",  
     dicionario= pais 
  )





app.run(port=5000, debug=True)