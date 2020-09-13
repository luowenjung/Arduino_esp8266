unsigned long predataTick=0; //数据接收时间

char uploadjson[128];
#define MessageLen 15  //规定报文长度。
/* 循环队列 */
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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  InitQueue(&gatawayBuff);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()>0)
  {
    uint8_t u8=Serial.read();
    Serial.print("read a data from Serial : ");
    Serial.println(u8);
    if(!EnterQueue(&gatawayBuff,u8))
    {
      Serial.println("Full!!!!!");
      predataTick=predataTick-100;
    }
    predataTick=millis();
  }
  if((!IsEmpty(&gatawayBuff))&& (millis()-predataTick>100))
  {
    sortAmessage(&gatawayBuff);
    predataTick=millis();
  }
  delay(5);
}

/*
 * 以下是循环队列的操作
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
    
    Serial.print("DeQueue a data : ");
    Serial.println(value);
    if (value==0xFE)//fing 0xFE of message
    {
      Serial.println("yes");

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
