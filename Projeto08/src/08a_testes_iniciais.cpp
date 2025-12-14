#include <Matter.h>
#include <GFButton.h>
#include <Adafruit_BME680.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WebServer.h>

/*MatterColorLight luzRGBMatter;
MatterOccupancySensor movimentoMatter;
MatterHumiditySensor umidadeMatter;
MatterTemperatureSensor temperaturaMatter;
MatterThermostat termostatoMatter;

MatterGenericSwitch botaoMatter; */

GFButton sensor(21);

Adafruit_BME680 sensorBME;

float temperatura;
float umidade;

GFButton botao_cinco(5);

int count = 0;

unsigned long instanteAnterior = 0;

Preferences preferencias;

WebServer servidor(80);

bool mudouLuz(bool acender, espHsvColor_t corHSV)
{
  espRgbColor_t corRGB = espHsvColorToRgbColor(corHSV);
  // acende ou apaga a sua luz de acordo com variável acender
  if (acender == true)
  {
    rgbLedWrite(RGB_BUILTIN, corRGB.r, corRGB.g, corRGB.b);
    acender = false;
  }
  else
  {
    rgbLedWrite(RGB_BUILTIN, 0, 0, 0);
    acender = true;
  }

  return true;
}

void movimento(GFButton &sensor)
{
  Serial.println("Movimento detectado!");
  // movimentoMatter.setOccupancy(true);
}

void inercia(GFButton &sensor)
{
  Serial.println("Inércia detectada!");
  // movimentoMatter.setOccupancy(false);
}

/*void recomissionarMatter()
{
  if (!Matter.isDeviceCommissioned())
  {
    Serial.println("Procurando ambiente Matter");
    Serial.printf("Código de pareamento: %s\r\n", Matter.getManualPairingCode().c_str());
    Serial.printf("Site com QR code de pareamento: %s\r\n", Matter.getOnboardingQRCodeUrl().c_str());
    Serial.print("Procurando ambiente Matter...");
    while (!Matter.isDeviceCommissioned())
    {
      Serial.print(".");
      delay(1000);
    }
    Serial.println("\nConectado no ambiente Matter!");
  }
} */

void botaoPressionado(GFButton &botaoDoEvento)
{
  Serial.println("Botão foi pressionado!");
  count += 1;
  Serial.print("Contagem: ");
  Serial.println(count);
  preferencias.putInt("contagem", count);
}

void botaoSolto(GFButton &botaoDoEvento)
{
  Serial.println("Botão foi solto!");
}

void pagina1()
{
  servidor.send(200, "text/html", "João Gabriel!");
}

void setup()
{
  Serial.begin(115200);
  delay(500);

  // movimentoMatter.begin();
  // temperaturaMatter.begin();
  // luzRGBMatter.begin();
  // umidadeMatter.begin();
  // termostatoMatter.begin();

  // botaoMatter.begin();
  // luzRGBMatter.onChange(mudouLuz);

  sensor.setPressHandler(inercia);
  sensor.setReleaseHandler(movimento);

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
  sensorBME.setGasHeater(320, 150); // ˚C e ms, (0, 0) para desativar

  botao_cinco.setPressHandler(botaoPressionado);
  botao_cinco.setReleaseHandler(botaoSolto);

  preferencias.begin("JG_Cunha"); // nome único seu

  count = preferencias.getInt("contagem", 0);

  WiFi.softAP("Joao_Gabriel_Cunha", "12345678");
  Serial.println("WiFi Access Point iniciado!");
  Serial.print("IP: " + WiFi.softAPIP().toString());

  servidor.on("/pagina1", HTTP_GET, pagina1);
  servidor.begin();

  // Matter.begin();
  // recomissionarMatter();
}

void loop()
{

  sensor.process();

  unsigned long instanteAtual = millis();
  if (instanteAtual > instanteAnterior + 10000)
  {

    Serial.println("+10 segundo");

    sensorBME.performReading();
    temperatura = sensorBME.temperature;
    umidade = sensorBME.humidity;

    // temperaturaMatter.setTemperature(temperatura);
    // umidadeMatter.setHumidity(umidade);

    instanteAnterior = instanteAtual;
  }

  botao_cinco.process();

  servidor.handleClient();
  int numeroDeConexoes = WiFi.softAPgetStationNum();

  // recomissionarMatter();
}