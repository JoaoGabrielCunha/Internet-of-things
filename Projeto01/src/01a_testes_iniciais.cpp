#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <WebServer.h>
#include <uri/UriBraces.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "certificados.h"
#include <WifiClientSecure.h>
#include <LittleFS.h>
#include <GFButton.h>




WebServer servidor(80);
unsigned long instanteAnterior = 0;
struct tm tempo;

int pinoDoLED = 40;
int pinoLuz = 18;

String chaveTelegram = "8461303397:AAFmnVVnmS19XDH8wwKsGHsO-dtd7wCn5jU";
String idDoChat =  "895090204";
String enderecoBase =  "https://api.telegram.org/bot" + chaveTelegram;

WiFiClientSecure conexaoSegura; 

GFButton sensorDeMovimento(35);


void reconectarWiFi() { 
  if (WiFi.status() != WL_CONNECTED) { 
    WiFi.begin("Projeto", "2022-11-07"); 
    Serial.print("Conectando ao WiFi..."); 
    while (WiFi.status() != WL_CONNECTED) { 
      Serial.print("."); 
      delay(1000); 
    } 
    Serial.print("conectado!\nEndereço IP: "); 
    Serial.println(WiFi.localIP()); 
  } 
}
void paginaInicial(){
  servidor.send(200,"text/html","Bem-Vindo!");
}
void pagina1(){
  servidor.send(200, "text/html", "Bem vindo!");
}
void paginaComParametros(){
  int vermelho = servidor.pathArg(0).toInt();
  int verde = servidor.pathArg(1).toInt();
  int azul =  servidor.pathArg(2).toInt();
  
  rgbLedWrite(RGB_BUILTIN,vermelho,verde,azul);
   servidor.send(200, "text/html", "Ascendeu a cor");
}
void enviarMensagemTelegram(String mensagem){
  JsonDocument dados;
  dados["chat_id"] = idDoChat;
  dados["text"] = mensagem ;
  String dadosString;
  serializeJson(dados, dadosString);

  String enderecoMensagemTexto = enderecoBase + "/sendMessage";
  HTTPClient requisicao;
  requisicao.begin(conexaoSegura, enderecoMensagemTexto);
  requisicao.addHeader("Content-Type", "application/json");
  int codigoDoResultado = requisicao.POST(dadosString);
  String resposta = requisicao.getString();
  Serial.println(resposta);

  if (codigoDoResultado !=200)
  {
    Serial.println("Erro ao enviar mensagem!");
  }

}
void paginaEnviaTelegram(){
  String texto = servidor.pathArg(0);
  servidor.send(200,"text.html","Texto enviado no Telegram");
  enviarMensagemTelegram(texto);

}
String LeArquivo(String nome_arquivo){
  File arquivo = LittleFS.open(nome_arquivo, "r"); // "read" 

if (!arquivo) { 
  Serial.println("Erro ao abrir o arquivo!"); 
  while (true) {}; // trava programa aqui 
} 
String conteudo = arquivo.readString(); 
arquivo.close(); 
Serial.println(conteudo);
return conteudo;
}
void EscreveArquivo(int entrada ){
  File arquivo = LittleFS.open("/arquivo2.txt", "w");
  arquivo.println(entrada);
  
  arquivo.close();
}
void paginaLeLuz()
{
  String Codigo_HTML = LeArquivo("/index.html");
  int leitura = analogRead(pinoLuz);
  String porcentagemLuz = String(map(leitura,0,4095,0,100));
  
  Codigo_HTML.replace("{{valorDaLuz}}",porcentagemLuz);
  servidor.send(200, "text/html", Codigo_HTML);

}



void movimentoDetectado(GFButton& sensor){
  int count = LeArquivo("/arquivo2.txt").toInt();
  count = count+1;
  EscreveArquivo(count);

  Serial.print("Número lido e adicionado! ");
  Serial.println(count);

}

void setup() {
  Serial.begin(115200); delay(500);

  
  reconectarWiFi();
  configTzTime("<-03>3", "a.ntp.br","pool.ntp.org");
  


  


  servidor.on("/inicio", HTTP_GET, paginaInicial);
  servidor.on(UriBraces("/parametros/{}/{}/{}"), HTTP_GET,paginaComParametros);
  servidor.on(UriBraces("/telegram/{}"),HTTP_GET,paginaEnviaTelegram);
  servidor.on(UriBraces("/VerificaLuz"),HTTP_GET,paginaLeLuz);
  servidor.begin();

  pinMode(pinoDoLED,OUTPUT);
  digitalWrite(pinoDoLED,HIGH);

  conexaoSegura.setCACert(certificado1);
  conexaoSegura.setCACert(certificado2);


  //Montando sistema de arquivos:
  if (!LittleFS.begin()) { 
    Serial.println("Falha ao montar o sistema de arquivos!"); 
    while (true) {}; // trava programa aqui em caso de erro 
  } 

  sensorDeMovimento.setReleaseHandler(movimentoDetectado);


}
  


void loop() {
  
reconectarWiFi();

unsigned long instanteAtual = millis();
if (instanteAtual > instanteAnterior +1000){
  getLocalTime(&tempo);
  Serial.print("Agora são ");
  Serial.println(&tempo, "%H:%M:%S");
  instanteAnterior = instanteAtual;
}


 servidor.handleClient();
 sensorDeMovimento.process();



}
