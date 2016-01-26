/*
  This example demonstrate how to upload sensor data to MQTT server of LASS.
  It include features:
      (1) Connect to WiFi
      (2) Retrieve NTP time with WiFiUDP
      (3) Get PM 2.5 value from PMS3003 air condition sensor with UART
      (4) Optional DHT support ,comment #define USE_DHT
      (5) Connect to MQTT server and try reconnect when disconnect
      
  https://github.com/LinkItONEDevGroup/LASS

*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>


//#define USE_DHT
#ifdef USE_DHT
#include "DHT.h"
#endif


char ssid[] = "YourWiFiAPName";      // your network SSID (name)
char pass[] = "YourWiFiAPPass";     // your network password

char gps_lat[] = "24.7805647";  // device's gps latitude
char gps_lon[] = "120.9933177"; // device's gps longitude


#ifdef USE_DHT

#define DHTPIN 2     // what digital pin we're connected to
#define DHTTYPE DHT11   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

DHT dht(DHTPIN, DHTTYPE);
#endif

const char server[] = "gpssensor.ddns.net"; // the MQTT server of LASS
const char ntpServer[] = "pool.ntp.org";

#define MAX_CLIENT_ID_LEN 10
#define MAX_TOPIC_LEN     50
char clientId[MAX_CLIENT_ID_LEN];
char outTopic[MAX_TOPIC_LEN];

void MQTTcallback(char* topic, byte* payload, unsigned int length) 
{
	Serial.print(F("Message arrived ["));
	Serial.print(topic);
	Serial.print(F("] "));
	
	for (int i=0;i<length;i++) {
		Serial.print((char)payload[i]);
	}

	Serial.println();
}

WiFiClient wifiClient;
PubSubClient mqttclient((char*)server, 1883, MQTTcallback, wifiClient);
WiFiUDP Udp;

void initializeWiFi() 
{
  int status = WL_IDLE_STATUS;

	// attempt to connect to Wifi network:
	while (status != WL_CONNECTED) {
		Serial.print("Attempting to connect to SSID: ");
		Serial.println(ssid);
		// Connect to WPA/WPA2 network. Change this line if using open or WEP network:
		status = WiFi.begin(ssid, pass);

		// wait 10 seconds for connection:
		Serial.println("WiFi connected , wait for 10 sec to get IP");
		delay(10000);
	}

       // DHCP IP
	Serial.println(F("IP address: "));
	Serial.println(WiFi.localIP());

	 // local port to listen for UDP packets
	 Udp.begin(2390);
}

//NTP
unsigned int localPort = 2390;      // local port to listen for UDP packets
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
const byte nptSendPacket[ NTP_PACKET_SIZE] = {
  0xE3, 0x00, 0x06, 0xEC, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x31, 0x4E, 0x31, 0x34,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
byte ntpRecvBuffer[ NTP_PACKET_SIZE ];
uint32_t epochSystem = 0; // timestamp of system boot up

#define LEAP_YEAR(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )
static const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0

void  retrieveNtpTime(){
  Serial.println("Send NTP packet");

  Udp.beginPacket(ntpServer, 123); //NTP requests are to port 123
  Udp.write(nptSendPacket, NTP_PACKET_SIZE);
  Udp.endPacket();

  if(Udp.parsePacket()) {
    Serial.println("NTP packet received");
    Udp.read(ntpRecvBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
    
    unsigned long highWord = word(ntpRecvBuffer[40], ntpRecvBuffer[41]);
    unsigned long lowWord = word(ntpRecvBuffer[42], ntpRecvBuffer[43]);
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    const unsigned long seventyYears = 2208988800UL;
    unsigned long epoch = secsSince1900 - seventyYears;

    epochSystem = epoch - millis() / 1000;
  }
}



//Sensor
long pmcf10=0;
long pmcf25=0;
long pmcf100=0;
long pmat10=0;
long pmat25=0;
long pmat100=0;

void retrievePM25Value() 
{  
  int count;
  unsigned char c;
  unsigned char high;
  
  count=0;
  while (Serial1.available()) {
    c = Serial1.read();
    if((count==0 && c!=0x42) || (count==1 && c!=0x4d)){
      Serial.println(F("#[G3-ERROR-CHECKSUM]"));
      break;
    }
    if(count > 15){
      Serial.println("G3 complete");
      break;
    }
    else if(count == 4 || count == 6 || count == 8 || count == 10 || count == 12 || count == 14) {
      high = c;
    }
    else if(count == 5){
      pmcf10 = 256*high + c;
      Serial.print("CF=1, PM1.0=");
      Serial.print(pmcf10);
      Serial.println(" ug/m3");
    }
    else if(count == 7){
      pmcf25 = 256*high + c;
      Serial.print("CF=1, PM2.5=");
      Serial.print(pmcf25);
      Serial.println(" ug/m3");
    }
    else if(count == 9){
      pmcf100 = 256*high + c;
      Serial.print("CF=1, PM10=");
      Serial.print(pmcf100);
      Serial.println(" ug/m3");
    }
    else if(count == 11){
      pmat10 = 256*high + c;
      Serial.print("atmosphere, PM1.0=");
      Serial.print(pmat10);
      Serial.println(" ug/m3");
    }
    else if(count == 13){
      pmat25 = 256*high + c;
      Serial.print("atmosphere, PM2.5=");
      Serial.print(pmat25);
      Serial.println(" ug/m3");
    }
    else if(count == 15){
      pmat100 = 256*high + c;
      Serial.print("atmosphere, PM10=");
      Serial.print(pmat100);
      Serial.println(" ug/m3");
    }
    count++;
  }
  while(Serial1.available()) Serial1.read();
}

void initializeMQTT() {
	byte mac[6];

	WiFi.macAddress(mac);
	memset(clientId, 0, MAX_CLIENT_ID_LEN);
	sprintf(clientId, "FT2_0%02X%02X", mac[4], mac[5]);
	sprintf(outTopic, "LASS/Test/Pm25Ameba/%s", clientId);

	Serial.print("MQTT client id:");
	Serial.println(clientId);
	Serial.print("MQTT topic:");
	Serial.println(outTopic);

}

void reconnectMQTT() {
	// Loop until we're reconnected
	while (!mqttclient.connected()) {
		Serial.print(F("Attempting MQTT connection..."));
		
		// Attempt to connect
		if (mqttclient.connect(clientId)) {
			Serial.println(F("connected"));
			mqttclient.subscribe(outTopic);
		} else {
			Serial.println(F("Failed... try again in 5 seconds"));

			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}

void getCurrentTime(unsigned long epoch, int *year, int *month, int *day, int *hour, int *minute, int *second) {
  int tempDay = 0;

  *hour = (epoch  % 86400L) / 3600;
  *minute = (epoch  % 3600) / 60;
  *second = epoch % 60;

  *year = 1970;
  *month = 0;
  *day = epoch / 86400;

  for (*year = 1970; ; (*year)++) {
    if (tempDay + (LEAP_YEAR(*year) ? 366 : 365) > *day) {
      break;
    } else {
      tempDay += (LEAP_YEAR(*year) ? 366 : 365);
    }
  }
  tempDay = *day - tempDay; // the days left in a year
  for ((*month) = 0; (*month) < 12; (*month)++) {
    if ((*month) == 1) {
      if (LEAP_YEAR(*year)) {
        if (tempDay - 29 < 0) {
          break;
        } else {
          tempDay -= 29;
        }
      } else {
        if (tempDay - 28 < 0) {
          break;
        } else {
          tempDay -= 28;
        }
      }
    } else {
      if (tempDay - monthDays[(*month)] < 0) {
        break;
      } else {
        tempDay -= monthDays[(*month)];
      }
    }
  }
  (*month)++;
  *day = tempDay+2; // one for base 1, one for current day
}


char payload[300];
void sendMQTTMessage(){
	unsigned long epoch = epochSystem + millis() / 1000;
	int year, month, day, hour, minute, second;
	getCurrentTime(epoch, &year, &month, &day, &hour, &minute, &second);

      sprintf(payload, "|ver_format=3|fmt_opt=1|app=Pm25Ameba|ver_app=0.0.1|device_id=%s|tick=%d|date=%4d-%02d-%02d|time=%02d:%02d:%02d|device=Ameba|s_d0=%d|gps_lat=%s|gps_lon=%s|gps_fix=1|gps_num=9|gps_alt=2",
        clientId,
        millis(),
        year, month, day,
        hour, minute, second,
        pmat25,
        gps_lat, gps_lon
      );

	// Once connected, publish an announcement...
	mqttclient.publish((char*)outTopic,payload);
	Serial.print(outTopic);
	Serial.println(payload);
}

void setup()
{
  Serial.begin(9600);
	delay(10);
	initializeWiFi();
	retrieveNtpTime();
	initializeMQTT();
#ifdef USE_DHT
	dht.begin();
#endif
  Serial1.begin(9600); // PMS 3003 UART has baud rate 9600
}


void loop()
{ 
#ifdef USE_DHT
	h = dht.readHumidity();
	t = dht.readTemperature();
	if(isnan(h) || isnan(t)) { h=-1;t=-1;}
#endif

	retrievePM25Value();
  if (pmat25 == 0 ) {
    Serial.println("try to read PM25 sensor later");
    delay(5000);
    return;
  }

	//Process Filter or any logic control below
	if(mqttclient.connected()){
			sendMQTTMessage();
      mqttclient.loop();
      delay(60000);  
	} else {
		reconnectMQTT();
    delay(1000);
    return;
	}
	
}

