// In a webbrowser go to http://sam.local/say/{message} to make it speak
// ex: http://sam.local/say/hello world

#include <Arduino.h>

#include <ESPSAM.h>
#include <AudioOutputI2S.h>

// #include <ESP8266mDNS.h>
// #include <ESP8266NetBIOS.h>

#ifdef ESP32
#include <WiFi.h>
#endif
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#endif

#include <WiFiManager.h>
WiFiManager _wifiManager;

#include <uri/UriBraces.h>

AudioOutputI2S *out = NULL;

#ifdef ESP8266
ESP8266WebServer server(80);
#endif
#ifdef ESP32
WebServer server(80);
#endif// const char *NAME = "SAM";

String urldecode(String str);

void setup()
{
  Serial.begin(SERIAL_BAUD);

  if (!_wifiManager.autoConnect())
  {
    Serial.println("Failed to connect and hit timeout");
#ifdef ESP8266
    ESP.reset();
#endif
#ifdef ESP32
    ESP.restart();
#endif
  }
  else
  {
    Serial.println("\nConnected with: " + WiFi.SSID());
    Serial.println("IP Address: " + WiFi.localIP().toString());
  }

  out = new AudioOutputI2S();
  out->begin();

  // MDNS.begin(NAME);
  // MDNS.addService("http", "tcp", 80);
  // NBNS.begin(NAME);

  server.on(UriBraces("/say/{}"), []() {
    String message_encoded = server.pathArg(0);
    String message_decoded = urldecode(message_encoded);
    const char* message = message_decoded.c_str();

    Serial.println(message_encoded);
    Serial.println(message_decoded);
    Serial.println(message);
    
    ESPSAM *sam = new ESPSAM;
    sam->Say(out, message);
    delete sam;    
    server.send(200, "text/plain", "OK"); 
  });

  server.begin();
}

void loop()
{
  server.handleClient();
}

char *string2char(String command)
{
  if (command.length() != 0)
  {
    char *p = const_cast<char *>(command.c_str());
    return p;
  }
  else
  {
    return "";
  }
}

unsigned char h2int(char c)
{
  if (c >= '0' && c <= '9')
  {
    return ((unsigned char)c - '0');
  }
  if (c >= 'a' && c <= 'f')
  {
    return ((unsigned char)c - 'a' + 10);
  }
  if (c >= 'A' && c <= 'F')
  {
    return ((unsigned char)c - 'A' + 10);
  }
  return (0);
}

String urldecode(String str)
{

  String encodedString = "";
  char c;
  char code0;
  char code1;
  for (int i = 0; i < str.length(); i++)
  {
    c = str.charAt(i);
    if (c == '+')
    {
      encodedString += ' ';
    }
    else if (c == '%')
    {
      i++;
      code0 = str.charAt(i);
      i++;
      code1 = str.charAt(i);
      c = (h2int(code0) << 4) | h2int(code1);
      encodedString += c;
    }
    else
    {

      encodedString += c;
    }

    yield();
  }

  return encodedString;
}