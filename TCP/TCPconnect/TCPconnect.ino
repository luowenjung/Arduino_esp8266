#include<ESP8266WiFi.h>
#include<WiFiClient.h>
//需要设置的服务器地址以及端口
#define TCP_SERVER_ADDR "47.96.146.251"
//#define TCP_SERVER_ADDR "192.168.1.108"
#define TCP_SERVER_PORT "8306"
//用户私钥，可在控制台获取,修改为自己的UID
String UID = "esp8266";

#define debug false

#define upDataTime 2*1000
//设置心跳
#define upheartTime 30*1000
int intNumber = 0;

//TCP客户端初始化
WiFiClient TCPclient;
uint8_t TcpClient_Buff[512];
unsigned int TcpClient_BuffIndex = 0;
unsigned long TcpClient_preTick = 0;
unsigned long preHeartTick = 0;//心跳
unsigned long predataTick = 0;//数据发送时间

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
void seneUINT8toTCPServer(uint8_t *p);

bool p=true;
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
    delay(500);
  }
  else 
  {
    if(p)
    {
      p=false;
      Serial.println(WiFi.localIP());
      Serial.println("smartconfig success");
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
  doTCPClientTick();
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
  p=true;
  //digitalWrite(4,HIGH);
}
void startTCPClient(){
  if(TCPclient.connect(TCP_SERVER_ADDR,atoi(TCP_SERVER_PORT)))
    {
      Serial.print("\nConnected to server:");
      Serial.print(TCP_SERVER_ADDR);
    }
    else
    {
      Serial.print("\nFailed connected to server:");
      Serial.print(TCP_SERVER_ADDR);
      Serial.print("\nPORT :");
      Serial.print(TCP_SERVER_PORT);
      TCPclient.stop();
    }
  }
void doTCPClientTick()
{
  if(WiFi.status()!=WL_CONNECTED) return;
  if(!TCPclient.connected())
  {
    TCPclient.stop();
    //这里需要添加一个重新连接的时间周期。
    startTCPClient();
  }
  else
  {
    
    if(Serial.available()>0)//接收
    {
      uint8_t c = Serial.read();
      TcpClient_Buff[TcpClient_BuffIndex] = c;
      TcpClient_BuffIndex ++;
      //Serial.println(TcpClient_Buff);
      TcpClient_preTick = millis();
      if(TcpClient_BuffIndex>=MAX_PACKETSIZE-1)
      {
        TcpClient_BuffIndex=MAX_PACKETSIZE-2;
        TcpClient_preTick=TcpClient_preTick-100;
      }
      preHeartTick = millis();
      predataTick = millis();
    }
    
    if(millis() - preHeartTick >= upheartTime)
    {//发送心跳
      preHeartTick = millis();
      
      String upstr = "";
      upstr = "cmd=0&msg=ping\r\n";
      intNumber++;
      sendtoTCPServer(upstr);
      upstr = "";
    }
    //上传数据
    if(millis()-predataTick>=upDataTime && debug==true)
    {
      predataTick = millis();
      
      String upstr = "";
      upstr = "cmd = 2& uid = "+UID+" &msg = "+intNumber+"\r\n";
      intNumber++;
      sendtoTCPServer(upstr);
      upstr = "";
      
    }
  if((TcpClient_BuffIndex >= 1)&& (millis() - TcpClient_preTick>=100))
  {//data ready
    TcpClient_preTick = millis();
    
    TCPclient.flush();
    Serial.println("receive from zigbee in Buff:");
    //Serial.println("%s",TcpClient_Buff);
    seneUINT8toTCPServer(TcpClient_Buff);//将收到的报文送给服务器。
    //TcpClient_Buff="";
    TcpClient_BuffIndex = 0;
  }
  }
}
void seneUINT8toTCPServer(uint8_t *p)
{
  if (!TCPclient.connected()) 
  {
    Serial.println("Client is not readly");
    return;
  }
  for(int i = 0;i < TcpClient_BuffIndex - 3 ; i++)
  {
    TCPclient.print(p[i]);
    TCPclient.print(',');
    Serial.print(p[i]);
    Serial.print(' ');
    p[i]=0;
  }
  Serial.println();
}

void sendtoTCPServer(String p){
  
  if (!TCPclient.connected()) 
  {
    Serial.println("Client is not readly");
    return;
  }
  TCPclient.print(p);
  Serial.println("[Send to TCPServer]:String");
  Serial.println(p);
}
