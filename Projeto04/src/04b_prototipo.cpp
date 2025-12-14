#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <SQLiteManager.h>
#include <SD_MMC.h>
#include <ArduinoJson.h>
#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <GxEPD2_BW.h>
#include <BarcodeGFX.h>
#include <QRCodeGFX.h>
#include <GFButton.h>

U8G2_FOR_ADAFRUIT_GFX fontes;
GxEPD2_290_T94_V2 modeloTela(10, 14, 15, 16);
GxEPD2_BW<GxEPD2_290_T94_V2, GxEPD2_290_T94_V2::HEIGHT> tela(modeloTela);

void setup()
{
  tela.init();
  tela.setRotation(3);
  tela.fillScreen(GxEPD_WHITE);
  tela.display(true);

  fontes.begin(tela);
  fontes.setForegroundColor(GxEPD_BLACK);

  fontes.setFont(u8g2_font_helvB18_te);
  fontes.setFontMode(1);
  fontes.setCursor(40, 30);
  fontes.print("Bem vindooo!");

  fontes.setFont(u8g2_font_helvB12_te);
  fontes.setFontMode(1);
  fontes.setCursor(30, 80);
  fontes.print("Passe o produto no leitor");

  tela.display(true);

  Serial1.begin(9600, SERIAL_8N1, 47, 48);
  Serial1.println("~M00910001.");
  delay(100);
  Serial1.println("~M00210001.");
  delay(100);
  Serial1.println("~M00B00014.");
}

void loop()
{

  if (Serial1.available() > 0)
  {
    String texto = Serial1.readStringUntil('\n');
    texto.trim();
    if (texto.length() > 5)
    {
      Serial.println("Resposta do leitor: " + texto);
      long long code = strtoll(texto.c_str(), nullptr, 10);
    }
  }

  tela.display(true);
}