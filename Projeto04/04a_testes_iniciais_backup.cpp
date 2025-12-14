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

int id_prova = 1;
int numero = 3;

void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial1.begin(9600, SERIAL_8N1, 47, 48);

  tela.init();
  tela.setRotation(3);
  tela.fillScreen(GxEPD_WHITE);

  codigoBarras.setScale(2);
  codigoBarras.draw("7891528038810", 2, 40, 60);
  tela.display(true);

  qrcode.setScale(2);
  qrcode.draw("http://janks.link/produto.", 220, 20);

  Serial1.println("~M00910001."); // habilita comandos serial
  delay(100);
  Serial1.println("~M00210001."); // escaneamento contínuo
  delay(100);
  Serial1.println("~M00B00014."); // Atraso para ler o mesmo produto.

  SD_MMC.setPins(39, 38, 40);
  if (!SD_MMC.begin("/sdcard", true))
  {
    Serial.println("Falha na inicialização do cartão SD!");

    while (true)
      ; // trava programa aqui em caso de erro
  }
  try
  {
    banco.open("/sdcard/banco.db");
  }
  catch (const std::exception &e)
  {
    Serial.println(e.what());
    while (true)
      ; // trava programa aqui em caso de erro
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
    { // ignora respostas estranhas
      Serial.println("Resposta do leitor: " + texto);
      long long codigo = strtoll(texto.c_str(), nullptr, 10);
      try
      {
        JsonDocument resultados = banco.execute("SELECT * FROM Produtos WHERE codigo = ?", codigo);

        for (int i = 0; i < resultados.size(); i++)
        {
          JsonDocument Tabela_produto = resultados[i];
          int preco = Tabela_produto["preco"];

          String texto = Tabela_produto["nome"];
          String preco_em_texto = String(preco);
          preco_em_texto[-2]= ',';
          preco_em_texto[0] = 'R';
          preco_em_texto[1] = '$';
          Serial.println(texto);
    
          Serial.println(preco_em_texto);
        }
      }

      catch (const std::exception &e)
      {
        Serial.println(e.what());
      }
    }
  }

  tela.display(true);
}
