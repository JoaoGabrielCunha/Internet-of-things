#include <Arduino.h>
#include<WiFi.h>
#include<WiFiClientSecure.h>
#include "certificados.h"
#include<MQTT.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <GxEPD2_BW.h>
#include <GFButton.h>
#include <time.h>
#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>


GxEPD2_290_T94_V2 modeloTela(10, 14, 15, 16);
GxEPD2_BW<GxEPD2_290_T94_V2, GxEPD2_290_T94_V2::HEIGHT> tela(modeloTela);
U8G2_FOR_ADAFRUIT_GFX fontes;





WiFiClientSecure conexaoSegura;
MQTTClient mqtt(1000);
GFButton botaoCentro(5);
GFButton botaoDireita(4);
GFButton botaoEsquerda(3);
GFButton botaoTras(2);
GFButton botaoFrente(1);




struct tm tempo;

int hora ;
int minuto;
int segundo;
int dia;
int mes;
int ano;
char buffer_data_hora[150];
String tempoString;

unsigned long instanteAnterior = 0; 

int x= 150;
int y= 60;






void reconectarMQTT() { 
  if (!mqtt.connected()) { 
    Serial.print("Conectando MQTT..."); 
    while(!mqtt.connected()) { 
      mqtt.connect("8883", "aula", "zowmad-tavQez"); 
      Serial.print("."); 
      delay(1000); 
    } 
    Serial.println(" conectado!"); 
    
    mqtt.subscribe("nome");                // qos = 0  
    mqtt.subscribe("nome/+/parametro", 1); // qos = 1  
  } 
}

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


void recebeuMensagem(String topico, String conteudo){
  Serial.println(topico + ": " + conteudo);
}


void FuncaoBotaoCentro(GFButton &botao){
  mqtt.publish("nome","JoaoGabriel");
};

void EscreverDataHora_E_DesenhaCírculo(String tempoString){
tela.fillCircle(x,y,5, GxEPD_BLACK);
fontes.setFont( u8g2_font_helvB24_te ); // Helvetica Bold 24pt
fontes.setFontMode(1); // modo transparente
fontes.setCursor(0, 85); // x, y

fontes.print(tempoString);
tela.display(true);


}

void EscreverDataHora(String tempoString){

fontes.setFont( u8g2_font_helvB24_te ); // Helvetica Bold 24pt
fontes.setFontMode(1); // modo transparente
fontes.setCursor(0, 50); // x, y

fontes.print(tempoString);
tela.display(true);


}



void atualiza(int deltaX, int deltaY)
{ tela.fillScreen(GxEPD_WHITE);
  
  x+= deltaX;
  y+= deltaY;
  EscreverDataHora_E_DesenhaCírculo(tempoString);
}



void setup() {
  Serial.begin(115200); delay(500);

  reconectarWiFi();
  conexaoSegura.setCACert(certificado1);

  mqtt.begin("mqtt.janks.dev.br",8883,conexaoSegura);

  reconectarMQTT();

  mqtt.onMessage(recebeuMensagem);
  mqtt.setKeepAlive(10);
  mqtt.setWill("tópico da desconexão","conteúdo");
  
  
  botaoCentro.setPressHandler(FuncaoBotaoCentro);

  getLocalTime(&tempo);
  configTzTime("<-03>3","a.ntp.br","pool.ntp.org");
  strftime(buffer_data_hora, sizeof(buffer_data_hora), "%d/%m/%Y %H:%M:%S", &tempo);
  tempoString = String(buffer_data_hora);

  tela.init();
  tela.setRotation(3);
  tela.fillScreen(GxEPD_WHITE);
  tela.display(true);

  fontes.begin(tela);
  fontes.setForegroundColor(GxEPD_BLACK);

  botaoEsquerda.setPressHandler([](GFButton &b){atualiza(-5,0);});
  botaoDireita.setPressHandler([](GFButton &b){atualiza(+5,0);});
  botaoFrente.setPressHandler([](GFButton &b){atualiza(0,-5);});
  botaoTras.setPressHandler([](GFButton &b){atualiza(0,+5);});
}

void loop() {

unsigned long instanteAtual = millis();
 if (instanteAtual > instanteAnterior + 2000) {
    EscreverDataHora_E_DesenhaCírculo(tempoString);
    instanteAnterior = instanteAtual;
    }


reconectarWiFi();

reconectarMQTT();

mqtt.loop();

botaoCentro.process();

botaoEsquerda.process();
botaoDireita.process();
botaoFrente.process();
botaoTras.process();

}
