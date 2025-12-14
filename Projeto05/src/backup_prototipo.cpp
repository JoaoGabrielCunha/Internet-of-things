/*#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <GFButton.h>
#include <HX711.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "certificados.h"
#include <MQTT.h>
#include <ArduinoJson.h>

String nomeAlimento = "Ervilha com Wasabi";
int id_alimento = 2;
float preco100g = 6.99;
float peso_total;
float preco_total;
String preco_total_em_string;

MFRC522 rfid(46, 17);
MFRC522::MIFARE_Key chaveA = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};

String idCliente;
String nomeCliente;

U8G2_FOR_ADAFRUIT_GFX fontes;
GxEPD2_290_T94_V2 modeloTela(10, 14, 15, 16);
GxEPD2_BW<GxEPD2_290_T94_V2, GxEPD2_290_T94_V2::HEIGHT> tela(modeloTela);

GFButton botao(5);
HX711 balanca;

float pesoMedio;
String pesoMedio_emString;

WiFiClientSecure conexaoSegura;
MQTTClient mqtt(1000); 

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

void botaoPressionado(GFButton &botaoDoEvento)
{
  Serial.println("Botão foi pressionado!");
  if (idCliente != "")
  {
    balanca.tare(5);
  }
}


void botaoSolto(GFButton &botaoDoEvento)
{
  Serial.println("Botão foi solto!");
  dados["peso"] = peso_total;
  dados["custo"]=  preco_total;
  dados["rfid_cliente"] =  idCliente;
  dados["id_alimento"] =  id_alimento;
  serializeJson(dados,dados_emString);
  mqtt.publish("consumo/09",dados_emString);
  idCliente = "";
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

    mqtt.subscribe("consumo/09");                
    
  }
}

void recebeuMensagem(String topico, String conteudo) 
{
 Serial.println(topico + ": " + conteudo);
}


void setup()
{
  Serial.begin(115200);
  delay(1500);
  Serial.println("Testes iniciais!");

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

  balanca.begin(6, 7); // pinos de comunicação
  balanca.set_scale(424);

  mqtt.begin("mqtt.janks.dev.br", 8883, conexaoSegura);
  mqtt.onMessage(recebeuMensagem);
  mqtt.setKeepAlive(10);
  mqtt.setWill("tópico da desconexão", "conteúdo"); 

  reconectarWiFi();
  conexaoSegura.setCACert(certificado1); 
  reconectarMQTT();

}

void loop()
{
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())
  {
    tela.fillScreen(GxEPD_WHITE);
    idCliente = lerUID();
    Serial.println("UID da tag: " + idCliente);
    String texto = lerTextoDoBloco(5);

    Serial.println("Nome do cliente: " + texto);

    rfid.PICC_HaltA(); // interrompe leitura (não fica repetindo)
    rfid.PCD_StopCrypto1();

    fontes.setFont(u8g2_font_helvB24_te);
    fontes.setFontMode(1);
    fontes.setCursor(10, 40);
    String texto_1 = "Olá," + texto;
    fontes.print(texto_1);

    fontes.setFont(u8g2_font_helvB12_te);
    fontes.setFontMode(1);
    fontes.setCursor(10, 80);
    fontes.print("Coloque a embalagem e \n segura o botão");

    tela.display(true);
  }

  botao.process();

  if (idCliente != "" && botao.isPressed())
  {
    tela.fillScreen(GxEPD_WHITE);
    Serial.println("Entrou no loop!");

    pesoMedio = balanca.get_units(5);
    pesoMedio_emString = String(pesoMedio) + "g";

    fontes.setFont(u8g2_font_helvB12_te);
    fontes.setFontMode(1);

    fontes.setCursor(10, 40);
    fontes.print(nomeAlimento);

    fontes.setCursor(10, 80);
    fontes.print(pesoMedio_emString);

    preco_total = preco100g * pesoMedio / 100.0;
    preco_total_em_string = String(preco_total);

    fontes.setCursor(10, 95);
    preco_total_em_string = "R$" + preco_total_em_string;
    fontes.print(preco_total_em_string);
    tela.display(true);
  }

  reconectarWiFi();
  reconectarMQTT();
  mqtt.loop();

}
*/