#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <Wifi.h>
#include <WebServer.h>
#include <WifiClientSecure.h>
#include <GFButton.h>
#include <uri/UriBraces.h>
#include <HTTPClient.h>
#include "certificados.h"

int leds[] = {42, 41, 40};
int pino = 18;

JsonDocument dados;

WebServer servidor(80);
String chaveTelegram = "8461303397:AAFmnVVnmS19XDH8wwKsGHsO-dtd7wCn5jU";
String idDoChat = "895090204";
String enderecoBase = "https://api.telegram.org/bot" + chaveTelegram;

WiFiClientSecure conexaoSegura;

GFButton sensorDeMovimento(21);

struct tm tempo;

void reconectarWiFi()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.begin("Projeto", "2022-11-07");
    Serial.print("Conectando ao WiFi...");
    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print(".");
      delay(1000);
    }
    Serial.print("conectado!\nEndereço IP: ");
    Serial.println(WiFi.localIP());
  }
}
void enviarMensagemTelegram(String mensagem)
{
  JsonDocument dados;
  dados["chat_id"] = idDoChat;
  dados["text"] = mensagem;
  String dadosString;
  serializeJson(dados, dadosString);

  String enderecoMensagem = enderecoBase + "/sendMessage";
  HTTPClient requisicao;
  requisicao.begin(conexaoSegura, enderecoMensagem);
  requisicao.addHeader("Content-Type", "application/json");
  int codigoDoResultado = requisicao.POST(dadosString);
  String resposta = requisicao.getString();
  Serial.println(resposta);

  if (codigoDoResultado != 200)
  {
    Serial.println("Erro ao enviar a mensagem!");
  }
}
void movimentoDetectado(GFButton &sensorDeMovimento)
{

  if (dados["alertarMovimento"] == true)
  {
    Serial.println("Movimento detectado,enviando mensagem para o Telegram!");
    enviarMensagemTelegram("Movimento foi detectado");
  }
}
void inercia(GFButton &sensor)
{
  Serial.println("Inércia detectada!");
}
void pagina1()
{
  File arquivo = LittleFS.open("/ajustes.html","r");
  if (!arquivo){
    servidor.send(500,"text/html","Erro no HTML");
    return ;
  }

  String html = arquivo.readString();
  arquivo.close();
  html.replace("{{nome}}","João");
  servidor.send(200,"text/html",html);

}



void setup()
{
  Serial.begin(115200);
  delay(500);

  reconectarWiFi();

  if (!LittleFS.begin())
  {
     
    while (true)
    {
    };

    
  }


  File arquivo = LittleFS.open("/ajustes.json", "r");
  deserializeJson(dados, arquivo);
  arquivo.close();

  sensorDeMovimento.setPressHandler(inercia);
  sensorDeMovimento.setReleaseHandler(movimentoDetectado);

  conexaoSegura.setCACert(certificado1);
  conexaoSegura.setCACert(certificado2);

  pinMode(leds[0], OUTPUT);
  pinMode(leds[1], OUTPUT);
  pinMode(leds[2], OUTPUT);
  digitalWrite(leds[0],HIGH);
  digitalWrite(leds[1],HIGH);
  digitalWrite(leds[2],HIGH);

  configTzTime("<-03>3", "a.ntp.br", "pool.ntp.org");

  servidor.on("/inicio",HTTP_GET, pagina1);
  servidor.begin();

}

void loop()
{

  reconectarWiFi();

  int leituraLuz = analogRead(pino);
  int porcentagemLuz = map(leituraLuz, 0, 4095, 0, 100);

  if (porcentagemLuz < 20)
  {
    rgbLedWrite(RGB_BUILTIN, 0, 0, 0);
  }
  else
  {
    if (dados["corRGB"] == "vermelho")
    {
      rgbLedWrite(RGB_BUILTIN, 255, 0, 0);
    }
    else if (dados["corRGB"] == "verde")
    {
      rgbLedWrite(RGB_BUILTIN, 0, 255, 0);
    }
    else if (dados["corRGB"] == "amarelo")
    {
      rgbLedWrite(RGB_BUILTIN, 255, 255, 0);
    }
    else if (dados["corRGB"] == "rosa")
    {
      rgbLedWrite(RGB_BUILTIN, 100.0, 55.3, 63.1);
    }
  }
  leituraLuz = analogRead(pino);

  sensorDeMovimento.process();

  getLocalTime(&tempo);

  char buffer[100];
  strftime(buffer, sizeof(buffer), "%H:%M:%S", &tempo);
  String tempoString = String(buffer);
  

  if (tempoString == dados["horarioAcenderTudo"])
  {
    digitalWrite(leds[0], LOW);
    digitalWrite(leds[1], LOW);
    digitalWrite(leds[2], LOW);
  }

  else if (tempoString == dados["horarioApagarTudo"])
  {
    digitalWrite(leds[0], HIGH);
    digitalWrite(leds[1], HIGH);
    digitalWrite(leds[2], HIGH);
  }
}
