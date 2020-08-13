#include<ESP8266WiFi.h>
void smartConfig()
{
  if(WiFi.status() == WL_CONNECTED)
  {
    WiFi.disconnect(false);
  }
  WiFi.mode(WIFI_STA);
  Serial.println("waiting for smartconfig");
  delay(2000);
  WiFi.beginSmartConfig();
  while(1)
  {
    Serial.print(".");
    Serial.print("read io4 :");
  Serial.println(digitalRead(4));
    delay(500);
    if(WiFi.smartConfigDone())
    {
      Serial.println("SmartConfig Success");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      WiFi.setAutoConnect(true);  // 设置自动连接
      break;
    }
   }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(4,HIGH);
}

void setup()
{
  Serial.begin(115200);
  pinMode(4,OUTPUT);
  digitalWrite(4,HIGH);
  pinMode(4,INPUT);
  //attachInterrupt(4, smartConfig, CHANGE);//当int.0电平改变时,触发中断函数blink
  
  //smartConfig();  
}

void loop()
{
  delay(100);
  if(WiFi.status() != WL_CONNECTED)
  {
    Serial.println("unconnect");
  }
  else
  {
    Serial.println("smartconfig success");
  }
  Serial.print("read io4 :");
  Serial.println(digitalRead(4));
  if(!digitalRead(4))
  {
    smartConfig();
    }
  delay(500);
}
