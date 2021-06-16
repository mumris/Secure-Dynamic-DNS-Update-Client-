//This software version is tested on M5Stack Atom Light and provided "as is" by Saulius Pakalnis (Mumris).
#include "sdkconfig.h"
#include "esp_system.h"
#include "Adafruit_NeoPixel.h"
#define RGBLEDPIN 27

#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  60*60*4 //less 4 hours period
#include <WiFiClientSecure.h>
//#include <WiFiClient.h>
//Flash memeory of ESP32 Programming

#include <EEPROM.h>
#define EEPROM_SIZE 1000 // this is maximum size
#define MAXCOUNT 100
#define NUMBEROFFATTEMPTS 10
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, RGBLEDPIN, NEO_GRB + NEO_KHZ800); 
struct  MyObject
{
  char a[256];
};

char Parsed_IP_address[50];
char Public_IP_address[1000];

RTC_DATA_ATTR long unsigned int bootCount = 0;
//const char*  server = "www.duckdns.org";  // for an example DDNS Server URL
//Some of Public IP (plain text) servers requests:
//  https://ipecho.net/plain
//  https://icanhazip.com/ 
//  https://ipv4bot.whatismyipaddress.com/
//  https://httpbin.org/ip
//  https://api.my-ip.io/ip.txt
//  https://ip.seeip.org/

/////////////////////////////////////////////////////////
//You can use x.509 client certificates if you want
//const char* test_client_key = "";   //to verify the client
//const char* test_client_cert = "";  //to verify the client
////////////////////////////////////////////////////////////
const char* test_root_ca= NULL; //we (as client) do not care about sertificate of DDNS.
WiFiClientSecure client;
MyObject aaa, bbb, ccc, ddd, eee, fff, serv1, serv2, serv3;
char Public_IP_serverName[256];
char DDNS_server_Name[256];
char WebResponse[256];

void setup() 
{    
  int error =0;
  int no_error =0;
  pixels.begin();
  pixels.setPixelColor(0, pixels.Color( 0,0,25)); 
  pixels.show();
    
  //Initialize serial and wait for port to open:
   Serial.begin(115200);       
   Serial.println ("The module every 4 hours sends http GET requests. You can modify the command and save it into nonvolatile memory");
   Serial.println ("Usefull feature to auto update your Public IP address in DDNS server");
   //Serial.println ("GET command example: https://www.duckdns.org/update?domains=PUT_YOUR_NAME&token=e2403930-f58a-4237-a162-16f28444d825"); 
   Serial.setTimeout(30000); // set timeout of 30 seconds
   EEPROM.begin(EEPROM_SIZE); 
     
  /// fill get public ip address requests if they are not set 
  if_not_exists_write_to_EPROOM_string(400, "https://ip.seeip.org/");
  if_not_exists_write_to_EPROOM_string(500, "https://api.my-ip.io/ip.txt");
  if_not_exists_write_to_EPROOM_string(600, "https://icanhazip.com/");
  
  EEPROM_setup();  
  show_EEPROM_setup();
  if (Connect_to_WIFI(bbb.a, ccc.a)) // if 1 it is connected t o WIFI, if 0 - FAiled to connect to WIFI
  {     
    /////////////WIFI CONNECTED////////////////              
     bool Got_Public_IP = false;
     if (get_IP_address_from_Public_server(& serv1, Public_IP_serverName, Public_IP_address)) 
     {
     Serial.println("serv1:  " + (String) serv1.a);
     Got_Public_IP = true;  // true = got parsed public ip adddress
     }
     else 
     {
     if (get_IP_address_from_Public_server( & serv2, Public_IP_serverName, Public_IP_address))  
      {
       Serial.println("serv2:  " + (String) serv2.a);
       Got_Public_IP = true;
      }
     else 
      {
       if (get_IP_address_from_Public_server(& serv3, Public_IP_serverName, Public_IP_address))  
        {
         Serial.println("serv3:  " + (String) serv3.a);
        Got_Public_IP = true;
        }
        else
        { 
        Got_Public_IP = false;
        }
      }
     } 
   /////////////  
     if (Got_Public_IP)     
     {   
         //;ok got parsed IP address
         Serial.print("My public IP address is: "); Serial.println(Parsed_IP_address);       
         //// compare public IP address from memeory to IP just obtained     
        
        if( !Compare_two_IP_addresses(Parsed_IP_address) || bootCount == 0) //old IP not equal to new  IP
        {
          Serial.println("Let's update it in EEPROM and on DDNS server");  
          //first save new IP address
          EEPROM.put(300, Parsed_IP_address); 
          EEPROM.commit();       
          //get server name from fttps string
            copyA(ddd.a, DDNS_server_Name, 250); //web server server name
            GetServerName(DDNS_server_Name);               
          //upgrade ddd https request with new IP address    
          find_and_replace_string(ddd.a, ccc.a, Parsed_IP_address); //"MY_PUBLIC_IP_ADDR" string is replaced with real IP address  
           
          if (https_get(&ccc, DDNS_server_Name, WebResponse))   //try to connect to remote web page server with GET command
          {
          Serial.print("Content returned from DDNS site: ");
         
          Serial.println(DDNS_server_Name);
          Serial.println(); 
          Serial.println(WebResponse);//what to do with the response?//

           int k;
           for (k=0; (k < 80 && aaa.a[k] != '\n' && aaa.a[k] != '\r' && aaa.a[k] != 255 && aaa.a[k] != '\0') ; k++) ;
           //Serial.print("k is: "); Serial.println(k, DEC);
          if (k>0)
          {
            if (strstr(WebResponse, aaa.a)) 
            {
            Serial.println("DDNS response is as expected. Perfect.");         
            pixels.setPixelColor(0, pixels.Color( 0,10,0)); // put white color RGB LED output
            pixels.show();
            } 
            else
            {
            Serial.println("DDNS response is NOT THE SAME as expected.");
            pixels.setPixelColor(0, pixels.Color( 10,0,0)); // put white color RGB LED output
            pixels.show();
            } 
         }
         else  
         {
         Serial.println("DDNS response is NOT VERIFIED, as expected response is not set.");
         pixels.setPixelColor(0, pixels.Color( 0,10,0)); // put white color RGB LED output
         pixels.show();
         }        
        } 
        }
        else   
        {
        Serial.println("Equal IP strings. IP address did not change from last check."); 
        pixels.setPixelColor(0, pixels.Color( 0,10,0));  
        pixels.show();
        }              
     }
     else
     { 
     Serial.println("IP address not found.");
     pixels.setPixelColor(0, pixels.Color( 10,0,0)); // red RGB output
     pixels.show();
      //get out with error;
    }   
      
    ///////////////  
   WiFi.disconnect();   
 }
 else //failed to connect to WIFI
 {
    pixels.setPixelColor(0, pixels.Color( 10,0,0)); //blue color WIFI connect error
    pixels.show(); 
 }
 
/////Anyway go to sleep for 4 hours//////////////////////////

  Serial.println("Boot number: " + String(bootCount));
  delay(1000);
  Serial.flush(); 
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
  if (bootCount == 0) 
  {
   Serial.println("Going to sleep just for 1 minute now.");
  esp_sleep_enable_timer_wakeup(60 * uS_TO_S_FACTOR);
  }
  else 
  {
  Serial.println("Going to sleep 4 hours now and every next time.");
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  }
   //Increment boot number and print it every reboot
  ++bootCount;
  esp_deep_sleep_start();  
  //reboot after sleep/////
}
//////////////////
void loop()
{
  //This should be not to be called if esp_deep_sleep_start() runs before.
  Serial.println("I am in loop");
  pixels.setPixelColor(0, pixels.Color( 10,0,0));
  pixels.show(); 
  delay(500);
  pixels.setPixelColor(0, pixels.Color( 0,0,0));
  pixels.show(); 
  delay(500);
}

/////////////////////////////////////
bool if_not_exists_write_to_EPROOM_string(int addr, const String server_name)  
  {
  char str[80];
  int k;
  EEPROM.get(addr, str);                      // expected ip address server response to verify

  for (k=0; (k < 80 && str[k] != '\n' && str[k] != '\r' && str[k] != 255 && str[k] != '\0') ; k++) ;
  //Serial.print("k is: "); Serial.println(k, DEC);
  if (k >8 && k<75) return false;
  else 
  {
   writeStringToEEPROM(addr, server_name);
   return true;
  }
  }
  //////////
  
  void writeStringToEEPROM(int addr, String strToWrite)
  {
  byte len = strToWrite.length();
  int i;
  for (i = 0; i < len; i++)
  {
    EEPROM.write(addr + i, strToWrite[i]);
  }
   EEPROM.write(addr + i, '\0');
   EEPROM.commit(); 
   delay(20);
  }

///////////////////////////////
bool get_IP_address_from_Public_server(MyObject * server, char Public_IP_serverName[], char Public_IP_address[])  //serv1.a
 {
      copyA(server->a, Public_IP_serverName, 250); 
      GetServerName(Public_IP_serverName);   // get URL from EEPROM              
     
      if (https_get(server, Public_IP_serverName, Public_IP_address)) //try to get public IP address with GET command
     {       
      //GOT RESPONSE FROM WHAT IS MY PUBLIC IP SERVER//////  
      Serial.print("Content returned from ");  
      Serial.print(server->a);      
      Serial.print(" site is: "); 
      Serial.println(Public_IP_address);
      Serial.println();
        if (parseIP(Public_IP_address, Parsed_IP_address)) //if 1 -Error
        {
         return false;//on error 
        }
        else
        {
         return true;  //ok got parsed IP address
        }
     } 
   return false;  //get request failed
 }


/////////////////////////
bool Compare_two_IP_addresses(char Parsed_IP_address[])
{ 
 EEPROM.get(300, eee);    //extract IP from memory
 remove_End_of_Line(&eee);           
 Serial.print("IP address from EEPROM: ");
 Serial.println(eee.a);
 //Serial.print("Length: ");  Serial.println(strlen(eee.a), DEC); 
  if (strlen(eee.a) <6 && strlen(eee.a) >15) return false;    //old IP address isn't in IP range       
  int lenghtstr = strlen(eee.a);
  for (int i = 0; i < lenghtstr; i++)
  {
  if(eee.a[i] != Parsed_IP_address[i])  return false;             
  }
  return true; 
}  

//////////////////////////////////
 void EEPROM_setup()
{
 Serial.println("ENTERING SETUP - You have 30 seconds to press to type 'y' <ENTER>: ");
 String ss = Serial.readStringUntil('\n'); //with timeout
 char serverName[256];
 if (ss.indexOf('Y')>=0 || ss.indexOf('y')>=0)
  {
  pixels.setPixelColor(0, pixels.Color( 10,10,0));
  pixels.show();   
  write_to_EEPROM(0,   "Would you like to set network WIFI name?", "Now enter WIFI network name: "); //address = 0  
  write_to_EEPROM(100, "Would you like to set network WIFI password?", "Now enter WIFI network password: "); //address = 200    
  Serial.println("You can insert 'MY_PUBLIC_IP_ADDR' string in https request. It will be replaced with real public IP address");
  write_to_EEPROM(700, "Would you like to set https string as you in browser", "Enter smth like in example: https://www.duckdns.org/update?domains=PUT_YOUR_NAME&token=e2403930-f58a-4237-a162-16f28444d8f7"); //address = 400
  write_to_EEPROM(200, "Would you like to set expected true response to https request?", "Enter word or phrase of expected response to verify: "); 
  Serial.println("Some of Public IP (plain text) servers requests."); 
  Serial.println("They are already preset, but you can modify them:");
  Serial.println("https://ipecho.net/plain");
  Serial.println("https://icanhazip.com/");  
  Serial.println("https://ipv4bot.whatismyipaddress.com/");
  Serial.println("https://httpbin.org/ip");
  Serial.println("https://api.my-ip.io/ip.txt");
  Serial.println("https://ip.seeip.org/");
  Serial.println("https://v4.ident.me/");      
  write_to_EEPROM(400, "Would you like to set first request to get public IP address?", "Enter public IP server request: "); 
  write_to_EEPROM(500, "Would you like to set secont request to get public IP address?", "Enter public IP server request: "); 
  write_to_EEPROM(600, "Would you like to set third request to get public IP address?", "Enter public IP server request: ");      
  }
}
/////////////////////////////////
  void show_EEPROM_setup()
  {
  pixels.setPixelColor(0, pixels.Color( 0,0,10));
  pixels.show();
  EEPROM.get(0, bbb);                 //0 wifi name
  Serial.print("WIFI NAME is: ");
  Serial.print(bbb.a);
  
  EEPROM.get(100, ccc);               // wifi password
  Serial.print("WIFI PASSWORD is: ");
  Serial.print(ccc.a);
   
  EEPROM.get(200, aaa);                      // expected ip address server response to verify
  int k;
  for (k=0; (k < 80 && aaa.a[k] != '\n' && aaa.a[k] != '\r' && aaa.a[k] != 255 && aaa.a[k] != '\0') ; k++) ;
  Serial.print("Expected server response to https request: "); 
  if (k==0)  Serial.println("String is empty");
  else Serial.println(aaa.a);
  
  EEPROM.get(300, eee);                     // saved public address
  if (strlen(eee.a) >6 && strlen(eee.a) <17) 
  {
  Serial.print("Saved Public IP address is: ");
  Serial.println(eee.a);
  }
  else Serial.println("No Public IP address is saved: ");
    
  EEPROM.get(400, serv1);                      // expected ip address server response to verify
  Serial.print("Public IP server1 name: ");
  Serial.println(serv1.a);
    
  EEPROM.get(500, serv2);                      // expected ip address server response to verify
  Serial.print("Public IP server2 name: ");
  Serial.println(serv2.a);

  EEPROM.get(600, serv3);                      // expected ip address server response to verify
  Serial.print("Public IP server3 name: ");
  Serial.println(serv3.a);
  
  EEPROM.get(700, ddd);
  Serial.print("HTTPS Request to DDNS is: ");   //https big string
  Serial.print(ddd.a);
  
  remove_End_of_Line(&aaa);
  remove_End_of_Line(&bbb);
  remove_End_of_Line(&ccc); 
  remove_End_of_Line(&ddd); 
  remove_End_of_Line(&eee);
  remove_End_of_Line(&serv1);
  remove_End_of_Line(&serv2); 
  remove_End_of_Line(&serv3);  
  }  
/////////////////////////////////
void GetServerName(char ServerName[])
{
  const char* substring1 = "http";
  char * pch;
  char * p;
  p = strstr (ServerName, substring1);
  
  if(p)
  {
  pch = strtok (ServerName, "/:");
  pch = strtok (NULL, "/:");
  }
  else pch = strtok (ServerName, "/:");
  
  copyA(pch, ServerName, 100); 
}
//////////////////////////////////
void copyA(char * src, char * dst, int len) 
{
    memcpy(dst, src, sizeof(src[0])*len);
}


//////////////////////
void remove_End_of_Line(MyObject * bbb)
{
  int str_lenght = strlen(bbb->a);
  for (int k=0; k < str_lenght; k++)
  {
  char inByte = bbb->a[k];
  //find and remove /n character
  if (inByte == '\r' || inByte == '\n')  bbb->a[k]='\0'; //null termination
  }
}

//////////////////////////
int write_to_EEPROM(int adress, String my_name1, String my_name2) 
{  

 MyObject str;  //create structure str 
 String s1;
 s1.reserve(256); 
 s1 = "                                                           ";  //s1 is String type variable.
 int timeoutCount =0;
 int saved = 0;
 for (int k=0; k < sizeof(str.a); k++) str.a[k] = '\0';
 
 //you have to set it #define SERIAL_BUFFER_SIZE 256 in RingBuffer.h file, also
 //#define EPX_SIZE 256 // 64 for Full Speed, EPT size max is 1024 in USBAPI.h file
  while (!Serial && timeoutCount < MAXCOUNT) //wait until serial is available or timeout
  {
  timeoutCount++;
  delay(100);
  }

   if (timeoutCount<=MAXCOUNT)
   {
   Serial.println (my_name1); 
   Serial.setTimeout(30000); // set timeout of 30 seconds
   Serial.println("You have 30 seconds to press to type 'Y' or 'y': ");
   s1 = Serial.readStringUntil('\n'); //with timeout
        if (s1.indexOf('Y')>=0 || s1.indexOf('y')>=0)
           {
           // Process message when new line character is recieve                  
            s1 = ""; // clear string
            Serial.print("Yes received. ");                                
            while (Serial.available()) Serial.read();   //clean buffer
            Serial.setTimeout(50); // now set timeout of 0.1 seconds                    
            Serial.println (my_name2);
            while (!Serial.available()) delay (10); //wait for string            
              int i = 0;
              int k= Serial.available();
              char inchar ='\0';
              while ((k  > 0) && (k < 400)) //no more as 400 characters string
               {
                 inchar = Serial.read();  
                 str.a[i] = inchar;           
                 if (k < 3) delay (10); //gives possibility to collect more characters
                 k = Serial.available();
                 i++;
               }                          
              str.a[i]='\0'; //null termination
        
         if (i>1) 
         { 
         //EEPROM.put(adress, str); //EEPROM.put(10,"https://www.duckdns.org/update?domains=PUT_YOUR_NAME&token=e2403930-f58a-4237-a162-16f28444d8f7"); 
         // EEPROM.commit();  
         writeString(adress, str.a, i); 
         delay(100); 
         saved =1;             
         }
       else 
       {
       Serial.println("No Y received. Then proceeding with previous settings.");
       saved = 0;
       }   
   } //end if
  }
  Serial.print("This string is saved or extracted from EEPROM: ");   
  EEPROM.get(adress, str); 
  s1 = String(str.a);
  Serial.println(s1);
  return saved;
}

 /////////////////////////////////////////////////////////
void writeString(int add, char data[], int _size)
{
  //int _size = data.length();
  int i;
  for(i=0;i<_size;i++)
  {
    EEPROM.write(add+i,data[i]);
  }
  EEPROM.write(add +_size,'\0');   //Add termination null character for String Data
  EEPROM.commit();
}

/////////////////////////////////////////////////////////
  
int  Connect_to_WIFI(char ssid[], char password[]) 
  {
  int result = 0;
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);
  
  //WiFi.disconnect(true);
delay(50);
WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
WiFi.setHostname("android-ce166eae64b93da4"); // set hostname     
delay(50);
WiFi.begin(ssid, password);
  
  // attempt to connect to Wifi network:
  while ((WiFi.status() != WL_CONNECTED) && (result < NUMBEROFFATTEMPTS)) 
  {
    Serial.print(".");
    // wait 3 second for re-trying
    delay(3000);
    result++;
  }
   if (result < NUMBEROFFATTEMPTS)
   {
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.println("");
    Serial.print("Local IP address: ");
    Serial.println(WiFi.localIP());
    return 1; // OK
   } 
   else  
   {
   Serial.println("FAILED connect to WIFI! ");
   return 0; //Failed to connect to WIFI
   }
  }  
////////////////////////////////////////////////////

int https_get(MyObject * str, char httpsServer[], char sss[]) 
  {
   String line;
  char a[256];
  client.setCACert(test_root_ca);
  //client.setCertificate(test_client_key); // for client verification
  //client.setPrivateKey(test_client_cert);  // for client verification

  Serial.println("\nStarting connection to server:  " + String (httpsServer));
  if (!client.connect(httpsServer, 443))
    Serial.println("Connection failed!");
  else {
    Serial.println("Connected to server!");  
    Serial.print("Making GET secure request to ");  
    Serial.println(httpsServer);
    Serial.print("GET ");
    Serial.print(str->a);
    Serial.print(" HTTP/1.0"); 
    Serial.println("\n"); 
                         
    client.print("GET ");
    client.print(str->a);
    client.print(" HTTP/1.0"); 
    client.print("\n");    
    client.print("Host: ");
    client.println(httpsServer);
    client.println("Connection: close");
    client.println();             
    Serial.println("HEADER TOP"); 
    while (client.connected()) 
    { 
    line = client.readStringUntil('\n');    
    Serial.print(line);  //This is Header IMPORTANT
    if (line == "\r" || line == "\0" ) break;
    }    
    Serial.println("HEADER END");
    line ="";        
    int k = 0;
    char inchar ='\0';
              if (client.available()) //read rest text
              {
              for (int i= 0; i < 80; i++) //no more as 80 characters string
               {
                 inchar = client.read();  
                 if (inchar == '\0' || inchar == 255) break;     
                 a[k] = inchar;                     
                 k++;     
               }                          
              a[k]='\n'; //null termination 
              k++;
              a[k]='\0'; //null termination                   
              copyA(a, sss, k); 
              }      
    delay(30); //must give time to process
    //Serial.print("k= "); Serial.println(k, DEC);                         
    Serial.println("Client closed");
    client.stop();
  return 1;
  }
return 0;
}
//////////////////////  
//"MY_PUBLIC_IP_ADDR" pattern is replaced with real IP address
void find_and_replace_string(char input[], char output[], char IP_address[]) 
{
char *_index; 
char * _start = &input[0];
int input_Lenght;
int IP_address_Lenght;
int i=0;
for ( i=0; input[i] != '\0'; i++);
input_Lenght = i+1;
for (i=0; IP_address[i] != '\0'; i++); 
IP_address_Lenght = i;
//Serial.print("input string Lenght = ");
//Serial.println(input_Lenght);

 _index = strstr(input, "MY_PUBLIC_IP_ADDR");
 if (_index != NULL)
 {
 int ii=0;
 for (char *_i = _start; _i != _index; ++_i)
 {
 output[ii] = input[ii];
 //Serial.print("ii0: ");  Serial.println(output[ii]);
 ii++;
 }
 int next = ii+17; //points to input char after MY_PUBLIC_IP_ADDR string
//Serial.print("next: ");  Serial.println(input[next]);
//
for (int i=0; i<IP_address_Lenght; i++)
 {
  output[ii] = IP_address[i];
 //Serial.print("ii1: ");  Serial.println(output[ii]);
  ii++;
 }
///

 for (int i=next; i<input_Lenght; i++)
 {
 output[ii] = input[i];
 //Serial.print("ii2: ");  Serial.println(output[ii]);
  ii++; 
  }  
  output[ii]='\0';
 }
else 
 {
 Serial.println("Optional 'MY_PUBLIC_IP_ADDR' substring for replacement with real IP address is not found in GET command.");
 copyA(input, output, input_Lenght+1); 
 }
 Serial.println(output);
}

//////////////////////////////////////////////////////
int parseIP(char input[], char output[])
{
  int i = 0;
  int minn = 0;
  int maxx = 0;
  char test[10000];  
  //chrono = micros();
  
  removeExtraChars(input);

  //Serial.println(); 
  //Serial.print("Cleaned IPstring: "); 
  //Serial.println(input);
  //delay(10);
  while ((input[i] != ' ') && (input[i] != '\0')) i++; // find the "space" char
  if (input[i] == '\0')  //"space" char is not found
  {
  if(i < 7) return 1; //"space" char is not found and string length <7 It cannot be IP address
  else 
  {
 //"space" char not found but string length >=7. Validate IP from begining of the string
  copySubstring(test, input, 0,  15);
  //Serial.print("Substring with no space is: "); 
  //Serial.println(test);
  int error = checkSubstring_for_IP(test, output);  
  return error; // if IP found return 0, else return 1;  
  }
  }
  else //"space" char found but not at zero position. But first we must validate the substring before for IP
  {   
  /////Validate string for IP before "space" char.
  if (i > 6)
  {
  minn = 0;
  maxx = 15;
  copySubstring(test, input, minn,  maxx);
  //Serial.print("Substring before space is: "); 
  //Serial.println(test);
  int error = checkSubstring_for_IP(test, output);  
  if (error ==0) return 0; // IP found return
  } 
  //IP in substring is not found or is less than 7, lets test string after "space" char
  /////Validate string for IP after "space" char.
     for (int spaceNumber = 0; spaceNumber < 50; spaceNumber++)
     {
      i++ ; // skip the "space" char or set i to 0 if no "space" char
      minn = i;
      maxx = i+ 15;
      //Serial.print("index is: "); 
      //Serial.println(i);
     copySubstring(test, input, minn, maxx );
     //Serial.print("Substring after space is: "); 
     //Serial.println(test);     
     int error = checkSubstring_for_IP(test, output);  
    if (error == 0) return 0; //found IP string return
    spaceNumber++; 
    //find next space and repate last test for IP address validation   
    i=minn;
    while ((input[i] != ' ') && (input[i] != '\0')) i++; // find the "space" char
    if (input[i] == '\0')  return 1; //"space" char is not found, IP address also   
    }
}
}

/////////////////////////////////
void removeExtraChars(char input[])
{
   int j = 0; 
//replace ALL no IP characters with SPACES////////
    for(int i = 0; input[i] != '\0'; i++) 
    {
    if ((input[i] >= '0' && input[i] <= '9') || input[i] == '.' || input[i] == ' ')  input[j++] = input[i];    
    else input[j++] = ' '; 
    }  

/// replace dublicated char points with spaces
    j = 1;    
    for(int i = 1; input[i] != '\0'; i++) //replace dublicated char points with spaces
    {
       if (input[i] == '.') //char point found in midle what to do with it?
       {
        if((input[i-1] == ' ') && (input[i+1] == '.'))  ; // removing point: space, point, point
        else if((input[i-1] == ' ') && (input[i+1] == ' '))  ; // removing point: space, point, space
        else if((input[i-1] == '.') )  ;// removing point: point,point,nomatter
        else if((input[i-1] != '.') ) input[j++] =  '.';// nonpoint,point,nomatter Write it.
       }
        else input[j++] =  input[i]; //non point midle character found. Just Write it
    }
 input[j]='\0'; //terminate string at the j position

///remove dublicated SPACES
    j = 1;    
    for(int i = 1; input[i] != '\0'; i++) //Now remove dublicated SPACES
    {
       if (input[i] == ' ') //space found
       {
        if(input[i-1] != ' ' )  input[j++] = ' ' ; // found not dublicated space. Write it.
        else ; // if dublicated found do nothing
       }
    else input[j++] =  input[i]; //other character found write it
    }
 input[j]='\0'; //terminate string at the j position

}

////////
void copySubstring(char test[], char items[], int minn,  int maxx)
{
  maxx++;
  int k;
  int i=0;
  for (k = minn ; k < maxx ; k++) 
  {
  test[i] = items[k];
  i++;
  }
  test[k] = '\0';
}
 
//////////////////////////   
int checkSubstring_for_IP( char input[], char output[])  
  {  
    char test[100]; 
    uint8_t ip[4] ={0,0,0,0}; 
    byte part = 0 ; // index into ip_addr
    unsigned int i = 0, charvalue =0, done =0;
        
    strcpy(test, input);    
    while (true)
    {
      if (input[i] == '.' || input[i] == '\0') // part separator / end of string
      { 
        part++ ;  // skip to next byte
        if (input[i] == 0 || part >= 4) {done=1; break;}// done if 4 parts parsed or end of string         
        else
        ip[part] = 0 ;
      } 
      else if (input[i] >= '0' && input[i] <= '9') 
      {
        charvalue = ip[part] * 10 + (input[i] - '0') ; // build up value from decimal digits.        
        //Serial.print("ip: "); Serial.println(part);
        //Serial.print("Charvalue: "); Serial.println(charvalue, DEC);
        if (charvalue > 255) return 1; // error, unsigned byte cannot be large as 255.        
        else ip[part]= charvalue;
      }      
      else {done=0; break;}//no conversion appeared, exit     
      i++ ;  // step to next char
    }
  
  if (done ==0 && part<3) return 1;

  if (done ==0 && part<3) return 1;
  sprintf(output,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);
  i = ip[0]+ip[1]+ip[2]+ip[3];
  if (i == 0) return 1;
  else return 0;
 }
 /////////////////////////////////
