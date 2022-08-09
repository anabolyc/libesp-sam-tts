#include <Arduino.h>
#include <ESPSAM.h>
#include <AudioOutputI2S.h>

AudioOutputI2S *out = new AudioOutputI2S();
ESPSAM *sam = new ESPSAM;

uint8_t counter = 0;

void setup()
{
  out->begin();
}

void loop()
{
  sam->Say(out, ++counter);
  delay(500);
}
