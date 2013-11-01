/* Requires Arduino IDE and MIRF library
 * MIRF lib: https://github.com/aaronds/arduino-nrf24l01/tree/master/Mirf
 * NOTE: It may be necessary to modify MirfHardwareSpiDriver.cpp with; 
 * SPI.setClockDivider(SPI_CLOCK_DIV2);
 * to increase stability on Freescale K series uC.
 * NOTE: added flushRX() function declare to HardwareSerial.h & function to HardwareSerial.cpp
 * flushRX() is the old <1.0 flush() function to clear the serial buffer.
 */
 
#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>



//GSM driver pins
byte gsmDriverPin[3] = {3,4,5};

//Xively API Key
char xApiKey[49] = "CAhdALe5DFe3xjtcUTdFk0HqWAOwB8xCM3tiLsZqaBVen0zS";
//byte data[Mirf.payload]; // data buffer - unused currently
//float data; // recieved data

void setup(){
  
  //setup GSM driver pins
  for (int i = 0 ; i < 3; i++){
    pinMode(gsmDriverPin[i],OUTPUT);
  }
  
  digitalWrite(5,HIGH);//Output GSM Timing 
  delay(1500);
  digitalWrite(5,LOW);
  delay(5000);  
  digitalWrite(3,LOW);//Enable the GSM mode
  digitalWrite(4,HIGH);//Disable the GPS mode
  delay(2000);
  
  Serial.begin(9600);
  delay(5000);//call ready & wait for comms 
  //delay(5000);
  
  enableGPRS();
  enableGPS();
  
  //Teensy pin config
  //Mirf.cePin = 9;
  //Mirf.csnPin = 10;   
  
  // Set the SPI Driver.
  Mirf.spi = &MirfHardwareSpi;
  
  // Setup pins / SPI
  Mirf.init();
  
  // Configure reciving address.   
  Mirf.setRADDR((byte *)"tbmn1");
  
  // payload on client and server must be the same. 
  Mirf.payload = sizeof(float);
  
  // Write channel and payload config then power up reciver.
  Mirf.config();
  
  // Get IP address and post it
  getIP();
}

////////// main loop //////////
void loop(){
  
  postDataVolts(pollSensor());

  postDataGPS(getLatitude(), lat_dir(), getLongitude(), lon_dir());
  
  delay(30000);
 
}

////////// function to get the current IP and update xevily //////////
void getIP() {
  //char* ipAddr[];
  char serData;
  String stringIpAddr = "";
  
  digitalWrite(3,LOW);  //Enable GSM mode
  digitalWrite(4,HIGH);  //Disable GPS mode
  
  Serial.println("AT+SAPBR=1,1"); //bring up connection
  delay(3000);
  
  Serial.println("AT+HTTPINIT"); //Init HTTP engine
  delay(1500);
 
  Serial.flushRX(); 
  Serial.println("AT+SAPBR=2,1"); //get IP address
  delay(1000);
  
  while (Serial.available() > 0) {
    serData = Serial.read();
    stringIpAddr += serData;
  }
  
  if (stringIpAddr.indexOf("\"") > 0) {
    //Serial.println(stringIpAddr);
    stringIpAddr = stringIpAddr.substring(stringIpAddr.indexOf("\"") + 1, stringIpAddr.lastIndexOf("\"") - 1); 
  }
  
  Serial.print("AT+HTTPPARA=\"URL\",\"ec2-54-242-171-87.compute-1.amazonaws.com/xively/xivelyPut.php?X-ApiKey=CAhdALe5DFe3xjtcUTdFk0HqWAOwB8xCM3tiLsZqaBVen0zS&chan=IPAddr&DATA=");

  Serial.print(stringIpAddr);
  Serial.println("\"");
  delay(1000);
  
  Serial.println("AT+HTTPREAD");
  delay(2000);
  
  Serial.println("AT+HTTPACTION=0"); //do HTTP get
  delay(6000);  

  Serial.println("AT+HTTPTERM");
  delay(1000);

}

float pollSensor(){
 
  float data;
  
  /*
   * If a packet has been recived.
   * isSending also restores listening mode when it 
   * transitions from true to false.
   */
   
  if(!Mirf.isSending() && Mirf.dataReady()){
    
    // load the packet into the buffer.        
    Mirf.getData((byte *)&data);
   
    // Set the send address.
    Mirf.setTADDR((byte *)"volt1");
    
    // Send the data back to the client.     
    Mirf.send((byte *)&data);
    
    /*
     * Wait untill sending has finished
     * NB: isSending returns the chip to receving after returning true.
     */
    
    return data;
  }
  
}

////////// POST Battery Voltage Data to Xively /////////
void postDataVolts(float senseData) {
//  char serDat[25];
  
  //enableGPRS();
  digitalWrite(3,LOW);  //Enable GSM mode
  digitalWrite(4,HIGH);  //Disable GPS mode
   
  Serial.println("AT+SAPBR=1,1"); //bring up connection
  delay(3000);
  
  Serial.println("AT+HTTPINIT"); //Init HTTP engine
  delay(1500);

  //set URL
  Serial.print("AT+HTTPPARA=\"URL\",\"ec2-54-242-171-87.compute-1.amazonaws.com/xively/xivelyPut.php?X-ApiKey=CAhdALe5DFe3xjtcUTdFk0HqWAOwB8xCM3tiLsZqaBVen0zS&chan=volts&DATA=");
  Serial.print(senseData);
  Serial.println("\"");
  delay(1000);

  Serial.println("AT+HTTPACTION=0"); //do HTTP get
  delay(6000);

  Serial.println("AT+HTTPREAD");
  delay(2000);

  Serial.println("AT+HTTPTERM");
  delay(1000);
 
}

////////// POST GPS Data to Xively //////////
void postDataGPS(double lat, char latDir, double lon, char lonDir) {
//  char serDat[25];
  
  //enableGPRS();  
  
  digitalWrite(3,LOW);  //Enable GSM mode
  digitalWrite(4,HIGH);  //Disable GPS mode
  
  Serial.println("AT+SAPBR=1,1"); //bring up connection
  delay(3000);
  
  Serial.println("AT+HTTPINIT"); //Init HTTP engine
  delay(1500);

  //set URL
  Serial.print("AT+HTTPPARA=\"URL\",\"ec2-54-242-171-87.compute-1.amazonaws.com/xively/xivelyPut.php?X-ApiKey=CAhdALe5DFe3xjtcUTdFk0HqWAOwB8xCM3tiLsZqaBVen0zS&chan=location&DATA=");
  Serial.print(lat);
  Serial.print(latDir);
  Serial.print(":");
  Serial.print(lon);
  Serial.print(lonDir);
  Serial.println("\"");
  delay(1000);

  Serial.println("AT+HTTPACTION=0"); //do HTTP get
  delay(6000);

  Serial.println("AT+HTTPREAD");
  delay(2000);

  Serial.println("AT+HTTPTERM");
  delay(1000);
 
}

////////// Enable GPS //////////
void enableGPS() {

  //digitalWrite(3,LOW);  //Enable GSM mode
  //digitalWrite(4,HIGH);  //Disable GPS mode
  delay(2000);
  //Serial.begin(9600); 
  delay(5000);//GPS ready
 
  Serial.println("AT");   
  delay(2000);
  //turn on GPS power supply
  Serial.println("AT+CGPSPWR=1");
  delay(1000);
  //reset GPS in autonomy mode
  Serial.println("AT+CGPSRST=1");
  delay(1000);
 
  //digitalWrite(4,LOW);  //Enable GPS mode
  //digitalWrite(3,HIGH);  //Disable GSM mode
  delay(2000);

}

////////// Enable GPRS //////////
void enableGPRS() {
  
//  digitalWrite(3,LOW);  //Enable GSM mode
//  digitalWrite(4,HIGH);  //Disable GPS mode
  
  Serial.println("AT+CMEE=1");
  delay(2000); 
  
  Serial.println("AT+CGATT=1");  //attach gprs service
  delay(2000);
  
  Serial.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");  //Set bearer connection type
  delay(2000);
  
  Serial.println("AT+SAPBR=3,1,\"APN\",\"wap.cingular\"");  //set bearer mode
  delay(2000);
  
  Serial.println("AT+SAPBR=3,1,\"USER\",\"wap@cingulargprs.com\"");  //set bearer user
  delay(2000);
  
  Serial.println("AT+SAPBR=3,1,\"PWD\",\"cingular1\"");  //Set bearer pass
  delay(2000);
}

////////// Get Latitude //////////
double getLatitude() {
  
  //enableGPS();

  digitalWrite(4,LOW);  //Enable GPS mode
  digitalWrite(3,HIGH);  //Disable GSM mode
  
  Serial.flushRX();
  
  double latitude = 0;
  char i;
  char lat[10]={
    '0','0','0','0','0','0','0','0','0','0'
  };
 
 
  if( ID())
  {
    comma(2);
    while(1)
    {
      if(Serial.available())
      {
        lat[i] = Serial.read();
        i++;
      }
      if(i==10)
      {
        i=0;
        latitude = Datatransfer(lat,5);  //latitude 
        return latitude;
      }  
    }
  }
}

//////////  Get Lat Direction //////////
char lat_dir() {
  
  //enableGPS();
  
  digitalWrite(4,LOW);  //Enable GPS mode
  digitalWrite(3,HIGH);  //Disable GSM mode
  
  Serial.flushRX();
  
  char i=0, latDir;
 
  if( ID())
  {
    comma(3);
    while(1)
    {
      if(Serial.available())
      {
        latDir = Serial.read();
        i++;
      }
      if(i==1)
      {
        i=0;
        return latDir;
      }  
    }
  }
}

////////// Get Longitude //////////
double getLongitude() {
  
  //enableGPS();
  
  //digitalWrite(4,LOW);  //Enable GPS mode
  //digitalWrite(3,HIGH);  //Disable GSM mode
  
  Serial.flushRX();
  
  double longitude = 0;
  char i;
  char lon[11]={
    '0','0','0','0','0','0','0','0','0','0','0'
  };

  if( ID())
  {
    comma(4);
    while(1)
    {
      if(Serial.available())
      {
        lon[i] = Serial.read();
        i++;
      }
      if(i==11)
      {
        i=0;
        longitude = Datatransfer(lon,5);
        return longitude;
      }  
    }
  }
}

////////// Get Lon Direction //////////
char lon_dir() {
  
  //enableGPS();
  
  digitalWrite(4,LOW);  //Enable GPS mode
  digitalWrite(3,HIGH);  //Disable GSM mode
  
  Serial.flushRX();
  
  char i=0,lonDir;
 
  if( ID())
  {
    comma(5);
    while(1)
    {
      if(Serial.available())
      {
        lonDir = Serial.read();
        i++;
      }
      if(i==1)
      {
        i=0;
        return lonDir;
      }  
    }
  }
}

////////// Match the ID commands //////////
char ID() {
  
  char i=0;
  char value[6]={
    '$','G','P','G','G','A'    };  //match the gps protocol
  char val[6]={
    '0','0','0','0','0','0'    };
 
  while(1)
  {
    if(Serial.available())
    {
      val[i] = Serial.read();  //get the data from the serial interface
      if(val[i]==value[i])  //Match the protocol
      {    
        i++;
        if(i==6)
        {
          i=0;
          return 1;  //break out after get the command
        }
      }
      else
        i=0;
    }
  } 
}

////////// get a comma //////////
void comma(char num)
{   
  char val;
  char count=0;  //count the number of ','
 
  while(1)
  {
    if(Serial.available())
    {
      val = Serial.read();
      if(val==',')
        count++;
    }
    if(count==num)  //if the command is right, run return
      return;
  }
 
}

////////// convert the data to the float type //////////
double Datatransfer(char *data_buf,char num) { //*data_buf：the data array
                                        
  double temp=0.0;  //the number of the right of a decimal point
  unsigned char i,j;
 
  if(data_buf[0]=='-')
  {
    i=1;
    //process the data array
    while(data_buf[i]!='.')
      temp=temp*10+(data_buf[i++]-0x30);
    for(j=0;j<num;j++)
      temp=temp*10+(data_buf[++i]-0x30);
    //convert the int type to the float type
    for(j=0;j<num;j++)
      temp=temp/10;
    //convert to the negative numbe
    temp=0-temp;
  }
  else//for the positive number
  {
    i=0;
    while(data_buf[i]!='.')
      temp=temp*10+(data_buf[i++]-0x30);
    for(j=0;j<num;j++)
      temp=temp*10+(data_buf[++i]-0x30);
    for(j=0;j<num;j++)
      temp=temp/10 ;
  }
  return temp;
}
