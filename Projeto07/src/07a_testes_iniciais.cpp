#include <Arduino.h>
#include <CayenneLPP.h>
#include <Adafruit_BME680.h>
#include <GFButton.h>

CayenneLPP dados(50);
Adafruit_BME680 sensorBME;

float temperatura;
float pressao;
float altitude;
float umidade;
float resistencia_gas;

unsigned long instanteAnterior = 0;

GFButton botao(1);
GFButton sensor(21);
GFButton botao_meio(5);

int pinoParaAcordar = 21;

void ler_e_printar_dados()
{
  sensorBME.performReading();

  temperatura = sensorBME.temperature;
  umidade = sensorBME.humidity;
  pressao = sensorBME.pressure / 100.0;
  resistencia_gas = sensorBME.gas_resistance / 1000.0;

  String temperatura_texto = String(temperatura) + "˚C";
  String umidade_texto = String(umidade) + "de umidade";
  String pressao_texto = String(pressao) + "hPa";
  String resistencia_gas_texto = String(resistencia_gas) + "kOhm";

  Serial.println(temperatura_texto);
  Serial.println(umidade_texto);
  Serial.println(pressao_texto);
  Serial.println(resistencia_gas_texto);
  Serial.println("\n");
}

void botaoPressionado(GFButton &botaoDoEvento)
{
  Serial.println("Botão foi pressionado!");
  esp_deep_sleep_start();
}

void botaoSolto(GFButton &botaoDoEvento)
{
  Serial.println("Botão foi solto!");
}

void botao_meio_Pressionado(GFButton &botaoDoEvento)
{
  Serial.println("Botão do meio foi pressionado!");
  Serial1.println("AT+SEND=1: João Gabriel da Cunha Vasconcellos"); // porta : texto
 

}
void botao_meio_Solto(GFButton &botaoDoEvento)
{
  Serial.println("Botão do meio foi solto!");
}

void movimento(GFButton &sensor)
{
  Serial.println("Movimento detectado!");
}

void inercia(GFButton &sensor)
{
  Serial.println("Inércia detectada!");
}

void setup()
{

  Serial1.begin(9600, SERIAL_8N1, 47, 48);

  Serial.begin(115200);
  delay(500);
  Serial.println("Projeto 07 – Testes Iniciais");

  Serial1.println("AT+JOIN");

  if (!sensorBME.begin())
  {
    Serial.println("Erro no sensor BME");
    while (true)
      ;
  }

  sensorBME.setTemperatureOversampling(BME680_OS_8X);
  sensorBME.setHumidityOversampling(BME680_OS_2X);
  sensorBME.setPressureOversampling(BME680_OS_4X);
  sensorBME.setIIRFilterSize(BME680_FILTER_SIZE_3);
  sensorBME.setGasHeater(320, 150); // ˚C e ms, (0, 0)

  botao.setPressHandler(botaoPressionado);
  botao.setReleaseHandler(botaoSolto);

  sensor.setPressHandler(inercia);
  sensor.setReleaseHandler(movimento);

  // pinMode(pinoParaAcordar, INPUT);
  // esp_sleep_enable_ext0_wakeup((gpio_num_t)pinoParaAcordar, HIGH);

  botao_meio.setPressHandler(botao_meio_Pressionado);
  botao_meio.setReleaseHandler(botao_meio_Solto);
}

void loop()
{

  unsigned long instanteAtual = millis();
  if (instanteAtual > instanteAnterior + 5000)
  {
    Serial.println("+5 segundos");
    instanteAnterior = instanteAtual;
    ler_e_printar_dados();
  }

  botao.process();
  sensor.process();


  botao_meio.process();
}