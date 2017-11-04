#include <SoftwareSerial.h>       //虚拟软件串口
SoftwareSerial gsmSerial(9,11);  //gsm串口
SoftwareSerial gpsSerial(0,1);   //gps串口
int Direction;
int tempFlag = 0,irFlag = 0,shakeFlag = 0;
char gpsData[100];
char gsmData[100];
char Degree[3];
String longitude, latitude;
String phoneNum = "AT + CMGS= \"+8618260635830\"";
String text = "";

int string_to_int(){
  int a,Num = 0;
  for(int i = 0; i <= 2;){
    switch(Degree[i]){
      case '0':
        a = 0;
        break;
      case '1':
        a = 1;
        break;
      case '2':
        a = 2;
        break;
      case '3':
        a = 3;
        break;
      case '4':
        a = 4;        break;
      case '5':
        a = 5;
        break;
      case '6':
        a = 6;
        break;
      case '7':
        a = 7;
        break;
      case '8':
        a = 8;
        break;
      case '9':
        a = 9;
        break;
    }
   switch(i){
     case 0:
       Num = Num + a * 100;
       break;
     case 1:
       Num = Num + a * 10;
       break;
     case 2:
       Num = Num + a * 1;
       break;
   }
  i = i + 1; 
  }  return Num;
}

/*
 *  编译器自带,在上电、复位后运行一次
 *  初始化GSM串口，准备接受短信。
 */
void setup() {
  // put your setup code here, to run once:
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(8, INPUT);
  Serial.begin(9600);
  gsmSerial.begin(115200);
  gpsSerial.begin(9600);
  gsmSerial.println("AT+CMGF=1");
  delay(200);
  gsmSerial.println("AT+CNMI=2,2,0,0,0");
  delay(200);
}

/*
 *  读取短信
 */
int judgment_sms(){
        switch(gsmData[0]){
          case 'p':
          case 'P':          //如果读到指定字符
            //memset(gsmData, 0, strlen(gsmData));
            gsmData[0] = '\0';
            return 1;
            break;

          case 'm':
          case 'M':
            switch(gsmData[1]){
              case '+':
                Direction = 0;
                break;
              case '-':
                Direction = 1;
                break;
              default:
                break;
            }
            for(int i = 0; i <= 2;){
              char tmp;
              tmp = gsmData[i+2];
              Degree[i] = tmp;
              i = i + 1;
            }
            return 2;
            break;
          default:
            return 0;
            break;
       }  
}

/*
 *  获得第num个逗号[前]的数据
 *  含数据前的","
 */
String Comma(int num){    
  unsigned char i,j = 0;
  int len = strlen(gpsData);
  String gpsInfo = "";
  for(i = 0; i <= len; i = i + 1){
    if(gpsData[i] == ','){
      j = j + 1;
      if(j == num){
        return gpsInfo;
      }
    }
    if(j == (num - 1)){
      gpsInfo += gpsData[i];
    }
  } 
}

/*
 *  判断读到的GPS数据
 *  并将经度，纬度写入全局变量longitude, latitude中
 */
int judgment_gpsData(){
  longitude = "";
  latitude = "";
  if(Comma(1) == "GPGLL"){
    if(Comma(3) == ",N"){
      longitude = "N";    }
    if(Comma(3) == ",S"){
      longitude = "S";
    }
    if(Comma(3) != ",N" && Comma(3) != ",S"){
      text = "No GPS position";
      return 0;
    }
    longitude += Comma(2);
      
    if(Comma(5) == ",E"){
      latitude = "E";
    }
    else{
      latitude = "W";
    }
    latitude += Comma(4);
    text = longitude + ", " + latitude;
    return 1;   //成功把数据存入longitude, latitude中
  }
  else{
    delay(5);
    return 3; 
  }
}

void step_motor(){
  for(int i = 4; i <= 7;){
    pinMode(i, OUTPUT);
    i = i + 1;   }

  int a = (string_to_int() / 5.625) * 8;
  Serial.print("5. a ");
  Serial.println(a);
    if(Direction == 0){
      while(a--){
        for(int i = 4; i <= 7;){
        digitalWrite(i, HIGH);
        delay(10);
        digitalWrite(i, LOW);
        i = i + 1;
        } 
      }
    }
    if(Direction == 1){
      while(a--){
        for(int i = 7; i >= 4;){
        digitalWrite(i, HIGH);
        delay(10);
        digitalWrite(i, LOW);
        i = i - 1;
        } 
      }
    }
}

/*
 *  编译器自带，单片机运行时一直循环
 *  类似  while(1){}
 */
void loop() {
  // put your main code here, to run repeatedly:
  int i = 0;
  gsmSerial.listen();
  while(gsmSerial.available()){
    char inSms = gsmSerial.read();
    if(inSms == '\n'){
      gsmData[i] = '\0';
      break;
    }
    else{
      gsmData[i] = inSms;
      i = i + 1;
    }
    delay(10);
  }

  switch(judgment_sms()){
    case 1:              //读到的GPS数据，并将经度，纬度写入全局变量longitude, latitude中
      location();
      send_sms(text);         //发送TEXT中的内容
      delay(10);
      rev_sms_setup();        //设置gsm处于接收短信的状态  
    break; 
    case 2:
      step_motor();
      rev_sms_setup();    break;
    default:
    break;
  }
  
  //火警
  delay(10);
  if(digitalRead(2) == HIGH){
    tempFlag = 1;
  }
  if(digitalRead(2) == LOW && tempFlag == 1){
    tempFlag = 0;
    text = "fire";
    send_sms(text);
    delay(10);
    rev_sms_setup();  
  }
  
  //红外
  delay(10);
  if(digitalRead(8) == HIGH){
    irFlag = 1;
  }
  if(digitalRead(8) == LOW && irFlag == 1){
    irFlag = 0;
    text = "broke";
    send_sms(text);
    delay(10);
    rev_sms_setup();  
  }
  //震动
  delay(10);
  if(digitalRead(3) == HIGH){
    shakeFlag = 1;
  }
  if(digitalRead(3) == LOW && shakeFlag == 1){
    shakeFlag = 0;
    location();
    send_sms(text);         //发送TEXT中的内容
    delay(10);
    rev_sms_setup();  
  }
}

/*
 *  若没有成功读到gps定位信息
 *  就一直读取gps串口数据
 */
void location(){
  int i = 0;
  do{
    get_gpsData();
    delay(10);
    i = i + 1;
    if(i > 1000){
      text = "time out";
      break; 
    }
    delay(5);  }while(judgment_gpsData() == 3);
}

void rev_sms_setup(){
  gsmSerial.println("AT+CMGF=1");
  delay(200);
  gsmSerial.println("AT+CNMI=2,2,0,0,0");
  delay(200);  
}

/*
 *  读取gps串口数据
 *  存入全局变量gpsData[100]中
 */
void get_gpsData(){
  char inChar;
  int i = 0;
  gpsSerial.listen();
  while(gpsSerial.available()){
    gpsSerial.listen();
    inChar = gpsSerial.read();
    if(inChar != '$'){  
      /*
       *  读$后的数据
       *  单片机的速度远快于串口速度
       *  除了第一个数据外，不会造成数据的丢失。
       */
      gpsData[i] = inChar;
      i = i + 1; 
      if(inChar == '\n'){      //当读到换行时一段数据读完
        gpsData[i] = '\0';
        break;
      }
    }
  }
}

/*
 *  发送短信
 */
void send_sms(String txt){
  //gsmSerial.listen();
  gsmSerial.println("AT+CMGF=1");  //println在串口写入数据后，末尾加回车
  delay(100);
  gsmSerial.println(phoneNum);
  delay(100);
  gsmSerial.println(txt);
  delay(100);
  gsmSerial.println((char)26); //Crtl+Z或ESC。结束输入内容，并发送短信
  delay(100);
  gsmSerial.println();
  delay(3000);
}
