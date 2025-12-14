// COPIE E COLE AQUI O CÓDIGO DO PROTÓTIPO

#include <Arduino.h>
#include <Arduino.h>
#include <WebServer.h>
#include <SD_MMC.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "certificados.h"
#include <MQTT.h>
#include <esp_camera.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEBeacon.h>
#include <ArduinoJson.h>

JsonDocument dados;
String textoJson;

int count = 0;
float soma_acumulada = 0;
float distancia;

camera_config_t config = {
    .pin_pwdn = -1, .pin_reset = -1, .pin_xclk = 15, .pin_sscb_sda = 4, .pin_sscb_scl = 5, .pin_d7 = 16, .pin_d6 = 17, .pin_d5 = 18, .pin_d4 = 12, .pin_d3 = 10, .pin_d2 = 8, .pin_d1 = 9, .pin_d0 = 11, .pin_vsync = 6, .pin_href = 7, .pin_pclk = 13, .xclk_freq_hz = 20000000, .ledc_timer = LEDC_TIMER_0, .ledc_channel = LEDC_CHANNEL_0, .pixel_format = PIXFORMAT_JPEG, .frame_size = FRAMESIZE_SVGA, .jpeg_quality = 10, .fb_count = 2, .grab_mode = CAMERA_GRAB_LATEST};

WebServer servidor(80);
String idDoMeuBeacon = "fda50693-a4e2-4fb1-afcf-c6eb07647825";

BLEScan *scannerBluetooth;

WiFiClientSecure conexaoSegura;
MQTTClient mqtt(1000);

float calcularDistancia(int potenciaSinal)
{
  if (potenciaSinal == 0)
  {
    return -1.0;
  } // erro
  int referencia = -59; // dBm
  float razao = potenciaSinal * 1.0 / referencia;
  float distancia;
  if (razao < 1.0)
  {
    distancia = pow(razao, 10);
  }
  else
  {
    distancia = (0.89976) * pow(razao, 7.7095) + 0.111;
  }
  return (float)(int)(distancia * 10 + 0.5) / 10.0;
}

class MeuRastreador : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice dispositivoBluetooth)
  {
    String dadosFabricante = dispositivoBluetooth.getManufacturerData();
    if (dadosFabricante.length() != 25)
    {
      return;
    } // não é Beacon
    BLEBeacon oBeacon = BLEBeacon();
    oBeacon.setData(dadosFabricante);
    String idDispositivo = oBeacon.getProximityUUID().toString();
    if (idDispositivo == idDoMeuBeacon)
    {
      scannerBluetooth->stop();
      int potencia = dispositivoBluetooth.getRSSI();
      distancia = calcularDistancia(potencia);
      count += 1;
      soma_acumulada += distancia;
      if (count > 4)
      {
        float distancia_media = soma_acumulada / 5;
        Serial.println(distancia_media);
        dados["distancia"] = distancia_media;
        dados["x"] = 11.4;
        dados["y"] = 0.5;
        dados["bancada"] = 9;
        serializeJson(dados, textoJson);
        mqtt.publish("distancia/09", textoJson);
        count = 0;
        soma_acumulada = 0;
      }
      Serial.printf("Beacon a %.1f metros!\n", distancia);
      if (distancia < 2.0)
      {
        rgbLedWrite(RGB_BUILTIN, 0, 255, 0);
      }
      else
      {
        rgbLedWrite(RGB_BUILTIN, 255, 0, 0);
      }
    }
  }
};

void tirarFotoEEnviarParaMQTT()
{
  camera_fb_t *foto = esp_camera_fb_get();
  if (mqtt.publish("foto/09",
                   (const char *)foto->buf, foto->len))
  {
    Serial.println("Foto enviada com sucesso");
  }
  else
  {
    Serial.println("Falha ao enviar foto");
  }

  esp_camera_fb_return(foto); // libera memória
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
      mqtt.connect("LET-09", "aula", "zowmad-tavQez");
      Serial.print(".");
      delay(1000);
    }
    Serial.println(" conectado!");

    mqtt.subscribe("camera/09", 1); // qos = 1
  }
}

void recebeuMensagem_enviou_foto(String topico, String conteudo)
{
  Serial.println(topico + ": " + conteudo);

  if (topico == "camera/09")
  {
    tirarFotoEEnviarParaMQTT();
  }
}

void tirarFotoESalvarNoSDCard()
{
  camera_fb_t *foto = esp_camera_fb_get();
  File arquivo = SD_MMC.open("/foto.jpeg", FILE_WRITE);
  arquivo.write(foto->buf, foto->len);
  arquivo.close();

  esp_camera_fb_return(foto); // libera memória
}

void setup()
{
  Serial.begin(115200);
  delay(500);

  reconectarWiFi();

  SD_MMC.setPins(39, 38, 40);
  if (!SD_MMC.begin("/sdcard", true))
  {
    Serial.println("Erro SD Card");
    while (true)
    {
    };
  }

  conexaoSegura.setCACert(certificado1);
  mqtt.begin("mqtt.janks.dev.br", 8883, conexaoSegura);
  mqtt.onMessage(recebeuMensagem_enviou_foto);
  mqtt.setKeepAlive(10);
  mqtt.setWill("desconectado", "09");

  reconectarMQTT();

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("Erro na câmera: 0x%x", err);
    while (true)
      ;
  }

  BLEDevice::init("");
  scannerBluetooth = BLEDevice::getScan();
  scannerBluetooth->setAdvertisedDeviceCallbacks(new MeuRastreador());
  scannerBluetooth->setActiveScan(true);
  scannerBluetooth->setInterval(100);
  scannerBluetooth->setWindow(99);
}

void loop()
{

  reconectarWiFi();
  reconectarMQTT();
  mqtt.loop();

  scannerBluetooth->start(1, true);
  scannerBluetooth->clearResults();
}
