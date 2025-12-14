#include <Arduino.h>
#include <Adafruit_BME680.h>
#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <CayenneLPP.h>
#include <String.h>

Adafruit_BME680 sensorBME;

int count = 0;
unsigned long instanteAnterior = 0;
unsigned long instanteAnterior_2 = 0;
unsigned long instanteAnterior_3 = 0;

U8G2_FOR_ADAFRUIT_GFX fontes;
GxEPD2_290_T94_V2 modeloTela(10, 14, 15, 16);
GxEPD2_BW<GxEPD2_290_T94_V2, GxEPD2_290_T94_V2::HEIGHT> tela(modeloTela);

CayenneLPP dados(50);

int pinoParaAcordar = 4;

void ler_e_printar()
{
  sensorBME.performReading();
  float temperatura = sensorBME.temperature;                 // ˚C
  float pressao = sensorBME.pressure / 100.0;                // hPa
  float altitude = sensorBME.readAltitude(1013.25);          // m
  float umidade = sensorBME.humidity;                        // %
  float resistencia_gas = sensorBME.gas_resistance / 1000.0; // kΩ
}

String hexadecimalParaTexto(String textoHex)
{
  String resultado = "";
  textoHex.replace(" ",
                   "");
  for (int i = 0; i < textoHex.length(); i += 2)
  {
    String par = textoHex.substring(i, i + 2);
    char caractere = (char)strtol(par.c_str(), NULL, 16);
    resultado += caractere;
  }
  return resultado;
}

void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial1.begin(9600, SERIAL_8N1, 47, 48);
  Serial1.println("AT+JOIN");
  // Serial1.println("AT+DR=5");

  Serial.println("Projeto 07 – Protótipo");

  if (!sensorBME.begin())
  {
    Serial.println("Erro no sensor BME");
    while (true)
      ;
  }
  // aumenta amostragem dos sensores (1X, 2X, 4X, 8X, 16X ou NONE)
  sensorBME.setTemperatureOversampling(BME680_OS_8X);
  sensorBME.setHumidityOversampling(BME680_OS_2X);
  sensorBME.setPressureOversampling(BME680_OS_4X);
  sensorBME.setIIRFilterSize(BME680_FILTER_SIZE_3);
  sensorBME.setGasHeater(320, 150); // ˚C e ms, (0, 0) para desativar

  tela.init();
  tela.setRotation(3);
  tela.fillScreen(GxEPD_WHITE);
  tela.display(true);

  fontes.begin(tela);
  fontes.setForegroundColor(GxEPD_BLACK);

  pinMode(pinoParaAcordar, INPUT);
  esp_sleep_enable_ext0_wakeup((gpio_num_t) pinoParaAcordar, HIGH);

}

void loop()
{

  unsigned long instanteAtual = millis();
  if (instanteAtual > instanteAnterior + 5000 && count == 0)
  {
    Serial.println("+5 segundo");
    instanteAnterior = instanteAtual;

    fontes.setFont(u8g2_font_open_iconic_all_4x_t);
    fontes.setFontMode(1);

    tela.fillRect(10, 30, 10, 15, GxEPD_BLACK);
    tela.drawRect(10, 15, 10, 15, GxEPD_BLACK);

    fontes.drawGlyph(100, 40, 0x98);
    fontes.drawGlyph(200, 40, 0x8d);

    sensorBME.performReading();

    double temperatura = sensorBME.temperature; // ˚C
    double pressao = sensorBME.pressure / 100.0;
    double umidade = sensorBME.humidity; // %
    double resistencia_gas = sensorBME.gas_resistance / 1000.0;

    String temperatura_texto = String(temperatura) + "˚C";
    String umidade_texto = String(umidade) + "%";
    String pressao_texto = String(pressao) + "hPa";

    fontes.setFont(u8g2_font_helvB12_te);
    fontes.setFontMode(1);
    fontes.setCursor(20, 40);
    fontes.print(temperatura_texto);

    fontes.setCursor(140, 40);
    fontes.print(umidade_texto);

    fontes.setCursor(230, 30);
    fontes.print(pressao_texto);

    tela.display(true);
    count += 1;

    dados.addTemperature(1, temperatura);
    dados.addPresence(1, true);
    dados.addBarometricPressure(1, pressao);
    dados.addRelativeHumidity(1, umidade);
    dados.addAnalogInput(1, resistencia_gas);

    uint8_t *buffer = dados.getBuffer();
    String mensagem = "";
    for (int i = 0; i < dados.getSize(); i++)
    {
      if (buffer[i] < 16)
      {
        mensagem += "0";
      }
      mensagem += String(buffer[i], HEX);
    }
    mensagem.toUpperCase();
    Serial.println(mensagem);
    String mensagem_no_formato = "AT+SENDB=1:" + mensagem;
    Serial1.println(mensagem_no_formato);
    dados.reset();
  }

  if (Serial1.available() > 0)
  {
    String texto = Serial1.readStringUntil('\n');
    Serial.println(texto);
    texto.trim();
    Serial.println("Resposta do módulo LoRaWAN: " + texto);
    Serial.println(texto.substring(0, 2));
    if (texto.substring(0, 2) == "RX")
    {
      int dois_pontos_um = texto.indexOf(":") + 1;
      int dois_pontos_dois = texto.indexOf(":", dois_pontos_um);
      String texto_em_Hexadecimal = texto.substring(dois_pontos_um, dois_pontos_dois);

      String resultado_em_texto = hexadecimalParaTexto(texto_em_Hexadecimal);
      String Resposta_no_formato_para_print = "Resposta em texto: " + resultado_em_texto;
      Serial.println(Resposta_no_formato_para_print);

      fontes.setFont(u8g2_font_helvB24_te);
      fontes.setFontMode(1);
      fontes.setCursor(10,80);
      fontes.print(resultado_em_texto);
      tela.display(true); // SEMPRE CHAMAR NO FINAL!
    }
  }

  if (instanteAtual > instanteAnterior_2 + 10000)
  {
    Serial.println("+ 10segundos");
    esp_deep_sleep_start();
    instanteAnterior_2 = instanteAtual;

  }

  if (instanteAtual > instanteAnterior_3 + 15000)
  {
    Serial.println("+ 15 segundos");
    digitalWrite(pinoParaAcordar,HIGH);
    Serial.println("+ 15 segundos");
    instanteAnterior_3 = instanteAtual;

  }

}