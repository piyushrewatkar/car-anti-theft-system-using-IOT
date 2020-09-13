#include <SoftwareSerial.h>

SoftwareSerial gsm(2, 3);
SoftwareSerial gps(10,11);

#define x A1
#define y A2
#define z A3

int xsample=0;
int ysample=0;
int zsample=0;

#define samples 10

#define minVal -50
#define MaxVal 50

int i=0,k=0;
int  gps_status=0;
double latitude=0; 
double logitude=0;                       
String Speed="";
String gpsString="";
char *test="$GPRMC";


char a[100];    
const int motor = 5;
const int key = 7;
int keystate = 0;
int flag = 0;

int initial=0;
int eflag=0;
int sendCount = 0;

char incomingByte; 
String incomingData;
bool atCommand = true;


int index = 0;
String number = "";
String message = "";
 

void setup() 
{
  pinMode(motor, OUTPUT);
  pinMode(key,INPUT);
  gsm.begin(9600);
  Serial.begin(9600);
  delay(100);

  
  Serial.println("Initializing....");
  initModule("AT","OK",1000);
  initModule("ATE1","OK",1000);
  initModule("AT+CPIN?","READY",1000);  
  initModule("AT+CMGF=1","OK",1000);     
  initModule("AT+CNMI=2,2,0,0,0","OK",1000);  
  Serial.println("Initialized Successfully");

  for(int i=0;i<samples;i++)
  {
    xsample+=analogRead(x);
    ysample+=analogRead(y);
    zsample+=analogRead(z);
  }

  xsample/=samples;
  ysample/=samples;
  zsample/=samples;

  Serial.println(xsample);
  Serial.println(ysample);
  Serial.println(zsample);
  delay(1000);
  gps.begin(9600);
//  get_gps();
  show_coordinate();

  Serial.println("System Ready..");
}

void loop() 
{
    int value1=analogRead(x);
    int value2=analogRead(y);
    int value3=analogRead(z);

    int xValue=xsample-value1;
    int yValue=ysample-value2;
    int zValue=zsample-value3;
    
    Serial.print("x=");
    Serial.print(xValue);
    Serial.print("\t\t");
    Serial.print("y=");
    Serial.print(yValue);
    Serial.print("\t\t");
    Serial.print("z=");
    Serial.print(zValue);
    Serial.print("\t\t");
    Serial.print("\n\n");
    delay(100);
    
    if(xValue < minVal || xValue > MaxVal  || yValue < minVal || yValue > MaxVal  || zValue < minVal || zValue > MaxVal)
    {
      
      get_gps();
      show_coordinate();
      if (sendCount == 0){
      Serial.println("Sending SMS");
      
        Send();
      sendCount++;
      Serial.println("SMS Sent");
      }
    }       


  keystate=digitalRead(key);
  if(keystate==HIGH){
    if(eflag==0){
    digitalWrite(motor,HIGH);
    flag=1;
    if(initial==0){
    
    SendTheft();
    }
    }
    
  }
  if(flag==1){
   if(keystate!=HIGH){
    digitalWrite(motor,LOW);
    flag=0;
  }
  
  }
  if(gsm.available()){
      delay(100);
      
      // Serial buffer
      while(gsm.available()){
        incomingByte = gsm.read();
        incomingData += incomingByte; 
       }
        
        delay(10); 
        if(atCommand == false){
          receivedMessage(incomingData);
        }else{
          atCommand = false;
        }
        
        //delete messages to save memory
        if (incomingData.indexOf("OK") == -1){
          gsm.println("AT+CMGDA=\"DEL ALL\"");
          delay(1000);
          atCommand = true;
        }
        
        incomingData = "";
  }
  
    
}


void initModule(String cmd, char *res, int t)
{
  while(1)
  {
    Serial.println(cmd);
    gsm.println(cmd);
    delay(100);
    while(gsm.available()>0)
    {
       if(gsm.find(res))
       {
        Serial.println(res);
        delay(t);
        return;
       }

       else
       {
        Serial.println("Error");
       }
    }
    delay(t);
  }
}


void gpsEvent()
{
  gpsString="";
  while(1)
  {
   while (gps.available()>0)            //Serial incoming data from GPS
   {
    char inChar = (char)gps.read();
     gpsString+= inChar;                    //store incoming data from GPS to temparary string str[]
     i++;
    // Serial.print(inChar);
     if (i < 7)                      
     {
      if(gpsString[i-1] != test[i-1])         //check for right string
      {
        i=0;
        gpsString="";
      }
     }
    if(inChar=='\r')
    {
     if(i>60)
     {
       gps_status=1;
       break;
     }
     else
     {
       i=0;
     }
    }
  }
   if(gps_status)
    break;
  }
}

void get_gps()
{
 
   gps_status=0;
   int x=0;
   while(gps_status==0)
   {
    gpsEvent();
    int str_lenth=i;
    coordinate2dec();
    i=0;x=0;
    str_lenth=0;
   }
}

void show_coordinate()
{
    Serial.print("Latitude:");
    Serial.println(latitude);
    Serial.print("Longitude:");
    Serial.println(logitude);
    Serial.print("Speed(in knots)=");
    Serial.println(Speed);
}

void coordinate2dec()
{
  String lat_degree="";
    for(i=20;i<=21;i++)         
      lat_degree+=gpsString[i];
      
  String lat_minut="";
     for(i=22;i<=28;i++)         
      lat_minut+=gpsString[i];

  String log_degree="";
    for(i=32;i<=34;i++)
      log_degree+=gpsString[i];

  String log_minut="";
    for(i=35;i<=41;i++)
      log_minut+=gpsString[i];
    
    Speed="";
    for(i=45;i<48;i++)          //extract longitude from string
      Speed+=gpsString[i];
      
     double minut= lat_minut.toDouble();
     minut=minut/60;
     double degree=lat_degree.toDouble();
     latitude=degree+minut;
     
     minut= log_minut.toDouble();
     minut=minut/60;
     degree=log_degree.toDouble();
     logitude=degree+minut;
}

void Send()
{ 
   gsm.println("AT");
   delay(500);
   serialPrint();
   gsm.println("AT+CMGF=1");
   delay(500);
   serialPrint();
   gsm.print("AT+CMGS=");
   gsm.print('"');
   gsm.print("9518959050");    //mobile no. for SMS alert
   gsm.println('"');
  delay(500);
   serialPrint();
  gsm.println("Accident detected at:");
   gsm.print("Latitude:");
   gsm.println(latitude);
   delay(500);
   serialPrint();
   gsm.print("Longitude:");
   gsm.println(logitude);
   delay(500);
   serialPrint();
   gsm.print(" Speed:");
   gsm.print(Speed);
   gsm.println("Knots");
   delay(500);
   serialPrint();
   gsm.print("http://maps.google.com/maps?&z=15&mrt=yp&t=k&q=");
   gsm.print(latitude,8);
   gsm.print("+");              //28.612953, 77.231545   //28.612953,77.2293563
   gsm.print(logitude,8);
   gsm.write(26);
   delay(2000);
   serialPrint();
}
void SendTheft()
{ initial=1;
   gsm.println("AT");
   delay(500);
   serialPrint();
   gsm.println("AT+CMGF=1");
   delay(500);
   serialPrint();
   gsm.print("AT+CMGS=");
   gsm.print('"');
   gsm.print("9518959050");    //mobile no. for SMS alert
   gsm.println('"');
  delay(500);
   serialPrint();
   Serial.println("Theft Detected.");
   gsm.println("Theft detected at:"); 
   gsm.print("Latitude:");
   gsm.println(latitude);
   delay(500);
   serialPrint();
   gsm.print("Longitude:");
   gsm.println(logitude);
   delay(500);
   serialPrint();
   gsm.print(" Speed:");
   gsm.print(Speed);
   gsm.println("Knots");
   delay(500);
   serialPrint();
   gsm.print("http://maps.google.com/maps?&z=15&mrt=yp&t=k&q=");
   gsm.print(latitude,8);
   gsm.print("+");              //28.612953, 77.231545   //28.612953,77.2293563
   gsm.print(logitude,8);
   gsm.write(26);
   delay(2000);
   serialPrint();
}

void serialPrint()
{
  while(gsm.available()>0)
  {
    Serial.print(gsm.read());
  }
}
void receivedMessage(String inputString){
  
  //Get The number of the sender
  index = inputString.indexOf('"')+1;
  inputString = inputString.substring(index);
  index = inputString.indexOf('"');
  number = inputString.substring(0,index);
  Serial.println("Number: " + number);

  //Get The Message of the sender
  index = inputString.indexOf("\n")+1;
  message = inputString.substring(index);
  message.trim();
  Serial.println("Message: " + message);
        
  message.toUpperCase(); // uppercase the message received

  //turn LED ON or OFF
  if (message.indexOf("ON") > -1){
      digitalWrite(motor, HIGH);
      Serial.println("Command: LED Turn On.");
   }
          
  if (message.indexOf("OFF") > -1){
        digitalWrite(motor, LOW);
        Serial.println("Command: LED Turn Off.");
   }
          
   delay(50);
  }
