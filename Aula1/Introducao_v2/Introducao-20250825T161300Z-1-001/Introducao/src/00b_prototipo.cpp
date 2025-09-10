#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Adafruit_ILI9341.h>
#include <Wire.h>
#include <Adafruit_FT6206.h>
#include <ArduinoJson.h>

Adafruit_ILI9341 tela = Adafruit_ILI9341(53, 49, 48);
Adafruit_FT6206 touch = Adafruit_FT6206();

void desenharRetangulo(JsonDocument elemento)
{
  int x = elemento["x"];
  int y = elemento["y"];
  int altura = elemento["altura"];
  int comprimento = elemento["comprimento"];
  unsigned int cor = elemento["cor"];
  tela.fillRect(x, y, comprimento, altura, cor);
}

void desenharCirculo(JsonDocument elemento)
{
  int x = elemento["x"];
  int y = elemento["y"];
  int raio = elemento["raio"];
  unsigned int cor = elemento["cor"];

  tela.fillCircle(x, y, raio, cor);
}
void desenharPoligono(JsonDocument elemento)
{
  int cx = elemento["centro"]["x"];
  int cy = elemento["centro"]["y"];
  unsigned int cor = elemento["cor"];

  for (unsigned int i = 0; i < elemento["pontos"].size()-1; i++)
  {
    JsonDocument ponto1 = elemento["pontos"][i];
    JsonDocument ponto2 = elemento["pontos"][i + 1];
    
    tela.fillTriangle(cx, cy, ponto1["x"], ponto1["y"], ponto2["x"], ponto2["y"], cor);
  }
  
  int Tam_Lista = elemento["pontos"].size();
  tela.fillTriangle(cx,cy,elemento["pontos"][0]["x"],elemento["pontos"][0]["y"],elemento["pontos"][Tam_Lista-1]["x"],elemento["pontos"][Tam_Lista-1]["y"],cor);

}



void setup()
{
  Serial.begin(115200);
  Serial.println("Começou!");

  tela.begin();
  tela.fillScreen(ILI9341_BLACK);
  touch.begin(40);

  JsonDocument retangulo;
  deserializeJson(retangulo, "{\"tipo\":\"retângulo\",\"x\":20,\"y\":20,\"comprimento\":200,\"altura\":130,\"cor\":992}");
  desenharRetangulo(retangulo);

  JsonDocument poligono;
  deserializeJson(poligono, "{\"tipo\":\"polígono\",\"centro\": {\"x\": 120, \"y\": 85},\"pontos\":[{\"x\":120,\"y\":40},{\"x\":40,\"y\":85},{\"x\":120,\"y\":130},{\"x\":200,\"y\":85}],\"cor\":65504}");
  desenharPoligono(poligono);

  JsonDocument circulo;
  deserializeJson(circulo, "{\"tipo\":\"círculo\",\"x\":120,\"y\":85,\"raio\":26,\"cor\":31}");
  desenharCirculo(circulo);
}

void loop()
{


  
}