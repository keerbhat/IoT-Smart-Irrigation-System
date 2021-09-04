//ESP8266_SERIAL_INIT
#include <SoftwareSerial.h>       //Software Serial library
SoftwareSerial espSerial(2, 3);  //pin2=Rx , pin3=Tx
#define DEBUG false
String mySSID = "SSID";       // WiFi SSID
String myPWD = "PASSWORD"; // WiFi Password
String myAPI = "XXXXXXXXXXXXXXXX";   // API Key
String myHOST = "api.thingspeak.com";
String myPORT = "80";
String myFIELD1 = "field1"; 
String myFIELD2="field2";


//DHT11_INIT
#include "DHT.h"
#define DHTPIN 7     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

//SERVO_INIT
#include <Servo.h>
Servo myservo;//servo object
int servopin=10;

//soil moisture level sensor
int soil_mpin=5;//arduino pin sensors digital output is connected to

//water level sensor
int water_lpin=A0;//arduino pin sensors digital output is connected to

//rain sensor
int rain_spin=8;

//values
float temprature;
float humidity;
int soil_m;
float water_l; 
int rain;
void setup() 
{
  espSerial.begin(115200);
  espData("AT+RST", 1000, DEBUG);                      //Reset the ESP8266 module
  espData("AT+CWMODE=1", 1000, DEBUG);                 //Set the ESP mode as station mode
  espData("AT+CWJAP=\""+ mySSID +"\",\""+ myPWD +"\"", 1000, DEBUG);   //Connect to WiFi network
  dht.begin();
  myservo.attach(servopin);
  myservo.write(0);//setting valve to closed
  pinMode(soil_mpin,INPUT);//SOIL MOISTURE LEVEL
  pinMode(12,OUTPUT);
  pinMode(rain_spin,INPUT);
  
}


void loop() 
{ 
  temprature=temp();
  humidity=hum();
  data_send(temprature,myFIELD1);
  data_send(humidity,myFIELD2);
  
 //checking if irrigation is needed 
  soil_m=digitalRead(soil_mpin);  
  rain=digitalRead(rain_spin);
  if (soil_m)
    {
      if (rain)
        {myservo.write(180);//open water valve
        }
      else
        {myservo.write(0);//close valves as rain is there
        } 
    }
  else
    {myservo.write(0);//closed water valve
    }

//checking water level
  water_l=analogRead(water_lpin); 
  if (water_l<100)
  {digitalWrite(12,HIGH);//indicates to owner water is less in tank
    }
  else if(water_l>100)
  {digitalWrite(12,LOW);
   }
   
  delay(2000);
}


//functions

//temprature
float temp()
{
 float t = dht.readTemperature();
 return t;
  }

  
//humidity
 float hum()
 {
  float h = dht.readHumidity();
  return h;
  }
  

//SEND DATA and wait for recv USING AT COMMANDS
 String espData(String command, const int timeout, boolean debug)
{
  
  String response = "";
  espSerial.println(command);
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (espSerial.available())
    {
      char c = espSerial.read();
      response += c;
    }
  }
  if (debug)
  {
    //Serial.print(response);
  }
  return response;
}  

//Send Sensor data

void data_send(float val,String field)
{
    String sendData = "GET /update?api_key="+ myAPI +"&"+ field +"="+String(val);
    espData("AT+CIPMUX=1", 1000, DEBUG);       //Allow multiple connections
    espData("AT+CIPSTART=0,\"TCP\",\""+ myHOST +"\","+ myPORT, 1000, DEBUG);
    espData("AT+CIPSEND=0," +String(sendData.length()+4),1000,DEBUG);  
    espSerial.find(">"); 
    espSerial.println(sendData);
    espData("AT+CIPCLOSE=0",1000,DEBUG);
   
  }








  
