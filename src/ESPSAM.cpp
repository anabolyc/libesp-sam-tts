/*
  ESPSAM
  Port of SAM to the ESP8266
  
  Copyright (C) 2017  Earle F. Philhower, III
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <Arduino.h>
#include <ESPSAM.h>

#include "reciter.h"
#include "sam.h"
#include "SamData.h"

String numbers10[10] = { "ten", "eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen", "seventeen", "eight teen", "nine teen" };
String numbers100[8] = { "twenty", "thirty", "forty", "fifty", "sixty", "sevety", "eight tee", "nine tee"};

SamData* samdata;

// Thunk from C to C++ with a this-> pointer
void ESPSAM::OutputByteCallback(void *cbdata, unsigned char b)
{
  ESPSAM *sam = static_cast<ESPSAM*>(cbdata);
  sam->OutputByte(b);
}

void ESPSAM::OutputByte(unsigned char b)
{
  // Xvert unsigned 8 to signed 16...
  int16_t s16 = b;// s16 -= 128; //s16 *= 128;
  int16_t sample[2];
  sample[0] = s16;
  sample[1] = s16;
  while (!output->ConsumeSample(sample)) yield();
}

bool ESPSAM::Say(AudioOutput *out, const uint32_t number) 
{
  bool result = true;
  if (number < 10) {
    result = Say(out, String(number).c_str());
  }
  else if (number < 20) {
    result = Say(out, numbers10[number - 10].c_str());
  } else if (number < 100) {
    uint8_t a = number % 10;
    uint8_t b = (number - a) / 10;
    result &= Say(out, numbers100[b - 2].c_str());
    if (a != 0) {
      result &= Say(out, String(a).c_str());
    }
  } else {
    uint8_t h = number / 100;
    result &= Say(out, String(h).c_str());
    result &= Say(out, "hundred");
    uint8_t left = number - h * 100;
    if (left != 0) {
      result &= Say(out, left);
    }
  }

  return result;
}

bool ESPSAM::Say(AudioOutput *out, const char *str)
{
  if (!str || strlen(str)>254) return false; // Only can speak up to 1 page worth of data...
  samdata = new SamData;
  if (samdata == nullptr)
  {
      // allocation failed!
      return false;
  }
  
  // These are fixed by the synthesis routines
  out->SetRate(22050);
  out->SetBitsPerSample(8);
  out->SetChannels(1);
  out->begin();

  // SAM settings
  EnableSingmode(singmode);
  if (speed) ::SetSpeed(speed);
  if (pitch) ::SetPitch(pitch);
  if (mouth) ::SetMouth(mouth);
  if (throat) ::SetThroat(throat);

  // Input massaging
  char input[256];
  for (int i=0; str[i]; i++)
    input[i] = toupper((int)str[i]);
  input[strlen(str)] = 0;

  // To phonemes
  if (phonetic) {
    strncat(input, "\x9b", 255);
  } else {
    strncat(input, "[", 255);
    if (!TextToPhonemes(input)) return false; // ERROR
  }

  // Say it!
  output = out;
  SetInput(input);
  SAMMain(OutputByteCallback, (void*)this);
  delete samdata;
  return true;
}

void ESPSAM::SetVoice(enum SAMVoice voice)
{
  switch (voice) {
    case VOICE_ELF: SetSpeed(72); SetPitch(64); SetThroat(110); SetMouth(160); break;
    case VOICE_ROBOT: SetSpeed(92); SetPitch(60); SetThroat(190); SetMouth(190); break;
    case VOICE_STUFFY: SetSpeed(82); SetPitch(72); SetThroat(110); SetMouth(105); break;
    case VOICE_OLDLADY: SetSpeed(82); SetPitch(32); SetThroat(145); SetMouth(145); break;
    case VOICE_ET: SetSpeed(100); SetPitch(64); SetThroat(150); SetMouth(200); break;
    default:
    case VOICE_SAM: SetSpeed(72); SetPitch(64); SetThroat(128); SetMouth(128); break;
  }
}

