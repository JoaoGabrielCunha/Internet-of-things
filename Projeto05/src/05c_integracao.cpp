
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <GFButton.h>
#include <HX711.h>
#include <Wifi.h>
#include <WiFiClientSecure.h>
#include "certificados.h"
#include <MQTT.h>
#include <ArduinoJson.h>



MFRC522 rfid(46, 17);
MFRC522::MIFARE_Key chaveA = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};

String idCliente;
String nomeCliente;

U8G2_FOR_ADAFRUIT_GFX fontes;
GxEPD2_290_T94_V2 modeloTela(10, 14, 15, 16);
GxEPD2_BW<GxEPD2_290_T94_V2, GxEPD2_290_T94_V2::HEIGHT> tela(modeloTela);

GFButton botao(5);

HX711 balanca;

String nome_alimento = "Ervilha com Wasabi";
int id_alimento = 2;
float preco_100g = 6.99;
String preco_100g_em_String = "6.99";

WiFiClientSecure conexaoSegura;
MQTTClient mqtt(1000); 

float preco_total;
float peso_atual_em_float ;

JsonDocument dados;
String dados_emString;

String lerUID()
{
  String id = "";
  for (byte i = 0; i < rfid.uid.size; i++)
  {
    if (i > 0)
    {
      id += " "; // espaço depois do byte anterior
    }
    if (rfid.uid.uidByte[i] < 0x10)
    {
      id += "0"; // para ficar "07" em vez de "7", por exemplo
    }
    id += String(rfid.uid.uidByte[i], HEX);
  }
  id.toUpperCase();
  return id;
}

String lerTextoDoBloco(byte bloco)
{
  byte tamanhoDados = 18; // 16 bytes de dados + 2 para CRC
  char dados[tamanhoDados];

  MFRC522::StatusCode status = rfid.PCD_Authenticate(
      MFRC522::PICC_CMD_MF_AUTH_KEY_A, bloco, &chaveA, &(rfid.uid));
  if (status != MFRC522::STATUS_OK)
  {
    return "";
  }

  status = rfid.MIFARE_Read(bloco, (byte *)dados, &tamanhoDados);
  if (status != MFRC522::STATUS_OK)
  {
    return "";
  }
  dados[tamanhoDados - 2] = '\0';
  return String(dados);
}

void botaoPressionado(GFButton& botaoDoEvento){
  Serial.println("Botão foi pressionado!");
  if (idCliente !="")
  {
    balanca.tare(5);
  }
}

void botaoSolto(GFButton& botaoDoEvento)
{
  Serial.println("Botão foi solto!");

  dados["peso"] =  String(peso_atual_em_float) ;
  dados["custo"] = String(preco_total);
  dados["rfid_cliente"] = idCliente;
  dados["id_alimento"] = id_alimento;
  serializeJson(dados,dados_emString);

  mqtt.publish("consumo/08", dados_emString);
  idCliente= "";
  nomeCliente = "";
  tela.fillScreen(GxEPD_WHITE);
  tela.display(true);

}

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

void recebeuMensagem(String topico, String conteudo)
{
  Serial.println(topico + ": " + conteudo);
if(topico == "ajuste/08")
{
  JsonDocument dados_recebidos_em_Json;
  deserializeJson(dados_recebidos_em_Json,conteudo);
  id_alimento =  dados_recebidos_em_Json["id"];
  nome_alimento = String(dados_recebidos_em_Json["nome"]);
   preco_100g_em_String = String(dados_recebidos_em_Json["preco_por_100g"]);
   Serial.println(id_alimento);
   Serial.println(nome_alimento);
   Serial.println(preco_100g_em_String);
}

}

void reconectarMQTT()
{
  if (!mqtt.connected())
  {
    Serial.print("Conectando MQTT...");
    while (!mqtt.connected())
    {
      mqtt.connect("8883", "aula", "zowmad-tavQez");
      Serial.print(".");
      delay(1000);
    }
    Serial.println(" conectado!");

    mqtt.subscribe("consumo/08"); 
    mqtt.subscribe("ajuste/08");             
    
  }
}




void setup()
{
  Serial.begin(115200);
  delay(1500);
  Serial.println("Prototipo!");
  SPI.begin();
  rfid.PCD_Init();

  tela.init();
  tela.setRotation(3);
  tela.fillScreen(GxEPD_WHITE);
  tela.display(true);

  fontes.begin(tela);
  fontes.setForegroundColor(GxEPD_BLACK);


  botao.setPressHandler(botaoPressionado);
  botao.setReleaseHandler(botaoSolto);

  balanca.begin(6,7);
  balanca.set_scale(424);




  
   

  mqtt.begin("mqtt.janks.dev.br", 8883, conexaoSegura);
 
  mqtt.setKeepAlive(10);
  mqtt.setWill("tópico da desconexão", "conteúdo");
  
   reconectarWiFi();
  conexaoSegura.setCACert(certificado1); 
  reconectarMQTT();
mqtt.onMessage(recebeuMensagem);
}

void loop()
{

  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())
  {
    idCliente = lerUID();
    Serial.println("UID da tag: " + idCliente);
    nomeCliente = lerTextoDoBloco(5);
    Serial.println("Texto no bloco 5: " + nomeCliente);

    fontes.setFont( u8g2_font_helvB24_te );
    fontes.setFontMode(1);
    fontes.setCursor(3, 45);
    String texto_de_cima = "Olá, " + nomeCliente;

    fontes.print(texto_de_cima);

    fontes.setFont( u8g2_font_helvB18_te );
    fontes.setFontMode(1);
    fontes.setCursor(3, 90);
    fontes.print("Coloque a embalagem e ");
    fontes.setCursor(3, 110);
    fontes.print("segure o botão");
    tela.display(true); // S
   
    rfid.PICC_HaltA(); // interrompe leitura (não fica repetindo)
    rfid.PCD_StopCrypto1();
  }

  botao.process();


  if(idCliente!=""  && botao.isPressed())
  {
    tela.fillScreen(GxEPD_WHITE);
    idCliente = lerUID();
    Serial.println("UID da tag: " + idCliente);
    String texto = lerTextoDoBloco(5);

    
    fontes.setFont( u8g2_font_helvB14_te );
    fontes.setFontMode(1);
    fontes.setCursor(10,40);
    fontes.print(nome_alimento);

    peso_atual_em_float =  balanca.get_units(5);
    String peso_atual_em_string = String(peso_atual_em_float) + "g";

    fontes.setCursor(10,80); 
    fontes.print(peso_atual_em_float);

    preco_total =   preco_100g* peso_atual_em_float/100.0;
    String preco_total_em_string = "R$" + String(preco_total);
    fontes.setCursor(10,95);
    fontes.print(preco_total_em_string);


                                           
    tela.display(true);
  }

  reconectarWiFi();
  reconectarMQTT();

  mqtt.loop();

}
