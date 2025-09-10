// COPIE E COLE AQUI O CÓDIGO DO PROTÓTIPO
#include <Arduino.h>
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
#include<string.h>
#include <ArduinoJson.h>



WiFiClientSecure conexaoSegura;
MQTTClient mqtt(1000);
GFButton botaoCentro(5);
GFButton botaoDireita(4);
GFButton botaoEsquerda(3);
GFButton botaoTras(2);
GFButton botaoFrente(1);


GxEPD2_290_T94_V2 modeloTela(10, 14, 15, 16);
GxEPD2_BW<GxEPD2_290_T94_V2, GxEPD2_290_T94_V2::HEIGHT> tela(modeloTela);
U8G2_FOR_ADAFRUIT_GFX fontes;

bool botoesHabilitados = false;
int conta_setas_enviadas= 0;
int total_setas;

JsonDocument dados_em_lista;

void reconectarMQTT() { 
  if (!mqtt.connected()) { 
    Serial.print("Conectando MQTT..."); 
    while(!mqtt.connected()) { 
      mqtt.connect("8883", "aula", "zowmad-tavQez"); 
      Serial.print("."); 
      delay(1000); 
    } 
    Serial.println(" conectado!"); 
    
    mqtt.subscribe("inicio/JoaoGabrielC");                 
    mqtt.subscribe("sequencia/JoaoGabrielC");  
    mqtt.subscribe("aguardando/JoaoGabrielC");                  
    mqtt.subscribe("resultados/JoaoGabrielC"); 
    mqtt.subscribe("fim/JoaoGabrielC");

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
  
  if(topico.startsWith("sequencia")){
    Serial.println("Recebeu sequencia" + conteudo);
    total_setas = conteudo.length();
     tela.fillScreen(GxEPD_WHITE);
    fontes.setFont( u8g2_font_helvB18_te ); // Helvetica Bold 24pt
    fontes.setFontMode(1); // modo transparente
    fontes.setCursor(0, 50); // x, y
  
    fontes.print("Memorize a sequência!");
    if(conteudo.length() <= 6)
    {
       fontes.setFont( u8g2_font_open_iconic_all_4x_t ); //símbolos
      fontes.setFontMode(1); // modo transparente
      
      fontes.setCursor(0, 90); // x, y
      

      fontes.print(conteudo);
      tela.display(true);

      
    }
    else


    {
     
      
     fontes.setFont( u8g2_font_open_iconic_all_4x_t ); //símbolos
      fontes.setFontMode(1); // modo transparente
      
      fontes.setCursor(0, 85); // x, y
      fontes.print(conteudo.substring(0,6));
      fontes.print("\n");
      fontes.print(conteudo.substring(6));
      tela.display(true);
    }
  }
  if (topico.startsWith("aguardando")){

   tela.fillScreen(GxEPD_WHITE);
   rgbLedWrite(RGB_BUILTIN,0 , 0, 0);
   
    fontes.setFont( u8g2_font_helvB14_te ); // Helvetica Bold 24pt
    fontes.setFontMode(1); // modo transparente
    fontes.setCursor(0, 50); // x, y
  
    fontes.print("Repita a sequência quando");
    fontes.print("\n");
    fontes.print(" a luz ficar verde");
    tela.display(true);
    rgbLedWrite(RGB_BUILTIN,0 , 255, 0);
    botoesHabilitados = true;

  }
    if (topico.startsWith("inicio")){
    
    tela.fillScreen(GxEPD_WHITE);
    fontes.setFont( u8g2_font_helvB18_te ); // Helvetica Bold 24pt
    fontes.setFontMode(1); // modo transparente
    fontes.setCursor(0, 50); // x, y
  
    fontes.print("Partida iniciada!");

                    
    mqtt.subscribe("sequencia/" +conteudo);  
    mqtt.subscribe("aguardando/" + conteudo);                  
    mqtt.subscribe("resultados/"+ conteudo); 
    mqtt.subscribe("fim/" + conteudo);
    tela.display(true);
    }

  if (topico.startsWith("resultado"))
  {
    tela.fillScreen(GxEPD_WHITE);
    tela.drawLine(5,50,291,50,GxEPD_BLACK);
    tela.drawLine(148,5,148,130,GxEPD_BLACK);

    
    fontes.setFont( u8g2_font_helvB12_te ); // Helvetica Bold 24pt
    fontes.setFontMode(1); // modo transparente
    fontes.setCursor(5,30); // x, y
    fontes.print("Jogador");

    fontes.setCursor(153,30);
    fontes.print("Pontuação");
    
    deserializeJson(dados_em_lista,conteudo);
    int x_inicial,y_inicial;
    x_inicial= 5;
    y_inicial = 55;
    for (unsigned int i=0; i<dados_em_lista.size();i++){
      String nome_jogador = dados_em_lista[i]["jogador"];
      y_inicial +=15;
      fontes.setCursor(x_inicial,y_inicial);
      fontes.print(nome_jogador);
    }
      x_inicial = 153;
      y_inicial = 55;
    for (unsigned int i=0; i<dados_em_lista.size(); i++)
    {
      String pontos_jogador = dados_em_lista[i]["pontos"];
      y_inicial+=15;
      fontes.setCursor(x_inicial,y_inicial);
      fontes.print(pontos_jogador);
    }
  tela.display(true);

  }

  if (topico.startsWith("fim"))
  {
    tela.fillScreen(GxEPD_WHITE);
    fontes.setFont( u8g2_font_helvB24_te ); // Helvetica Bold 24pt
    fontes.setFontMode(1); // modo transparente
    fontes.setCursor(10, 50); // x, y
  
    fontes.print("Fim de Jogo!");

    fontes.setCursor(12,70);
    fontes.setFont( u8g2_font_helvB14_te );
    fontes.setFontMode(1); // modo transparente
    fontes.print("Vitória de"+conteudo);
    tela.display(true);


  }


}


void LidaBotoes(String comando){
  
  if (botoesHabilitados==true){
    mqtt.publish("jogada/JoaoGabrielC", comando);
    rgbLedWrite(RGB_BUILTIN,255 ,255, 0);
    delay(30);
    rgbLedWrite(RGB_BUILTIN,0 ,0, 0);
    conta_setas_enviadas+=1;
    if (conta_setas_enviadas >= total_setas){
        tela.fillScreen(GxEPD_WHITE);
        fontes.setFont( u8g2_font_helvB18_te ); // Helvetica Bold 24pt
        fontes.setFontMode(1); // modo transparente
        fontes.setCursor(0, 50); // x, y
  
        fontes.print("Aguardando resultados....");
        
        conta_setas_enviadas =0;
        botoesHabilitados = false;
        tela.display(true);

    }
  }
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

  fontes.begin(tela);
  fontes.setForegroundColor(GxEPD_BLACK);

  mqtt.publish("entrar","JoaoGabrielC");

  tela.init();
  tela.setRotation(3);
  tela.fillScreen(GxEPD_WHITE);
  tela.display(true);

  fontes.setFont( u8g2_font_helvB24_te ); // Helvetica Bold 24pt
  fontes.setFontMode(1); // modo transparente
  fontes.setCursor(0, 50); // x, y

  fontes.print("Aguardando início....");
  tela.display(true);

  rgbLedWrite(RGB_BUILTIN,0 , 0, 0);
  
  botaoEsquerda.setPressHandler([](GFButton &b){LidaBotoes("J");});
  botaoDireita.setPressHandler([](GFButton &b){LidaBotoes("K");});
  botaoTras.setPressHandler([](GFButton &b){LidaBotoes("I");});
  botaoFrente.setPressHandler([](GFButton &b){LidaBotoes("L");});

}

void loop() {


  reconectarWiFi();

  reconectarMQTT();
  mqtt.loop();

  botaoEsquerda.process();
  botaoDireita.process();
  botaoFrente.process();
  botaoTras.process();
}

