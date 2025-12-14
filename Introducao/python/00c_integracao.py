# COPIE AQUI O CÓDIGO DO PROTÓTIPO

import json
from flask import Flask,render_template, redirect, request
from requests import post


with open("paises.json", "r", encoding="utf-8") as arquivo:
  lista_de_paises = json.load(arquivo)


for elements in lista_de_paises:
  print(elements)

app = Flask(__name__)

@app.route("/pais/<int:x>") 
def mostrar_pagina_principal(x): 
  pais = lista_de_paises[x]
  return render_template( 
     "pagina2.html",  
     dicionario= pais 
  )

for el in lista_de_paises:
  print(el)

@app.route("/formulario", methods = ["GET","POST"]) 
def mostrar_formulario(): 
    if request.method == "POST":
      Nome_Da_Bandeira = request.form["Nome_Bandeira"]
      URL_Da_Imagem= request.form["URL_imagem"]
      Json = request.form["Json"]

      lista_json= json.loads(Json)

      dicionario = {}
      dicionario["nome"] = Nome_Da_Bandeira
      dicionario["imagem"] = URL_Da_Imagem
      dicionario["elementos"]= lista_json
      lista_de_paises.append(dicionario)
      return redirect("/pais/4")
    return render_template("formulario.html") 





app.run(port=5000, debug=True)