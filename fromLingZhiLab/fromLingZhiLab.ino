void ICACHE_RAM_ATTR interrput_io4();
#include<ESP8266WiFi.h>
bool wifi_config=false;
//中断里卡太久会溢出。

void setup()
{
  Serial.begin(115200);
  //pinMode(4,INPUT);
  pinMode(4, INPUT);
  //attachInterrupt(4, smartConfig, CHANGE);//当int.0电平改变时,触发中断函数blink
  attachInterrupt(digitalPinToInterrupt(4), interrput_io4, FALLING);
  //smartConfig();  
}
void loop()
{
  //delay(100);
  if(WiFi.status()!= WL_CONNECTED)
  {
    Serial.println("unconnect");
  }
  else
  {
    Serial.println(WiFi.localIP());
    Serial.println("smartconfig success");
  }
  Serial.print("read io4 :");
  Serial.println(digitalRead(4));
  if(wifi_config)
  {
    detachInterrupt(4);
    smartConfig();
    attachInterrupt(digitalPinToInterrupt(4), interrput_io4, FALLING);
  }
  delay(500);
}
void interrput_io4()
{
    Serial.print("Enter the interrput digitalPinToInterrupt(4)");
    Serial.println(digitalPinToInterrupt(4));
    wifi_config=true;
}
void smartConfig()
{
  
  //if(WiFi.status() == WL_CONNECTED)
  //{
    //WiFi.disconnect(false);//造成第一次连接失败后，以后的连接会使用上次连接失败的密码账号。
  //}
  delay(1000);
  int count=0;
  //持续3秒低电平
  if(!digitalRead(4)){
    while(count>=40)
    {
      delay(100);
      if(digitalRead(4))
        {
          Serial.println("less than 3 second");
          wifi_config=false;
          return;
        }
        ++count;
    }
  }
  Serial.println("enter smartconfig mode!");
  WiFi.mode(WIFI_STA);
  Serial.println("waiting for smartconfig");
  
  //delay(2000);
  WiFi.beginSmartConfig();
  while(1)
  {
    Serial.print(".");
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
  Serial.print("Wi-Fi connecting");
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(300);
    Serial.printf(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  
  Serial.println(WiFi.localIP());
  WiFi.stopSmartConfig(); //向手机反馈连接成功
  wifi_config=false;
  //digitalWrite(4,HIGH);
}
