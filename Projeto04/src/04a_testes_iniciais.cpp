#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <BarcodeGFX.h>
#include <QRCodeGFX.h>
#include <SQLiteManager.h>
#include <SD_MMC.h>
#include <ArduinoJson.h>

GxEPD2_290_T94_V2 modeloTela(10, 14, 15, 16);
GxEPD2_BW<GxEPD2_290_T94_V2, GxEPD2_290_T94_V2::HEIGHT> tela(modeloTela);
BarcodeGFX codigoBarras(tela);
QRCodeGFX qrcode(tela);

SQLiteManager banco;

void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial1.begin(9600, SERIAL_8N1, 47, 48);
  Serial1.println("~M00910001.");
  delay(100);
  Serial1.println("~M00210001.");
  delay(100);
  Serial1.println("~M00B00014.");

  tela.init();
  tela.setRotation(3);
  tela.fillScreen(GxEPD_WHITE);
  tela.display(true);

  codigoBarras.setScale(2);
  codigoBarras.draw("7891528038810", 5, 20, 70);

  qrcode.setScale(2);
  tela.display(true);
  qrcode.draw("https://janks.link/produto", 220, 20);

 

  SD_MMC.setPins(39, 38, 40);
  if (!SD_MMC.begin("/sdcard", true))
  {
    Serial.println("Erro SD Card");
    while (true)
    {
    };
  }
  try
  {
    banco.open("/sdcard/banco.db");
  }
  catch (const std::exception &e)
  {
    Serial.println(e.what());
    while (true)
      ;
  }

   tela.display(true);

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
      Serial.println(code);
      try
      {
        JsonDocument resultados = banco.execute("SELECT *FROM Produtos WHERE codigo = ?", code);

        for (int i=0; i< resultados.size(); i++)
        {
          JsonDocument Tabela_produto = resultados[i];
          float preco = Tabela_produto["preco"];
          preco = preco/100;

          String nome_em_texto = Tabela_produto["nome"];
          String preco_em_texto = String(preco);

          preco_em_texto = "R$ " + preco_em_texto;

          Serial.println(nome_em_texto);
          Serial.println(preco_em_texto);

        }

      }
        catch(const std::exception &e)
        {
          Serial.println(e.what());
        }


    }
  }

   tela.display(true);
}