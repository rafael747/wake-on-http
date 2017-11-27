char junk; //https://forum.arduino.cc/index.php?topic=375865.0

//#define LCD_DISPLAY

#include <SPI.h>
#include <Ethernet.h>

#ifdef LCD_DISPLAY
#include <LiquidCrystal.h>
#endif

#include <utility/socket.h>
#include <ICMPPing.h>
//#include <SD.h>

#ifdef LCD_DISPLAY
LiquidCrystal lcd(9, 8, 7, 6, 5, 4);
#endif

// Entre com os dados do MAC e ip para o dispositivo.
// Lembre-se que o ip depende de sua rede local
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(10,0,0,2);
 
// Inicializando a biblioteca Ehternet
// 80 é a porta que será usada. (padrão http)
EthernetServer server(80);
String readString; 
SOCKET pingSocket = 0;
ICMPPing ping(pingSocket, (uint16_t)random(0, 255));

int alert=0;
int pos;
int pos2;

void setup() {
 // Abrindo a comunicação serial para monitoramento.
  Serial.begin(9600);
  
  #ifdef LCD_DISPLAY
  //Inicializando LCD
  lcd.begin(16, 2);
  lcd.print("IP por DHCP.....");
  #endif
  
  delay(1000);
  //tentar pegar ip por dhcp
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    
    #if defined(LCD_DISPLAY)
    lcd.setCursor(0,0);  
    lcd.print("Falha no DHCP!");
    delay(3000);
    lcd.setCursor(0,0);  
    lcd.print("Fixando IP......");
    #endif
    
    delay(2000);
    // colocar ip estatico
    Ethernet.begin(mac, ip);
    // no point in carrying on, so do nothing forevermore:
  }
  
  //  ///////////////// SD CARD
//  Serial.println("Initializing SD card...");
//    if (!SD.begin(4)) {
//        Serial.println("ERROR - SD card initialization failed!");
//        return;    // init failed
//    }
//    Serial.println("SUCCESS - SD card initialized.");
//    // check for index.htm file
//    if (!SD.exists("index.htm")) {
//        Serial.println("ERROR - Can't find index.htm file!");
//        return;  // can't find index file
//    }
//    Serial.println("SUCCESS - Found index.htm file.");
/////////////////////////////////
  
  // Inicia a conexão Ethernet e o servidor;
  
  //inicia o server
  server.begin();
  
  #if defined(LCD_DISPLAY)
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Servidor em: ");
  lcd.setCursor(0,1);
  lcd.print(Ethernet.localIP());
  #endif
}

void loop() {
  
  // Create a client connection
  EthernetClient client = server.available();
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();

        //read char by char HTTP request
        if (readString.length() < 100) {

          //store characters to string
          readString += c;
          //Serial.print(c);
        }

        //if HTTP request has ended
        if (c == '\n') {

          ///////////////
          Serial.println(readString); //print to serial monitor for debuging
                      String ipchecks;

          
          if(readString.indexOf("CHECK") > 0 ) // check if alive
          {
            pos = readString.indexOf(":");
            pos2 = readString.indexOf(" HTTP");
            
            ipchecks = readString.substring(pos+1, pos2);
            
            byte ipcheckb[4];
            parseBytes(ipchecks.c_str(), '.', ipcheckb, 4, 10);
            
            ICMPEchoReply echoReply = ping(ipcheckb, 1);
            if (echoReply.status == SUCCESS)
            {
              alert = 1;
                
//               
//                  sprintf(buffer,
//                  "Host: %d.%d.%d.%d is alive! time=%ldms TTL=%d",
//                    echoReply.addr[0],
//                    echoReply.addr[1],
//                    echoReply.addr[2],
//                    echoReply.addr[3],
//                    millis() - echoReply.data.time,
//                    echoReply.ttl);
            }
            else
            {
              alert = 2;
              //  sprintf(buffer, "Host: %s is NOT alive! Echo request failed", ipchecks.c_str());
             
            }
            
            
          }
          
          client.println("HTTP/1.1 200 OK"); //send new page
          client.println("Content-Type: text/html");
          client.println();
          
          
         // client.println("<HTML><HEAD><TITLE>Arduino Wake-on-Lan Center</TITLE></HEAD><BODY><h1>Arduino Wake-on-Lan Center</h1><h2>Power ON your devices!</h2><p>You can send wake-on-lan packets to these devices. To make sure they started, you can check connection using ICMP.</p><table border=\"1\" width=\"706\"><tbody><tr><td><strong>Device</strong></td><td><strong>IP</strong></td><td><strong>MAC</strong></td><td><strong>Wake</strong></td><td><strong>Check if alive</strong></td></tr><tr><td>PC - Casa</td><td>192.168.0.102</td><td>20-CF-30-55-5B-B2</td><td><a href=\"/?20cf30555bb2:192.168.0.102\">Wake me</a></td><td><a href=\"/?CHECKALIVE:192.168.0.102\">Check</a></td></tr><tr><td>Notebook</td><td>10.0.0.1</td><td>04-7D-7B-E4:76:51</td><td><a href=\"/?047D7BE47651:10.0.0.1\">Wake me</a></td><td><a href=\"/?CHECKALIVE:10.0.0.1\">Check</a></td></tr></tbody></table>");
          
//       
          client.println("<HTML><HEAD><TITLE>Arduino Wake-on-Lan Center</TITLE></HEAD><BODY><h1>Arduino Wake-on-Lan Center</h1>Power ON your devices!</h2><p>You can send wake-on-lan packets to these devices. To make sure they started, you can check connection using ICMP.</p>");

          client.println("<table border=\"1\" width=\"706\"><tbody><tr><td><strong>Device</strong></td><td><strong>IP</strong></td><td><strong>MAC</strong></td><td><strong>Wake</strong></td><td><strong>Check if alive</strong></td></tr>");

           //client.println("<tr><td>PC - Casa</td><td>192.168.0.102</td><td>20-CF-30-55-5B-B2</td><td><a href=\"/?20cf30555bb2:192.168.0.102\">Wake me</a></td><td><a href=\"/?CHECKALIVE:192.168.0.102\">Check</a></td></tr>");
          //client.println("<tr><td>PC - Casa</td><td>10.0.0.1</td><td>20-CF-30-55-5B-B2</td><td><a href=\"/?20cf30555bb2:10.0.0.1\">Wake me</a></td><td><a href=\"/?CHECKALIVE:10.0.0.1\">Check</a></td></tr>");

          client.println("<tr><td>Notebook</td><td>10.0.0.1</td><td>04-7D-7B-E4:76:51</td><td><a href=\"/?047D7BE47651:10.0.0.1\">Wake me</a></td><td><a href=\"/?CHECKALIVE:10.0.0.1\">Check</a></td></tr>");
          client.println("</tbody></table><br><br>");

          if(alert)
          {
              Serial.println(ipchecks.c_str());
              client.println("Host ");
              client.println(ipchecks);
              if (alert == 2 ) client.println(" is NOT alive!");
              else client.println(" is alive!");
              
              alert=0;

          }
           
           if(readString.indexOf("/?") > 0 && readString.indexOf("?CHECK") < 0 ) //tem href
                 client.println("Pacotes enviados, verifique a conexao! ");


           
         
         client.println("</BODY></HTML>");

           delay(10);
          //stopping client
          client.stop();


          //
          // GET the header and lookup for the mac address
          // get substring: ?.......' '
          //
       
          if(readString.indexOf("/?") > 0 && readString.indexOf("?CHECK") < 0 ) //tem href
          {
            pos = readString.indexOf("/?");
            String mac = readString.substring(pos+2, pos+14);
            Serial.println("Tem href, mac=");
            Serial.println(mac);
            
            pos = readString.indexOf(":");
            pos2 = readString.indexOf(" HTTP");
            
            String ipchecks;
            ipchecks = readString.substring(pos+1, pos2);
            Serial.println("IP=");
            Serial.println(ip);
            
            wake(mac, ipchecks);

            
          }
          //clearing string for next read
          readString="";

        }
      }
    }
  }
}

void resetLCD()
{
  #if defined(LCD_DISPLAY)

  delay(2000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Servidor em: ");
  lcd.setCursor(0,1);
  lcd.print(Ethernet.localIP());
  #endif
}


int wake (String hostMac, String hostIp) {

  //Serial.println(mac2StringC(host));
 
 byte mac[6];
 parseBytesNoSep(hostMac.c_str(),2, mac, 6, 16);
 
 byte ip[4];
 parseBytes(hostIp.c_str(), '.', ip, 4, 10);
 
 #if defined(LCD_DISPLAY)
 
 lcd.clear();
 //lcd.print("IP:");
 lcd.setCursor(0,0);
 lcd.print(hostIp);
 lcd.setCursor(0,1);
 //lcd.print("MAC:");
 //lcd.setCursor(4,1);
 lcd.print(mac2StringC(mac));
 delay(3000);
 lcd.clear();
 
 #endif
 
 
 
 return SendWOLMagicPacket(mac, ip);
  
  
  
}

void parseBytes(const char* str, char sep, byte* bytes, int maxBytes, int base) {
    for (int i = 0; i < maxBytes; i++) {
        bytes[i] = strtoul(str, NULL, base);  // Convert byte
        str = strchr(str, sep);               // Find next separator
        if (str == NULL || *str == '\0') {
            break;                            // No more separators, exit
        }
        str++;                                // Point to next character after separator
    }
}

void parseBytesNoSep(String str, int passo, byte* bytes, int maxBytes, int base) {
    for (int i = 0; i < maxBytes; i++) {
        bytes[i] = strtoul(str.substring(0,2).c_str(), NULL, base);
        //Serial.println(strtoul(str.substring(0,2).c_str(), NULL, base));
        str = str.substring(2);               // Find next separator
        if (str == NULL ) {
            break;                            // No more separators, exit
        }
        //str++;                                // Point to next character after separator
    }
}
int SendWOLMagicPacket(byte * pMacAddress, byte * ipAddr)
{
 // The magic packet data sent to wake the remote machine. Target machine's
 // MAC address will be composited in here.
 const int nMagicPacketLength = 102;
 byte abyMagicPacket[nMagicPacketLength] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
 byte abyTargetIPAddress[] = { 255, 255, 255, 255 }; // don't seem to need a real ip address.
 const int nWOLPort = 7;
 const int nLocalPort = 8888; // to "listen" on (only needed to initialize udp)


 // Compose magic packet to wake remote machine.
 for (int ix=6; ix<102; ix++)
   abyMagicPacket[ix]=pMacAddress[ix%6];

 //if (UDP_RawSendto(abyMagicPacket, nMagicPacketLength, nLocalPort, abyTargetIPAddress, nWOLPort) != nMagicPacketLength)
 if (UDP_RawSendto(abyMagicPacket, nMagicPacketLength, nLocalPort, ipAddr, nWOLPort) != nMagicPacketLength)
 {
   Serial.println("Error sending WOL packet");
   #if defined(LCD_DISPLAY)
   lcd.setCursor(0,0);
   lcd.print("ERRO:DISPOSITIVO");
   lcd.setCursor(0,1);
   lcd.print("NAO ENCONTRADO");
   resetLCD();
   #endif
   
   return 1;
 }
 else
 {
   #if defined(LCD_DISPLAY)
   lcd.setCursor(0,0);
   lcd.print("Pacotes enviados");
   lcd.setCursor(0,1);
   lcd.print("Teste a Conexao!");
   resetLCD();
   #endif
   return 0;
 }
}

int UDP_RawSendto(byte* pDataPacket, int nPacketLength, int nLocalPort, byte* pRemoteIP, int nRemotePort)
{
 int nResult;
 int nSocketId; // Socket ID for Wiz5100

 // Find a free socket id.
 nSocketId = MAX_SOCK_NUM;
 for (int i = 0; i < MAX_SOCK_NUM; i++)
 {
   uint8_t s = W5100.readSnSR(i);
   if (s == SnSR::CLOSED || s == SnSR::FIN_WAIT)
	 {
     nSocketId = i;
     break;
   }
 }

 if (nSocketId == MAX_SOCK_NUM)
   return 0; // couldn't find one.

 if (socket(nSocketId, SnMR::UDP, nLocalPort, 0))
 {
   nResult = sendto(nSocketId,(unsigned char*)pDataPacket,nPacketLength,(unsigned char*)pRemoteIP,nRemotePort);
   close(nSocketId);
 } else
   nResult = 0;

 return nResult;
}


//String mac2String(byte ar[]){
//  String s;
//  for (byte i = 0; i < 6; ++i)
//  {
//    char buf[3];
//    sprintf(buf, "%2X", ar[i]);
//    if(buf[0] == ' ') buf[0]='0';
//    s += buf;
//    if (i < 5) s += ':';
//  }
//  return s;
//}

String mac2StringC(byte ar[]){
  String s;
  for (byte i = 0; i < 6; ++i)
  {
    char buf[3];
    sprintf(buf, "%2X", ar[i]);
    if(buf[0] == ' ') buf[0]='0';
    s += buf;
  }
  return s;
}
