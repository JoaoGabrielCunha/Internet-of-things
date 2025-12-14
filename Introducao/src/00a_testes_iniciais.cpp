#include <Arduino.h>
#include <GFButton.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

int count = 0;
unsigned long instanteAnterior = 0;
Adafruit_ILI9341 tela(53, 49, 48);
bool cor = false;

GFButton botao(11);

void botaoPressionado(GFButton &botaoDoEvento)
{
  if (cor == false)
  {
    cor = true;
    tela.fillCircle(150,70,10,ILI9341_YELLOW);
  }
  else
  {
    cor = false;
    tela.fillCircle(150,70,10,ILI9341_BLUE);
  }


}

void setup(void)
{
  Serial.begin(9600);

  tela.begin();
  tela.setCursor(20, 100);
  tela.setTextColor(ILI9341_YELLOW);
  tela.setTextSize(4);

  botao.setPressHandler(botaoPressionado);
}

void loop()
{
  unsigned long instanteAtual = millis();

  if (instanteAtual > instanteAnterior + 300)
  {
    instanteAnterior = instanteAtual;
    count += 1;
    if (count <= 20)
    {
      Serial.println(count);
    }
  }

  if (Serial.available() > 0)
  {
    tela.fillRect(20, 100, 240, 120, ILI9341_BLACK);
    String texto = Serial.readStringUntil('\n');
    tela.setCursor(20, 100);
    tela.print(texto);
  }

  botao.process();

}