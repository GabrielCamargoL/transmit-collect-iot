// Crie um software embarcado que realize as seguintes funções:
// -conecte-se ao wifi
// -sincronize a data e a hora
// -colete e transmita informações a cada 5 minutos, em horário múltiplo de 5 (00:00, 00:05, 00:10, 00:15...)

#include <WiFi.h>
#include <HTTPClient.h>
#include "time.h"

TaskHandle_t tTask1;

struct tm data;

char *ssid = "Gabs";
char *pwd = "teste123";

char *ntpServer = "pool.ntp.org";
long gmtOffset_sec = 0;
int daylightOffset_sec = 0;

struct tm timeinfo;
time_t now;

String serverName = "http://postman-echo.com/";

boolean reqTime5Already = false;

void connectWifi()
{
  Serial.println("Conectando...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" ");
  Serial.print("Conectado ao WiFi com o IP: ");
  Serial.println(WiFi.localIP());
}

void minhaTask1(void *pvParameters){
  Serial.print("Minha task1 está no core ");
  Serial.println(xPortGetCoreID());

  while (true){
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    if (!getLocalTime(&timeinfo))
    {
      Serial.println("Acesso ao ntp falhou");
    }
    delay(10000);
  }
}

void setup()
{
  Serial.begin(115200);

  WiFi.begin(ssid, pwd);
  connectWifi();
  
  xTaskCreatePinnedToCore(
    minhaTask1, //task function
    "MinhaTask1", //task name
    10000, //Task stack size
    NULL, //task parameters
    1, //task priority
    &tTask1, //task handle
    0); // task core (core 1=loop)
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED) {
    if (!getLocalTime(&timeinfo)){
      Serial.println("Acesso ao ntp falhou");
    }

    if(timeinfo.tm_min % 5 != 0 && reqTime5Already == true) {
      reqTime5Already = false;
    } 
    
    if(timeinfo.tm_min % 5 != 0 || reqTime5Already == true) {
      Serial.print(" ");
    } else {
      Serial.println("\n==============================");
      Serial.print(timeinfo.tm_hour);
      Serial.print("h ");     
      Serial.print(timeinfo.tm_min);
      Serial.print("min "); 
      Serial.print(timeinfo.tm_sec);
      Serial.print("seg ");
      Serial.println(" "); 

      ////HTTP POST
      WiFiClient client;
      HTTPClient http_post;
      //"//http://postman-echo.com/post"
      String url = serverName + "post";
      http_post.begin(client, url);
      Serial.println("\nPOST");
      http_post.addHeader("Content-Type", "application/json");
      http_post.addHeader("x-api-key", "qwertyuiopasdfghjklzxcvbnm");
      String data = "{\"nome\":38972142176126, \"temp\":24.5, \"umi\":80}";
      
      int httpCode = http_post.POST(data);
      if (httpCode > 0) {
        Serial.println(httpCode);
        String payload = http_post.getString();
        Serial.print("Resposta do server: ");
        Serial.println(payload);
        Serial.println("==============================\n"); 
        reqTime5Already = true;
      } else {
        Serial.println("Http error");
      }
    }
  } else {
    Serial.println("Na FATEC nunca tem internet");
    connectWifi();
  }

  delay(28000);
}
