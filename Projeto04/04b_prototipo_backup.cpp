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

float totalDaCompra;
SQLiteManager banco;
int count = 0;
float totalDaCompra_formatado;
String totalDaCompra_formatado_String;

JsonDocument lista_produtos;
int x_t, y_t, x_n, y_n;

BarcodeGFX codigoBarras(tela);
QRCodeGFX qrcode(tela);

String inicio = "000201";
String prefixoPix = "0014BR.GOV.BCB.PIX";
String moeda = "5303986";                  // R$
String pais = "5802BR";                    // Brasil
String categoriaBeneficiario = "52040000"; // código MCC não definido
String dadosAdicionais = "62070503***";

String nomeBeneficiario = "5912Joao Gabriel";     // máximo 25 letras
String cidadeBeneficiario = "6014RIO DE JANEIRO"; // máximo 15 letras
String chaveBeneficiario = "011112082869784";    // ou email, CPF, etc

String chaveCompleta = prefixoPix + chaveBeneficiario;
String prefixoChave = "26" + String(chaveCompleta.length());

GFButton botao_5(5);


String adicionaCRC(String dados)
{
  String dadosComPrefixoCRC = dados + "6304";
  const char *dadosChar = dadosComPrefixoCRC.c_str();
  uint16_t crc = 0xFFFF;
  while (*dadosChar)
  {
    crc ^= (uint8_t)(*dadosChar++) << 8;
    for (uint8_t i = 0; i < 8; i++)
    {
      if (crc & 0x8000)
      {
        crc = (crc << 1) ^ 0x1021;
      }
      else
      {
        crc = crc << 1;
      }
    }
  }
  char codigoFinal[512];
  snprintf(
      codigoFinal, sizeof(codigoFinal),
      "%s%04X", dadosComPrefixoCRC.c_str(), crc);
  return String(codigoFinal);
}

void percorre_lista_printa_ultimos_5()
{
  x_t = 3;
  y_t = 16;

  x_n = 200;
  y_n = 16;

  int i = 0;
  if (lista_produtos.size() < 5)
  {
    i = 0;
  }
  else
  {
    i = lista_produtos.size() - 5;
  }

  for (; i < lista_produtos.size(); i++)
  {

    String nome_parcial = lista_produtos[i]["nome"];
    double preco_parcial_double = lista_produtos[i]["preco"];
    preco_parcial_double = preco_parcial_double / 100;

    String preco_parcial_string = String(preco_parcial_double);

    nome_parcial = nome_parcial.substring(0, 24);

    preco_parcial_string = "R$ " + preco_parcial_string;

    fontes.setFont(u8g2_font_helvR12_te);

    fontes.setFontMode(1);
    fontes.setCursor(x_t, y_t);
    fontes.print(nome_parcial);

    fontes.setFontMode(1);
    fontes.setCursor(x_n, y_n);
    fontes.print(preco_parcial_string);

    y_t += 16;
    y_n += 16;
  }
  tela.display(true);
  y_t = 16;
  y_n = 16;
}

void botao5Pressionado(GFButton &botao_5)
{
  Serial.println("Botao 5 foi pressionado");
  tela.fillScreen(GxEPD_WHITE);
  // em alguma função do código...
  // ajuste o total de caracteres (COM 2 DÍGITOS)!
  
  totalDaCompra_formatado = totalDaCompra / 100;
  totalDaCompra_formatado_String = String(totalDaCompra_formatado);
  Serial.println(totalDaCompra_formatado_String);
  String Tamanho_Compra = String(totalDaCompra_formatado_String.length());

  String valor =  "540" + Tamanho_Compra + totalDaCompra_formatado_String ;



  String codigoPix = inicio + prefixoChave + chaveCompleta +
                     categoriaBeneficiario + moeda + valor + pais +
                     nomeBeneficiario + cidadeBeneficiario + dadosAdicionais;
  codigoPix = adicionaCRC(codigoPix);
  Serial.println("Código Pix: " + codigoPix);
  qrcode.setScale(3);           // valor de escala inteiro ≥ 1
  qrcode.draw(codigoPix, 0, 0); // x, y
  tela.display(true);
}

void botao5Solto(GFButton &botao5)
{
  Serial.println("Botão 5 foi solto!");
}

void setup()
{
  x_t = 3;
  y_t = 16;

  x_n = 200;
  y_n = 16;

  Serial.begin(115200);
  delay(500);

  tela.init();
  tela.setRotation(3);
  tela.fillScreen(GxEPD_WHITE);
  tela.display(true);

  fontes.begin(tela);
  fontes.setForegroundColor(GxEPD_BLACK);

  fontes.setFont(u8g2_font_helvB18_te);
  fontes.setFontMode(1);
  fontes.setCursor(40, 30);
  fontes.print("Bem vindo!");

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

  botao_5.setPressHandler(botao5Pressionado);
  botao_5.setReleaseHandler(botao5Solto);
}

void loop()
{

  botao_5.process();

  if (Serial1.available() > 0)
  {
    count += 1;
    String texto = Serial1.readStringUntil('\n');
    texto.trim();
    if (texto.length() > 5)
    {
      Serial.println("Resposta do leitor: " + texto);
      long long codigo = strtoll(texto.c_str(), nullptr, 10);
      try
      {
        JsonDocument resultados = banco.execute("SELECT * FROM produtos WHERE codigo = ?", codigo);
        tela.fillScreen(GxEPD_WHITE);
        for (int i = 0; i < resultados.size(); i++)
        {
          JsonDocument resultado_parcial = resultados[i];

          lista_produtos.add(resultado_parcial);

          int compra = resultado_parcial["preco"];
          totalDaCompra += compra;

          Serial.println(totalDaCompra);
          codigoBarras.setScale(1);
          codigoBarras.draw(codigo, 3, 84, 40);
          x_t = 200;
          y_t = 100;
          fontes.setFontMode(1);
          fontes.setCursor(x_t, y_t);
          totalDaCompra_formatado = totalDaCompra / 100;
          totalDaCompra_formatado_String = String(totalDaCompra_formatado);
          totalDaCompra_formatado_String = "R$" + totalDaCompra_formatado_String;
          fontes.print(totalDaCompra_formatado_String);
          tela.display(true);
        }
      }

      catch (const std::exception &e)
      {
        Serial.println(e.what());
      }
    }
    percorre_lista_printa_ultimos_5();
  }

  tela.display(true);
}
