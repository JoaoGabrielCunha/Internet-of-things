#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <HX711.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "certificados.h"
#include <MQTT.h>
#include <WiFi.h>

MFRC522 rfid(46, 17);
MFRC522::MIFARE_Key chaveA = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};

unsigned long instanteAnterior = 0;
HX711 balanca;
float pesoMedido;

WiFiClientSecure conexaoSegura;
MQTTClient mqtt(1000);

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

    mqtt.subscribe("peso/08");                
    
  }
}

void recebeuMensagem(String topico, String conteudo)
{
  Serial.println(topico + ": " + conteudo);
}

void setup()
{
  Serial.begin(115200);
  delay(500);

  SPI.begin();
  rfid.PCD_Init();
  balanca.begin(6, 7); // pinos de comunicação
  balanca.set_scale(424);
   balanca.tare(5);

  reconectarWiFi();

  conexaoSegura.setCACert(certificado1);
  mqtt.begin("mqtt.janks.dev.br", 8883, conexaoSegura);
  mqtt.onMessage(recebeuMensagem);
  mqtt.setKeepAlive(10);
  mqtt.setWill("tópico da desconexão", "conteúdo");
  
}

void loop()
{

  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())
  {
    String id = lerUID();
    Serial.println("UID da tag: " + id);
    String texto = lerTextoDoBloco(6);
    Serial.println("Texto no bloco 6: " + texto);
    if (id == "D0 21 95 10")
    {
      rgbLedWrite(RGB_BUILTIN, 0, 255, 0);
    }
    else
    {
      rgbLedWrite(RGB_BUILTIN, 255, 0, 0);
    }

    rfid.PICC_HaltA(); // interrompe leitura (não fica repetindo)
    rfid.PCD_StopCrypto1();
  }
  
  unsigned long instanteAtual = millis();
  if (instanteAtual > instanteAnterior + 2000)
  {
    Serial.println("+2 segundos");
    instanteAnterior = instanteAtual;
    pesoMedido = balanca.get_units(5);
   
    String pesoMedido_String = String(pesoMedido);
    mqtt.publish("peso/08", pesoMedido_String); 
    
  }

  reconectarWiFi();
  reconectarMQTT();
  mqtt.loop();
}