#include<ESP8266WiFi.h>
#include<WiFiClient.h>

//需要设置的服务器地址以及端口
#define TCP_SERVER_ADDR "47.96.146.251"
//#define TCP_SERVER_ADDR "192.168.1.108"
#define TCP_SERVER_PORT "8306"

#define debug false //上传自增数据
#define upDataTime 2*1000 //上传自增数据周期

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

//Wi-Fi中断配置
void ICACHE_RAM_ATTR interrput_io4();
bool wifi_config=false;

//函数声明
void smartConfig();
void interrput_io4();

//TCP初始化连接
void doTCPClientTick();
void startTCPClient();
void sendtoTCPServer(String p);
void sendUINT8toTCPServer(uint8_t *p);
void sendJsontoTCPServer(char *p);
bool WL_info=true;//用于输出连接路由信息


/*  
 *   声明
 *  报文匹配  
 *  循环队列  
*/
char uploadjson[128];
#define MessageLen 15  //规定报文长度。

#define BuffLen 1024
struct Queue
{
  uint8_t Buff[BuffLen];
  int front;
  int rear;
}gatawayBuff;

struct json
{
  int ID;
  float data[3];
  } uploadData;

//循环队列操作声明
void InitQueue(struct Queue *p);
short IsFull(struct Queue *p);
short IsEmpty(struct Queue *p);
short EnterQueue(struct Queue *p,uint8_t key);
short DeQueue(struct Queue *p,uint8_t *value);

//报文处理
void sortAmessage(struct Queue *p);

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
    if(WL_info)
    {
      WL_info=false;
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
  WL_info=true;
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
      uint8_t u8=Serial.read();
      if(debug)
      {
        Serial.print("read a data from Serial : ");
        Serial.println(u8);
      }
      if(!EnterQueue(&gatawayBuff,u8))
      {
        Serial.println("Full!!!!!");
        predataTick=predataTick-100;
      }

      TcpClient_preTick = millis();
      preHeartTick = millis();
      predataTick = millis();
    }
    
    //发送心跳
    if(millis() - preHeartTick >= upheartTime)
    {
      preHeartTick = millis();
      
      String upstr = "";
      upstr = "cmd=0&msg=ping\r\n";
      intNumber++;
      sendtoTCPServer(upstr);
      upstr = "";
    }
    
    //上传自增数据
    
    if(millis()-predataTick>=upDataTime && debug==true)
    {
      predataTick = millis();
      
      String upstr = "";
      //upstr = "&msg = "+intNumber+"\r\n";
      intNumber++;
      sendtoTCPServer(upstr);
      upstr = "";
      
    }

    //服务器通信
    if((!IsEmpty(&gatawayBuff))&& (millis()-predataTick>100))
    {
      sortAmessage(&gatawayBuff);
      
      predataTick=millis();
    }
    /*
    if((TcpClient_BuffIndex >= 1)&& (millis() - TcpClient_preTick>=100))
    {//data ready
      TcpClient_preTick = millis();
      
      TCPclient.flush();
      Serial.println("receive from zigbee in Buff:");
      //Serial.println("%s",TcpClient_Buff);
      sendUINT8toTCPServer(TcpClient_Buff);//将收到的报文送给服务器。
      //TcpClient_Buff="";
      TcpClient_BuffIndex = 0;
    }
    */
  }
}
void sendUINT8toTCPServer(uint8_t *p)
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

void sendJsontoTCPServer(char *p){
  
  if (!TCPclient.connected()) 
  {
    Serial.println("Client is not readly");
    return;
  }
  TCPclient.print(p);
  Serial.println("[Send to TCPServer]:String");
  Serial.println(p);
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


/*
 * 以下是循环队列的操作
 * 报文识别函数的源代码
 */

//初始化队列
void InitQueue(struct Queue *p)
{
  p->front=0;
  p->rear=0;
}

short IsFull(struct Queue *p)
{
  if(p->front==(p->rear+1)%BuffLen)
  {
    return 1;
  }
  else return 0;
}

short IsEmpty(struct Queue *p)
{
  if (p->front==p->rear) return 1;
  else return 0;  
}
short EnterQueue(struct Queue *p,uint8_t key)
{
  if(IsFull(p)) return 0;
  p->Buff[p->rear]=key;
  p->rear=(p->rear+1)%BuffLen;
  return 1;
}
short DeQueue(struct Queue *p,uint8_t *value)
{
  if(IsEmpty(p)) return 0;
  *value=p->Buff[p->front];
  p->front=(p->front+1)%BuffLen;
  return 1;
}

/*
 * 报文整理
 */
void sortAmessage(struct Queue *p)
{
  uint8_t value, DataValue[12];
  while(!IsEmpty(p)) 
  {
    DeQueue(p,&value);
    if(debug)
    {
      Serial.print("DeQueue a data : ");
      Serial.println(value);
    }
    if (value==0xFE)//fing 0xFE of message
    {
          if(debug)Serial.println("yes");

      if(p->Buff[(p->front+MessageLen) % BuffLen ]==0xFF)//精确定位
      {
        Serial.println("find the end of message!!"); 
        DeQueue(p,&value);
        uploadData.ID=value;
        for(int i = 0;i < MessageLen - 3; i++) 
        {
          DeQueue(p,&value);
          DataValue[i]=value;
        }
        memcpy(&uploadData.data, DataValue, MessageLen - 3);

        sprintf(uploadjson,"{ \"ID\":%u,\"Humiture\":%f,\"Light\":%f,\"Temperature\":%f}"
        ,uploadData.ID,uploadData.data[0],uploadData.data[1],uploadData.data[2]);
        Serial.print("uploadjson : ");
        Serial.println(uploadjson);
        sendJsontoTCPServer(uploadjson);
      }
      
      else//drop a message,fing 0xFF
      {
        Serial.println("can't find the end, throw away a message!");
        unsigned int RearIndex=(p->front+MessageLen) % BuffLen;
          
        for(int i=0;i<3;i++)//往回找尾
        {
          uint8_t SearchData=p->Buff[(p->front+MessageLen-i) % BuffLen];
          
          if(SearchData==0xFF)
          {
            p->front=(RearIndex-i) % BuffLen;
            
            break;
          }
          else if(SearchData==0XFE)
          {
            p->front=(RearIndex-i-1) % BuffLen;
            break;
          }
        }
      }
    }
  }
}
