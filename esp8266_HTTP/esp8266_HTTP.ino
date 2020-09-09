#include<ESP8266WiFi.h>
#include<WiFiClient.h>
#include <ESP8266HTTPClient.h>
//需要设置的服务器地址以及端口
#define SERVER_ADDR "http://www.lingzhilab.com/resources/getAllResPage"
#define SERVER_PORT "80"
//用户私钥，可在控制台获取,修改为自己的UID
String UID = "esp8266";

#define httpTime 2*1000
//设置心跳
#define upheartTime 30*1000
int intNumber = 0;

//TCP客户端初始化
WiFiClient TCPclient;
String TcpClient_Buff="";
unsigned int TcpClient_BuffIndex = 0;
unsigned long TcpClient_preTick = 0;
unsigned long preHeartTick = 0;//心跳
unsigned long predataTick = 0;//数据发送时间
unsigned long getinterval=0;
//最大字节数
#define MAX_PACKETSIZE 512

void ICACHE_RAM_ATTR interrput_io4();
bool wifi_config=false;

//函数声明
void smartConfig();
void interrput_io4();
//TCP初始化连接
void doTCPClientTick();
void startTCPClient();
void sendtoTCPServer(String p);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  //pinMode(4,INPUT);
  pinMode(4, INPUT);
  //attachInterrupt(4, smartConfig, CHANGE);//当int.0电平改变时,触发中断函数blink
  attachInterrupt(digitalPinToInterrupt(4), interrput_io4, FALLING);
}
bool p=true;
void loop() {
  // put your main code here, to run repeatedly:
  if(WiFi.status()!= WL_CONNECTED)
  {
    Serial.println("unconnect");
    delay(500);
  }
  else 
  {
    if(p)
    {
      p=false;
      Serial.println(WiFi.localIP());
      Serial.println("smartconfig success");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      Serial.print("read io4 :");
      Serial.println(digitalRead(4));
    }
  }
  if(wifi_config)
  {
    detachInterrupt(4);
    smartConfig();
    attachInterrupt(digitalPinToInterrupt(4), interrput_io4, FALLING);
  }
  doHTTPget();
  //Serial.print(".");
  delay(10);
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
  delay(500);
  int count=0;
  //持续3秒低电平
  if(!digitalRead(4)){
    while(count<=5)
    {
      delay(500);
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
  p=true;
  //digitalWrite(4,HIGH);
}
void doHTTPget()
{
  if(millis() - getinterval > httpTime){
    if(WiFi.status() != WL_CONNECTED) return;
    HTTPClient http;
    http.begin(SERVER_ADDR);
    int httpResponseCode = http.POST(SERVER_ADDR);
    Serial.print("HTTP response code :");
    Serial.println(httpResponseCode);
    if(httpResponseCode > 0){
        //Serial.print("HTTP response code :");
        //Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
      }
    http.end();//free resources
    getinterval=millis();
  }
}
